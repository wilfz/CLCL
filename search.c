/*
 * CLCL
 *
 * search.c
 *
 * Copyright (C) 2026 by Wilf Zimmermann. MIT License.
 *		https://linguversa.de/clcl
 *		https://github.com/wilfz/clcl
 */

 /* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <commctrl.h>
#include <tchar.h>

#include "General.h"
#include "search.h"
#include "Memory.h"

#include "resource.h"

/* Global Variables */
typedef struct {
	TCHAR* str_search;
	BOOL with_date_time;
	FILETIME ft_from;
	FILETIME ft_until;
	TCHAR* str_wndname;
	DATA_INFO* current;
	DATA_INFO* scope;
} match_type;

static match_type viewer_srch = { NULL, FALSE, {0,0}, {0,0}, NULL, NULL };

// extern
extern HINSTANCE hInst;
extern DATA_INFO history_data;
extern DATA_INFO regist_data;

// forward declarations
DATA_INFO* find_next_match( HWND hWnd, match_type* pmt);
static BOOL data_or_title_matches(const HWND hWnd, TCHAR* str, DATA_INFO* item);
static void set_search_controls(HWND hDlg, match_type* psearch);
static void get_search_controls(HWND hDlg, match_type* psearch);
static BOOL FileTimeToSystemTimeCL(const FILETIME ft, SYSTEMTIME* st);
static BOOL SystemTimeToFileTimeCL(const SYSTEMTIME st, FILETIME* ft);

/*
 * search_item_proc
 */
static BOOL CALLBACK search_item_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		set_search_controls(hDlg, &viewer_srch);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case IDOK: {
				mem_free((void**)&viewer_srch.str_search);
				int len = SendMessage(GetDlgItem(hDlg, IDC_SEARCHTEXT), WM_GETTEXTLENGTH, 0, (LPARAM)0) + 1;
				if ((viewer_srch.str_search = mem_alloc(sizeof(TCHAR) * (len + 1))) != NULL) {
					*viewer_srch.str_search = TEXT('\0');
					SendMessage(GetDlgItem(hDlg, IDC_SEARCHTEXT), WM_GETTEXT, len, (LPARAM)viewer_srch.str_search);
				}
				viewer_srch.with_date_time = (SendMessage(GetDlgItem(hDlg, IDC_CHK_SRCHDATE), BM_GETCHECK, 0, 0) == BST_CHECKED);
				if (viewer_srch.with_date_time) {
					SYSTEMTIME date_from, time_from, date_until, time_until;
					DWORD dtstate = DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_DATEFROM), &date_from);
					DWORD tmstate = DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_TIMEFROM), &time_from);
					dtstate = DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_DATEUNTIL), &date_until);
					tmstate = DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_TIMEUNTIL), &time_until);
					date_from.wHour = time_from.wHour;
					date_from.wMinute = time_from.wMinute;
					date_from.wSecond = time_from.wSecond;
					date_from.wMilliseconds = time_from.wMilliseconds;
					date_until.wHour = time_until.wHour;
					date_until.wMinute = time_until.wMinute;
					date_until.wSecond = time_until.wSecond;
					date_until.wMilliseconds = time_until.wMilliseconds;
					SystemTimeToFileTimeCL(date_from, &viewer_srch.ft_from);
					SystemTimeToFileTimeCL(date_until, &viewer_srch.ft_until);
				}
				else {
					viewer_srch.ft_from.dwLowDateTime = viewer_srch.ft_from.dwHighDateTime = 0;
					viewer_srch.ft_until.dwLowDateTime = viewer_srch.ft_until.dwHighDateTime = 0;
				}
				EndDialog(hDlg, TRUE);
			}
			break;

		case IDC_CHK_SRCHDATE:
			if (HIWORD(wParam) == BN_CLICKED) {
				viewer_srch.with_date_time = (SendMessage(GetDlgItem(hDlg, IDC_CHK_SRCHDATE), BM_GETCHECK, 0, 0) == BST_CHECKED);
				EnableWindow(GetDlgItem(hDlg, IDC_DATEFROM), viewer_srch.with_date_time);
				EnableWindow(GetDlgItem(hDlg, IDC_TIMEFROM), viewer_srch.with_date_time);
				EnableWindow(GetDlgItem(hDlg, IDC_DATEUNTIL), viewer_srch.with_date_time);
				EnableWindow(GetDlgItem(hDlg, IDC_TIMEUNTIL), viewer_srch.with_date_time);
			}

		default:
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * search_item_dlg
 */
void search_item_dlg( HWND hWnd, DATA_INFO* scope)
{
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SEARCH_ITEM), hWnd, search_item_proc, 0) == FALSE)
		return;

	viewer_srch.scope = scope;
	viewer_srch.current = scope;
	DATA_INFO* item = find_next_match(hWnd, &viewer_srch);
	if (item == NULL || (BOOL)SendMessage(hWnd, WM_VIEWER_SELECT_ITEM, 0, (LPARAM)item) != TRUE) {
		// TODO: Show MessageBox "No match found."
	}
}

