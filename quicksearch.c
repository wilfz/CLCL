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
#include <strsafe.h>

#include "quicksearch.h"
#include "Data.h"
#include "Memory.h"
#include "ImageList.h"
#include "Ini.h"
#include "Profile.h"
#include "dpi.h"
#include "Font.h"

#include "dynamic_popup.h"


#define EM_HIDE_LISTBOX        (WM_USER + 86)

/* Global Variables */
HWND hWndClcl = NULL; // the one and only application window handle
TCHAR ini_path[MAX_PATH] = TEXT("");

// extern
extern HINSTANCE hInst;
extern DATA_INFO history_data;
extern DATA_INFO regist_data;
extern OPTION_INFO option;
extern TCHAR work_path[];

/* Item data structure for owner-drawn listbox */
typedef struct {
	DATA_INFO* data_info;
	int icon_index;
	TCHAR text[BUF_SIZE];
} LISTBOX_ITEM_DATA;

/* Local Function Prototypes */
static int get_icon_index_for_data(DATA_INFO* di);
static int listbox_add_matches(HWND hListBox, DATA_INFO* item, const TCHAR* srch, int max_cnt);
int listbox_add_matches(HWND hListBox, DATA_INFO* start, const TCHAR* srch, int max_cnt)
{
	int cnt = 0;
	int idx = 0;
	for (DATA_INFO* item = start; item != NULL; item = item->next)
	{
		switch (item->type) {
		case TYPE_ROOT:
		case TYPE_FOLDER:
			if (item->child != NULL) {
				cnt = listbox_add_matches(hListBox, item->child, srch, max_cnt);
				if (cnt >= max_cnt)
					return cnt;
			}
			break;
		case TYPE_ITEM:
			if (item->title && item->title[0] != TEXT('\0') 
				&& _tcsstr(item->title, srch) != NULL)
			{
				//listbox_add_item_with_icon(hListBox, item->title, item);
				int icon_index = get_icon_index_for_data(item);
				PopupAddString(hListBox, item->title, icon_index, (UINT_PTR) item);
				if ((cnt = SendMessage(hListBox, LB_GETCOUNT, 0, 0)) >= max_cnt)
					return cnt;
			}
			else if (item->menu_title && item->menu_title[0] != TEXT('\0')
				&& _tcsstr(item->menu_title, srch) != NULL) 
			{
				//listbox_add_item_with_icon(hListBox, item->menu_title, item);
				int icon_index = get_icon_index_for_data(item);
				PopupAddString(hListBox, item->menu_title, icon_index, (UINT_PTR)item);
				if ((cnt = SendMessage(hListBox, LB_GETCOUNT, 0, 0)) >= max_cnt)
					return cnt;
			}
			else {
				BOOL bMatches = FALSE;
				DATA_INFO* di = NULL;
				TCHAR* mem;

#ifdef UNICODE
				di = (DATA_INFO*)SendMessage(hWndClcl, WM_ITEM_GET_FORMAT_TO_ITEM, (WPARAM)TEXT("UNICODE TEXT"), (LPARAM)item);
#else
				di = (DATA_INFO*)SendMessage(hWndClcl, WM_ITEM_GET_FORMAT_TO_ITEM, (WPARAM)TEXT("TEXT"), (LPARAM)item);
#endif

				if (di == NULL || di->data == NULL || (mem = GlobalLock(di->data)) == NULL) {
					break;
				}

				switch (di->format) {
				case CF_UNICODETEXT:
					if (*(WCHAR*)mem == L'\0') {
						GlobalUnlock(di->data);
						continue;
					}
					break;

				case CF_TEXT:
					if (*(char*)mem == '\0') {
						GlobalUnlock(di->data);
						continue;
					}
					break;

				default:
					GlobalUnlock(di->data);
					continue;
				}

				bMatches = (_tcsstr((TCHAR*)mem, srch) != NULL);
				if (bMatches) {
					TCHAR buf[BUF_SIZE];
					StringCchCopy(buf, BUF_SIZE, (TCHAR*)mem);
					buf[BUF_SIZE-1] = TEXT('\0');
					//listbox_add_item_with_icon(hListBox, buf, item);
					int icon_index = get_icon_index_for_data(item);
					PopupAddString(hListBox, buf, icon_index, (UINT_PTR)item);
				}

				GlobalUnlock(di->data);

				if ((cnt = SendMessage(hListBox, LB_GETCOUNT, 0, 0)) >= max_cnt)
					return cnt;
			}
			break;
		}
	}

	cnt = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0);
	return cnt;
}

/*
 * menu_create_font - ????????????
 */
static HFONT menu_create_font(void)
{
	NONCLIENTMETRICS ncMetrics;

	if (*option.menu_font_name != TEXT('\0')) {
		return font_create(option.menu_font_name, option.menu_font_size, option.menu_font_charset,
			option.menu_font_weight, (option.menu_font_italic == 0) ? FALSE : TRUE, FALSE);
	}

	ncMetrics.cbSize = sizeof(NONCLIENTMETRICS);
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
		sizeof(NONCLIENTMETRICS), &ncMetrics, 0) == FALSE) {
		return NULL;
	}
	return CreateFontIndirect(&ncMetrics.lfMenuFont);
}

/*
 * get_icon_index_for_data
 * Determine the icon index based on the data format
 */
