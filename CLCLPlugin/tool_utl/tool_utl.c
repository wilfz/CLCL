/*
 * tool_utl
 *
 * tool_utl.c
 *
 * Copyright (C) 1996-2003 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "..\CLCLPlugin.h"

/* Define */
#define	INI_FILE_NAME				TEXT("tool_utl.ini")

/* Global Variables */
HINSTANCE hInst;
TCHAR ini_path[BUF_SIZE];

static TCHAR wave_file[BUF_SIZE];

TCHAR multi_save_format[BUF_SIZE];
TCHAR multi_save_file_name[BUF_SIZE];

/* Local Function Prototypes */
static BOOL dll_initialize(void);
static BOOL dll_uninitialize(void);

/*
 * DllMain - メイン
 */
int WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		hInst = hInstance;
		dll_initialize();
		break;

	case DLL_PROCESS_DETACH:
		dll_uninitialize();
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

/*
 * dll_initialize - 初期化
 */
static BOOL dll_initialize(void)
{
	TCHAR app_path[BUF_SIZE];
	TCHAR *p, *r;

	GetModuleFileName(hInst, app_path, BUF_SIZE - 1);
	for (p = r = app_path; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			p++;
			continue;
		}
#endif	// UNICODE
		if (*p == TEXT('\\') || *p == TEXT('/')) {
			r = p;
		}
	}
	*r = TEXT('\0');
	wsprintf(ini_path, TEXT("%s\\%s"), app_path, INI_FILE_NAME);

	GetPrivateProfileString(TEXT("play_sound"), TEXT("wave_file"), TEXT(""), wave_file, BUF_SIZE - 1, ini_path);

	GetPrivateProfileString(TEXT("multi_save"), TEXT("format"), TEXT("TEXT"), multi_save_format, BUF_SIZE - 1, ini_path);
	GetPrivateProfileString(TEXT("multi_save"), TEXT("file_name"), TEXT("%04d.txt"), multi_save_file_name, BUF_SIZE - 1, ini_path);
	return TRUE;
}

/*
 * dll_uninitialize - 終了処理
 */
static BOOL dll_uninitialize(void)
{
	return TRUE;
}

/*
 * get_tool_info_w - ツール情報取得
 */