void search_next(HWND hWnd)
{
	DATA_INFO* item = find_next_match(hWnd, &viewer_srch);
	if (item == NULL || (BOOL)SendMessage(hWnd, WM_VIEWER_SELECT_ITEM, 0, (LPARAM)item) != TRUE) {
		// TODO: Show MessageBox "No match found."
	}
}

void search_del(DATA_INFO* del_di)
{
	// prevent dangling pointer in search
	if (viewer_srch.scope == del_di) {
		viewer_srch.scope = NULL;
		viewer_srch.current = NULL;
	}
	if (viewer_srch.current == del_di) {
		viewer_srch.current = NULL;
	}
}

void search_free()
{
	// clean-up
	mem_free((void**)&(viewer_srch.str_search));
	mem_free((void**)&(viewer_srch.str_wndname));
}

BOOL item_matches(const HWND hWnd, match_type* pmt, const DATA_INFO* item)
{
	if (pmt == NULL)
		return FALSE;

	BOOL bFound = TRUE;

	if (bFound && pmt->str_wndname && pmt->str_wndname[0] != TEXT('\0')) {
		bFound = (_tcsstr(item->window_name, pmt->str_wndname) != NULL);
	}

	if (bFound && pmt->str_search != NULL && pmt->str_search[0] != TEXT('\0')) {
		bFound = data_or_title_matches(hWnd, pmt->str_search, (DATA_INFO*)item);
	}

	if (bFound && pmt->with_date_time) {
		bFound = (CompareFileTime(&item->modified, &pmt->ft_from) >= 0) 
			&& (CompareFileTime(&item->modified, &pmt->ft_until) <= 0);
	}

	return bFound;
}

// local helper function
static DATA_INFO* get_parent(const DATA_INFO* di)
{
	DATA_INFO* parent;

	if ((parent = data_check(&history_data, di)) != NULL) {
		return parent;
	}
	if ((parent = data_check(&regist_data, di)) != NULL) {
		return parent;
	}

	return (DATA_INFO*)NULL;
}

DATA_INFO* find_next_match( HWND hWnd, match_type* pmt)
{
	if (pmt == NULL)
		return (DATA_INFO*) NULL;

	if ((pmt->str_search == NULL || pmt->str_search[0] == TEXT('\0'))
		&& (pmt->str_wndname == NULL || pmt->str_wndname[0] == TEXT('\0'))
		&& pmt->with_date_time == FALSE) 
	{
		// nothing to search for
		return (DATA_INFO*) NULL;
	}

	DATA_INFO* item = pmt->current;
	if (pmt->scope == NULL && item == NULL) {
		item = &history_data;
		if (item == NULL) {
			pmt->current = item;
			return (DATA_INFO*)NULL;
		}
	}
	else if (item == NULL) {
		item = (DATA_INFO*)pmt->scope;
		if (item_matches(hWnd, pmt, item)) {
			pmt->current = item;
			return (DATA_INFO*)item;
		}
	}

	while (item != NULL) {
		if ((item->type == TYPE_FOLDER || item->type == TYPE_ROOT) && item->child != NULL) {
			item = item->child;
		}
		else if (item->next != NULL) {
			item = item->next;
		}
		else {
			// otherwise, move up to the parent and check for next siblings
			while ((item = get_parent(item)) != NULL) {
				if (item == pmt->scope) {
					pmt->current = NULL;
					return (DATA_INFO*)NULL; // Reached scope root, no more items to search
				}

				if (item == &history_data) {
					if (pmt->scope == NULL) {
						// continue searching in templates
						item = &regist_data;
						break; // break the while loop to continue searching in templates
					}
					else {
						pmt->current = NULL;
						return (DATA_INFO*)NULL; // Reached the history root, no more items to search
					}
				}

				if (item->next) {
					item = item->next;
					break; // Found a next item to check; break the inner while loop
				}
			}
		}

		if (item_matches(hWnd, pmt, item)) {
			pmt->current = item;
			return (DATA_INFO*)item; // Found a match
		}
	}

	pmt->current = NULL;
	return (DATA_INFO*)NULL; // No match found
}