static int get_icon_index_for_data(DATA_INFO* di)
{
	if (di == NULL)
		return 5; // Default icon index

	// Based on data format, return appropriate icon index
	// Adjust these indices to match your ImageList structure
	switch (di->type) {
	case TYPE_ROOT:
		return 1; // Main icon
	case TYPE_FOLDER:
		return 3; // Folder icon
	case TYPE_ITEM:
		// TODO:  find default format
		if (di->format >= 0 && (int)di->format < option.format_cnt) {
			return 6 + di->format;  // Format-specific icons start at index 6
		}
		return 5; // Default icon
	default:
		return 5; // Default icon
	}
}

/*
 * listbox_add_item_with_icon
 * Add a listbox item with icon and data
 */
static int listbox_add_item_with_icon(HWND hListBox, const TCHAR* text, DATA_INFO* di)
{
	LISTBOX_ITEM_DATA* item_data = (LISTBOX_ITEM_DATA*)malloc(sizeof(LISTBOX_ITEM_DATA));
	if (item_data == NULL)
		return -1;

	BOOL bIconFound = di ? format_get_menu_icon(di) : FALSE;
	item_data->data_info = di;
	item_data->icon_index = get_icon_index_for_data(di);
	StringCchCopy(item_data->text, BUF_SIZE, text);

	int idx = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)item_data->text);
	if (idx >= 0) {
		SendMessage(hListBox, LB_SETITEMDATA, (WPARAM)idx, (LPARAM)item_data);
	} else {
		free(item_data);
	}

	return idx;
}

void MyPopupPopulateHandler(const TCHAR* editText, HWND hwndListBox, void* pUserData);
void MyPopupSelectionHandler(const PopupItemData* pSelectedItem, void* pUserData);

// NEW: DIESER CALLBACK GIBT DEN TOOLTIP-TEXT BEIM HOVERN ZURÜCK
TCHAR* MyPopupTooltipHandler(const PopupItemData* pSelectedItem, void* pUserData)
{
	if (!pSelectedItem) return NULL;

	DATA_INFO* di = (DATA_INFO*)pSelectedItem->itemData;
	if (!di) return NULL;

	// Hier können Sie einen Multiline-Tooltip generieren
	// Beispiel: Rückgabe des Datums, der Kategorie oder des Inhalts
	static TCHAR tooltip_buffer[512];

	// Wenn es ein TYPE_ITEM ist, können wir zusätzliche Infos abrufen
	if (di->type == TYPE_ITEM) {
		{
			StringCchPrintf(tooltip_buffer, 512,
				TEXT("Titel: %s\nFormat: %d"),
				pSelectedItem->pszText,
				di->format
			);
		}
		return tooltip_buffer;
	}

	// Falls kein spezieller Tooltip-Text verfügbar
	return pSelectedItem->pszText;
}

UINT_PTR quicksearch(HWND hWnd, POINT pt)
{
	hWndClcl = hWnd;
	int icon_size = option.menu_icon_size ? option.menu_icon_size : Scale(16);
	int icon_margin = option.menu_icon_margin ? option.menu_icon_margin : Scale(2);
	int text_margin = option.menu_text_margin_left ? option.menu_text_margin_left : Scale(4);
	StringCbPrintf(ini_path, BUF_SIZE, TEXT("%s\\%s"), work_path, USER_INI);
	static unsigned int max_visible_items = 0;
	if (max_visible_items == 0)
		max_visible_items = (unsigned int)profile_get_int(TEXT("quicksearch"), TEXT("max_visible_items"), 10, ini_path);
	HIMAGELIST hImageList = create_imagelist(hInst);
	// Step 1: Create the popup window
	HWND hwndPopup = CreateDynamicPopupMenu(hWnd, pt.x, pt.y, option.menu_max_width);
	if (!hwndPopup) {
		return 0;
	}

	// Step 2: Configure the popup using setter functions
	// Callbacks:
	SetPopulateCallback(hwndPopup, MyPopupPopulateHandler);
	SetTooltipCallback(hwndPopup, MyPopupTooltipHandler);

	// Layout:
	SetImageList(hwndPopup, hImageList, icon_size);
	SetIconMargin(hwndPopup, icon_margin);
	SetTextMargin(hwndPopup, text_margin);
	SetMaxVisibleItems(hwndPopup, max_visible_items);
	SetUserData(hwndPopup, (void*)NULL);

	// Step 3: Track the popup modally and get the result
	UINT_PTR itemData = TrackDynamicPopup(hwndPopup);

	// Step 4: Process the result
	return itemData; // Return the selected item's data or 0 if no selection is made
}

// 1. DIESER CALLBACK BEFÜLLT DIE LISTBOX DYNAMISCH
void MyPopupPopulateHandler(const TCHAR* editText, HWND hwndListBox, void* pUserData) 
{
	// Add matching items to the listbox
	static int max_cnt = 0;
	if (max_cnt == 0)
		max_cnt = profile_get_int(TEXT("quicksearch"), TEXT("max_item_count"), 30, ini_path);

	int item_count = listbox_add_matches(hwndListBox, &history_data, editText, max_cnt);
	if (item_count < max_cnt) {
		item_count = listbox_add_matches(hwndListBox, &regist_data, editText, max_cnt);
	}
}

// 2. DIESER CALLBACK REAGIERT AUF DIE ENDGÜLTIGE AUSWAHL
void MyPopupSelectionHandler(const PopupItemData* pSelectedItem, void* pUserData) 
{
	if (pSelectedItem) {
		DATA_INFO* di = (DATA_INFO*) pSelectedItem->itemData;
		if (di) {
			LRESULT res = SendMessage(hWndClcl, WM_ITEM_TO_CLIPBOARD, 0, (LPARAM)di);
		}
	}
}