__declspec(dllexport) BOOL CALLBACK get_tool_info_w(const HWND hWnd, const int index, TOOL_GET_INFO *tgi)
{
	switch (index) {
	case 0:
		lstrcpy(tgi->title, TEXT("Clear &History"));
		lstrcpy(tgi->func_name, TEXT("clear_history"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_VIEWER;
		return TRUE;

	case 1:
		lstrcpy(tgi->title, TEXT("Clear &Clipboard"));
		lstrcpy(tgi->func_name, TEXT("clear_clipboard"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_VIEWER;
		return TRUE;

	case 2:
		lstrcpy(tgi->title, TEXT("&Play Sound"));
		lstrcpy(tgi->func_name, TEXT("play_sound"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_ADD_HISTORY;
		return TRUE;

	case 3:
		lstrcpy(tgi->title, TEXT("Always on &Top"));
		lstrcpy(tgi->func_name, TEXT("ztop"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_VIEWER;
		return TRUE;

	case 4:
		lstrcpy(tgi->title, TEXT("&Un Top"));
		lstrcpy(tgi->func_name, TEXT("unztop"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_VIEWER;
		return TRUE;

	case 5:
		lstrcpy(tgi->title, TEXT("Save of &more items"));
		lstrcpy(tgi->func_name, TEXT("multi_save"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_VIEWER;
		return TRUE;
	}
	return FALSE;
}

/*
 * clear_history - 履歴のクリア
 */
__declspec(dllexport) int CALLBACK clear_history(const HWND hWnd, TOOL_EXEC_INFO *tei, TOOL_DATA_INFO *tdi)
{
	DATA_INFO *di;
	
	if (MessageBox(hWnd, TEXT("Are you sure you want to delete all items?"), TEXT("CLCL - Clear History"),
		MB_ICONQUESTION | MB_YESNO | MB_TOPMOST) == IDNO) {
		return TOOL_SUCCEED;
	}
	// 履歴の取得
	di = (DATA_INFO *)SendMessage(hWnd, WM_HISTORY_GET_ROOT, 0, 0);
	if (di != NULL) {
		// 履歴の解放
		SendMessage(hWnd, WM_ITEM_FREE, 0, (LPARAM)di->child);
		di->child = NULL;
		// 履歴の変化を通知
		SendMessage(hWnd, WM_HISTORY_CHANGED, 0, 0);
	}
	return TOOL_SUCCEED;
}

/*
 * clear_history_property - プロパティ表示
 */
__declspec(dllexport) BOOL CALLBACK clear_history_property(const HWND hWnd, TOOL_EXEC_INFO *tei)
{
	return FALSE;
}

/*
 * clear_clipboard - クリップボードのクリア
 */
__declspec(dllexport) int CALLBACK clear_clipboard(const HWND hWnd, TOOL_EXEC_INFO *tei, TOOL_DATA_INFO *tdi)
{
	if (OpenClipboard(hWnd) == TRUE) {
		EmptyClipboard();
		CloseClipboard();
	}
	return TOOL_SUCCEED;
}

/*
 * clear_clipboard_property - プロパティ表示
 */
__declspec(dllexport) BOOL CALLBACK clear_clipboard_property(const HWND hWnd, TOOL_EXEC_INFO *tei)
{
	return FALSE;
}

/*
 * play_sound - WAVEファイルを鳴らす
 */
__declspec(dllexport) int CALLBACK play_sound(const HWND hWnd, TOOL_EXEC_INFO *tei, TOOL_DATA_INFO *tdi)
{
	if (*wave_file == TEXT('\0') || sndPlaySound(wave_file, SND_ASYNC | SND_NODEFAULT) == FALSE) {
		MessageBeep(MB_ICONASTERISK);
	}
	return TOOL_SUCCEED;
}

/*
 * play_sound_property - プロパティ表示
 */
__declspec(dllexport) BOOL CALLBACK play_sound_property(const HWND hWnd, TOOL_EXEC_INFO *tei)
{
	OPENFILENAME of;
	TCHAR buf[MAX_PATH];

	// ファイルの選択
	ZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hInstance = hInst;
	of.hwndOwner = hWnd;
	of.lpstrTitle = TEXT("WAVE file");
	of.lpstrFilter = TEXT("*.wav\0*.wav\0*.*\0*.*\0\0");
	of.nFilterIndex = 1;
	lstrcpy(buf, wave_file);
	of.lpstrFile = buf;
	of.nMaxFile = MAX_PATH - 1;
	of.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (GetOpenFileName(&of) == TRUE) {
		lstrcpy(wave_file, buf);
		WritePrivateProfileString(TEXT("play_sound"), TEXT("wave_file"), wave_file, ini_path);
	}
	return TRUE;
}

/*
 * ztop - 最前面に表示
 */
__declspec(dllexport) int CALLBACK ztop(const HWND hWnd, TOOL_EXEC_INFO *tei, TOOL_DATA_INFO *tdi)
{
	HWND hViewerWnd;

	hViewerWnd = (HWND)SendMessage(hWnd, WM_VIEWER_GET_HWND, 0, 0);
	if (hViewerWnd != NULL) {
		// 最前面に表示
		SetWindowPos(hViewerWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	return TOOL_SUCCEED;
}

/*
 * ztop_property - プロパティ表示
 */
__declspec(dllexport) BOOL CALLBACK ztop_property(const HWND hWnd, TOOL_EXEC_INFO *tei)
{
	return FALSE;
}

/*
 * unztop - 最前面に表示の解除
 */
__declspec(dllexport) int CALLBACK unztop(const HWND hWnd, TOOL_EXEC_INFO *tei, TOOL_DATA_INFO *tdi)
{
	HWND hViewerWnd;

	hViewerWnd = (HWND)SendMessage(hWnd, WM_VIEWER_GET_HWND, 0, 0);
	if (hViewerWnd != NULL) {
		// 最前面に表示
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	return TOOL_SUCCEED;
}

/*
 * unztop_property - プロパティ表示
 */
__declspec(dllexport) BOOL CALLBACK unztop_property(const HWND hWnd, TOOL_EXEC_INFO *tei)
{
	return FALSE;
}
/* End of source */