static BOOL data_or_title_matches(const HWND hWnd, TCHAR* str, DATA_INFO* item)
{
	BOOL bMatches = FALSE;
	DATA_INFO* di = NULL;
	TCHAR* mem;

#ifdef UNICODE
	di = (DATA_INFO*)SendMessage(hWnd, WM_ITEM_GET_FORMAT_TO_ITEM, (WPARAM)TEXT("UNICODE TEXT"), (LPARAM)item);
#else
	di = (DATA_INFO*)SendMessage(hWnd, WM_ITEM_GET_FORMAT_TO_ITEM, (WPARAM)TEXT("TEXT"), (LPARAM)tdi->di);
#endif

	if ((di == NULL || di->data == NULL) && item->title && item->title[0] != TEXT('\0')) {
		// search title only instead of content
		bMatches = (_tcsstr(item->title, str) != NULL);
		return bMatches;
	}

	if (di == NULL || di->data == NULL || (mem = GlobalLock(di->data)) == NULL) {
		return FALSE;
	}

	switch (di->format) {
	case CF_UNICODETEXT:
		if (*(WCHAR*)mem == L'\0') {
			GlobalUnlock(di->data);
			return FALSE;
		}
		break;

	case CF_TEXT:
		if (*(char*)mem == '\0') {
			GlobalUnlock(di->data);
			return FALSE;
		}
		break;

	default:
		GlobalUnlock(di->data);
		return FALSE;
	}

	bMatches = (_tcsstr((TCHAR*)mem, str) != NULL);

	GlobalUnlock(di->data);

	return bMatches;
}

static void set_search_controls(HWND hDlg, match_type* pmt)
{
	if (pmt == NULL)
		return;
	if ((pmt->str_search) == NULL) {
		mem_free((void**)&(pmt->str_search));
		int len = lstrlen(TEXT("Some Text")) + 1;
		if (((pmt->str_search) = mem_alloc(sizeof(TCHAR) * len)) != NULL) {
			lstrcpy((pmt->str_search), TEXT("Some Text"));
		}
	}
	SendMessage(GetDlgItem(hDlg, IDC_SEARCHTEXT), WM_SETTEXT, 0, (LPARAM)(pmt->str_search));
	SendMessage(GetDlgItem(hDlg, IDC_CHK_SRCHDATE), BM_SETCHECK, pmt->with_date_time ? BST_CHECKED : BST_UNCHECKED, 0);
}

static void get_search_controls(HWND hDlg, match_type* pmt)
{
	mem_free((void**)&(pmt->str_search));
	int len = SendMessage(GetDlgItem(hDlg, IDC_SEARCHTEXT), WM_GETTEXTLENGTH, 0, (LPARAM)0) + 1;
	if (((pmt->str_search) = mem_alloc(sizeof(TCHAR) * (len + 1))) != NULL) {
		*(pmt->str_search) = TEXT('\0');
		SendMessage(GetDlgItem(hDlg, IDC_SEARCHTEXT), WM_GETTEXT, len, (LPARAM)(pmt->str_search));
	}
	pmt->with_date_time = (SendMessage(GetDlgItem(hDlg, IDC_CHK_SRCHDATE), BM_GETCHECK, 0, 0) == BST_CHECKED);
	if (pmt->with_date_time) {
		SYSTEMTIME date_from, time_from, date_until, time_until;
		DWORD dtstate = DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_DATEFROM), &date_from);
		DWORD tmstate = DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_TIMEFROM), &time_from);
		dtstate = DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_DATEUNTIL), &date_until);
		tmstate = DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_TIMEUNTIL), &time_until);
		date_from.wHour = time_from.wHour;
		date_from.wMinute = time_from.wMinute;
		date_from.wSecond = time_from.wSecond;
		date_from.wMilliseconds = time_from.wMilliseconds;
		date_until.wHour = time_until.wHour;
		date_until.wMinute = time_until.wMinute;
		date_until.wSecond = time_until.wSecond;
		date_until.wMilliseconds = time_until.wMilliseconds;
		SystemTimeToFileTimeCL(date_from, &pmt->ft_from);
		SystemTimeToFileTimeCL(date_until, &pmt->ft_until);
	}
	else {
		pmt->ft_from.dwLowDateTime = pmt->ft_from.dwHighDateTime = 0;
		pmt->ft_until.dwLowDateTime = pmt->ft_until.dwHighDateTime = 0;
	}
}

static BOOL FileTimeToSystemTimeCL(const FILETIME ft, SYSTEMTIME* st)
{
	if (ft.dwHighDateTime != 0 && ft.dwLowDateTime != 0
		&& FileTimeToSystemTime(&ft, st))
	{
		return TRUE;
	}

	st->wYear = st->wMonth = st->wDay = st->wHour = 
		st->wMinute = st->wSecond = st->wMilliseconds = 0;

	return FALSE;
}

static BOOL SystemTimeToFileTimeCL(const SYSTEMTIME st, FILETIME* ft)
{
	if (st.wYear >= 1900 && st.wMonth > 0 && st.wDay > 0) {
		if (SystemTimeToFileTime(&st, ft))
			return TRUE;
	}

	ft->dwHighDateTime = ft->dwLowDateTime = 0;
	return FALSE;
}


/* End of source */
