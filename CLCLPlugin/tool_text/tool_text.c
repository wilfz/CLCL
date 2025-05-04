/*
 * tool_text
 *
 * tool_text.c
 *
 * Copyright (C) 1996-2003 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <wchar.h>

#include "..\CLCLPlugin.h"
#include "resource.h"

/* Define */
#define	INI_FILE_NAME				TEXT("tool_text.ini")

/* Global Variables */
HINSTANCE hInst;
TCHAR ini_path[BUF_SIZE];

TCHAR date_format[BUF_SIZE];
TCHAR time_format[BUF_SIZE];

TCHAR quote_char[BUF_SIZE];

int word_break_break_cnt;
int word_break_tab_size;
TCHAR word_break_m_oida[BUF_SIZE];
TCHAR word_break_m_bura[BUF_SIZE];
TCHAR word_break_s_oida[BUF_SIZE];
TCHAR word_break_s_bura[BUF_SIZE];

TCHAR put_text_open[BUF_SIZE];
TCHAR put_text_close[BUF_SIZE];

int delete_crlf_delete_space;

int join_text_add_return;

/* Local Function Prototypes */
static BOOL dll_initialize(void);
static BOOL dll_uninitialize(void);

/*
 * DllMain - ���C��
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
 * dll_initialize - ������
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

	GetPrivateProfileString(TEXT("convert_date"), TEXT("date_format"), TEXT(""), date_format, BUF_SIZE - 1, ini_path);
	GetPrivateProfileString(TEXT("convert_date"), TEXT("time_format"), TEXT(""), time_format, BUF_SIZE - 1, ini_path);

	GetPrivateProfileString(TEXT("quote"), TEXT("char"), TEXT(">"), quote_char, BUF_SIZE - 1, ini_path);

	word_break_break_cnt = GetPrivateProfileInt(TEXT("word_break"), TEXT("break_cnt"), 80, ini_path);
	word_break_tab_size = GetPrivateProfileInt(TEXT("word_break"), TEXT("tab_size"), 8, ini_path);
	GetPrivateProfileString(TEXT("word_break"), TEXT("m_oida"), TEXT(""), word_break_m_oida, BUF_SIZE - 1, ini_path);
	GetPrivateProfileString(TEXT("word_break"), TEXT("m_bura"), TEXT(""), word_break_m_bura, BUF_SIZE - 1, ini_path);
	GetPrivateProfileString(TEXT("word_break"), TEXT("s_oida"), TEXT("\\$([{�"), word_break_s_oida, BUF_SIZE - 1, ini_path);
	GetPrivateProfileString(TEXT("word_break"), TEXT("s_bura"), TEXT(",.?!%:;)]}�����"), word_break_s_bura, BUF_SIZE - 1, ini_path);

	GetPrivateProfileString(TEXT("put_text"), TEXT("open"), TEXT("<TAG>"), put_text_open, BUF_SIZE - 1, ini_path);
	GetPrivateProfileString(TEXT("put_text"), TEXT("close"), TEXT("</TAG>"), put_text_close, BUF_SIZE - 1, ini_path);

	delete_crlf_delete_space = GetPrivateProfileInt(TEXT("delete_crlf"), TEXT("delete_space"), 1, ini_path);

	join_text_add_return = GetPrivateProfileInt(TEXT("join_text"), TEXT("add_return"), 1, ini_path);
	return TRUE;
}

/*
 * dll_uninitialize - �I������
 */
static BOOL dll_uninitialize(void)
{
	return TRUE;
}

/*
 * get_tool_info_w - �c�[�����擾
 * Get tool information
 *
 *	���� / argument:
 *		hWnd - �Ăяo�����E�B���h�E / the calling window
 *		index - �擾�̃C���f�b�N�X (0�`) / the index of the acquisition (from 0)
 *		tgi - �c�[���擾��� / tool retrieval information
 *
 *	�߂�l / Return value:
 *		TRUE - ���Ɏ擾����c�[������ / has tools to get next
 *		FALSE - �擾�̏I�� / end of acquisition
 */
__declspec(dllexport) BOOL CALLBACK get_tool_info_w(const HWND hWnd, const int index, TOOL_GET_INFO *tgi)
{
	switch (index) {
	case 0:
		//lstrcpy(tgi->title, TEXT("�����ϊ�(&D)"));
		//lstrcpy(tgi->title, TEXT("&Date and Time Conversion"));
		LoadString(hInst, IDS_DATE_TIME_CONVERSION, tgi->title, BUF_SIZE - 1);
		lstrcpy(tgi->func_name, TEXT("convert_date"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_ITEM_TO_CLIPBOARD;
		return TRUE;

	case 1:
		//lstrcpy(tgi->title, TEXT("�������ɕϊ�(&L)"));
		//lstrcpy(tgi->title, TEXT("To &Lower"));
		LoadString(hInst, IDS_TO_LOWER, tgi->title, BUF_SIZE - 1);
		lstrcpy(tgi->func_name, TEXT("to_lower"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_MENU_COPY_PASTE | CALLTYPE_VIEWER;
		return TRUE;

	case 2:
		//lstrcpy(tgi->title, TEXT("�啶���ɕϊ�(&U)"));
		//lstrcpy(tgi->title, TEXT("To &Upper"));
		LoadString(hInst, IDS_TO_UPPER, tgi->title, BUF_SIZE - 1);
		lstrcpy(tgi->func_name, TEXT("to_upper"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_MENU_COPY_PASTE | CALLTYPE_VIEWER;
		return TRUE;

	case 3:
		//lstrcpy(tgi->title, TEXT("���p(&Q)"));
		//lstrcpy(tgi->title, TEXT("&Quotation"));
		LoadString(hInst, IDS_QUOTATION, tgi->title, BUF_SIZE - 1);
		lstrcpy(tgi->func_name, TEXT("quote"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_MENU_COPY_PASTE | CALLTYPE_VIEWER;
		return TRUE;

	case 4:
		//lstrcpy(tgi->title, TEXT("���p����(&N)"));
		//lstrcpy(tgi->title, TEXT("U&n Quotation"));
		LoadString(hInst, IDS_UNQUOTATION, tgi->title, BUF_SIZE - 1);
		lstrcpy(tgi->func_name, TEXT("unquote"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_MENU_COPY_PASTE | CALLTYPE_VIEWER;
		return TRUE;

	case 5:
		//lstrcpy(tgi->title, TEXT("�e�L�X�g���`(&W)"));
		//lstrcpy(tgi->title, TEXT("&Word Wrap"));
		LoadString(hInst, IDS_WORDWRAP, tgi->title, BUF_SIZE - 1);
		lstrcpy(tgi->func_name, TEXT("word_break"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_MENU_COPY_PASTE | CALLTYPE_VIEWER;
		return TRUE;

	case 6:
		//lstrcpy(tgi->title, TEXT("�e�L�X�g�̋��ݍ���(&P)"));
		//lstrcpy(tgi->title, TEXT("<&TAG></TAG>"));
		LoadString(hInst, IDS_TAG, tgi->title, BUF_SIZE - 1);
		lstrcpy(tgi->func_name, TEXT("put_text"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_MENU_COPY_PASTE | CALLTYPE_VIEWER;
		return TRUE;

	case 7:
		//lstrcpy(tgi->title, TEXT("���s�̏���(&R)"));
		//lstrcpy(tgi->title, TEXT("Delete C&RLF"));
		LoadString(hInst, IDS_DELETE_CRLF, tgi->title, BUF_SIZE - 1);
		lstrcpy(tgi->func_name, TEXT("delete_crlf"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_MENU_COPY_PASTE | CALLTYPE_VIEWER;
		return TRUE;

	case 8:
		//lstrcpy(tgi->title, TEXT("�e�L�X�g�̘A��(&O)"));
		//lstrcpy(tgi->title, TEXT("C&onnection of text"));
		LoadString(hInst, IDS_CONNECT_TEXT, tgi->title, BUF_SIZE - 1);
		lstrcpy(tgi->func_name, TEXT("join_text"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_VIEWER;
		return TRUE;

	case 9:
		//lstrcpy(tgi->title, TEXT("�e�L�X�g�ҏW(&E)"));
		//lstrcpy(tgi->title, TEXT("&Edit"));
		LoadString(hInst, IDS_EDIT_CLIPBOARD, tgi->title, BUF_SIZE - 1);
		lstrcpy(tgi->func_name, TEXT("text_edit"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_MENU_COPY_PASTE;
		return TRUE;
	}
	return FALSE;
}
/* End of source */
