/*
 * CLCL Installer
 *
 * Installer.c
 *
 * Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define COBJMACROS
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <exdisp.h>
#include <shldisp.h>

#include "instinfo.h"
#include "resource.h"
#include "Installer.h"
#include "unzip.h"

/* Define */
#define MSG_SIZE					1024
#define REG_TARGET_MAX				8
#define LOG_BOM						0xfeff

/* Global Variables */
static HINSTANCE hInst;
// インストールするファイル
static ZIP_ARCHIVE zip;
static DWORD total_size;
// バージョン文字列
static TCHAR ver_str[BUF_SIZE];
// メッセージボックスのタイトル
static TCHAR title_str[BUF_SIZE];
// インストール先
static TCHAR install_path[MAX_PATH];
// 上書きインストール
static BOOL update_mode;
// アプリの一覧の登録場所
static REG_TARGET reg_target[REG_TARGET_MAX];
static int reg_target_cnt;
// インストールしたファイルの一覧
static STR_BUF log_buf;
// インストール完了
static BOOL install_end;
// タイトル用のフォント
static HFONT hTitleFont;

/* Local Function Prototypes */
static TCHAR *str_load(const UINT id, TCHAR *buf, const int size);
static int msg_box(const HWND hWnd, const UINT id, const UINT type, const TCHAR *arg);
static void set_ctrl_text(const HWND hDlg, const int ctrl_id, const UINT str_id);
static BOOL sb_add(STR_BUF *sb, const TCHAR *str);
static void sb_free(STR_BUF *sb);
static void get_module_dir(TCHAR *path);
static BOOL create_dir(const TCHAR *path);
static void delete_dir_tree(const TCHAR *path);
static void remove_empty_dir(const TCHAR *path);
static BOOL write_file(const HWND hWnd, const TCHAR *path, const BYTE *buf, const DWORD size);
static BOOL is_running(void);
static BOOL check_running(const HWND hWnd);
static void set_language(void);

/*
 * str_load - 文字列リソースの取得
 */
static TCHAR *str_load(const UINT id, TCHAR *buf, const int size)
{
	*buf = TEXT('\0');
	LoadString(hInst, id, buf, size);
	return buf;
}

/*
 * msg_box - メッセージの表示
 */
static int msg_box(const HWND hWnd, const UINT id, const UINT type, const TCHAR *arg)
{
	TCHAR fmt[MSG_SIZE];
	TCHAR msg[MSG_SIZE + MAX_PATH];

	str_load(id, fmt, MSG_SIZE);
	if (arg == NULL) {
		lstrcpy(msg, fmt);
	} else {
		wsprintf(msg, fmt, arg);
	}
	return MessageBox(hWnd, msg, title_str, type);
}

/*
 * set_ctrl_text - コントロールに文字列リソースを設定
 */
static void set_ctrl_text(const HWND hDlg, const int ctrl_id, const UINT str_id)
{
	TCHAR buf[MSG_SIZE];

	SetDlgItemText(hDlg, ctrl_id, str_load(str_id, buf, MSG_SIZE));
}

/*
 * sb_add - 文字列バッファに追加
 */
static BOOL sb_add(STR_BUF *sb, const TCHAR *str)
{
	TCHAR *buf;
	DWORD len = lstrlen(str);
	DWORD size;

	if (sb->len + len + 1 > sb->size) {
		size = sb->size + ((len + 1 > 4096) ? len + 1 : 4096);
		buf = (sb->buf == NULL) ?
			(TCHAR *)HeapAlloc(GetProcessHeap(), 0, size * sizeof(TCHAR)) :
			(TCHAR *)HeapReAlloc(GetProcessHeap(), 0, sb->buf, size * sizeof(TCHAR));
		if (buf == NULL) {
			return FALSE;
		}
		sb->buf = buf;
		sb->size = size;
	}
	lstrcpy(sb->buf + sb->len, str);
	sb->len += len;
	return TRUE;
}

/*
 * sb_free - 文字列バッファの解放
 */
static void sb_free(STR_BUF *sb)
{
	if (sb->buf != NULL) {
		HeapFree(GetProcessHeap(), 0, sb->buf);
		sb->buf = NULL;
	}
	sb->len = 0;
	sb->size = 0;
}

/*
 * get_module_dir - 実行ファイルのあるフォルダを取得
 */
static void get_module_dir(TCHAR *path)
{
	TCHAR *p;

	*path = TEXT('\0');
	GetModuleFileName(NULL, path, MAX_PATH);
	for (p = path + lstrlen(path); p > path; p--) {
		if (*(p - 1) == TEXT('\\')) {
			*(p - 1) = TEXT('\0');
			break;
		}
	}
}

/*
 * create_dir - フォルダの作成 (親フォルダも作成)
 */
static BOOL create_dir(const TCHAR *path)
{
	TCHAR buf[MAX_PATH];
	TCHAR *p;

	if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES) {
		return TRUE;
	}
	lstrcpyn(buf, path, MAX_PATH);
	for (p = buf; *p != TEXT('\0'); p++) {
		if (*p != TEXT('\\') || p == buf || *(p - 1) == TEXT(':') || *(p - 1) == TEXT('\\')) {
			continue;
		}
		*p = TEXT('\0');
		if (GetFileAttributes(buf) == INVALID_FILE_ATTRIBUTES) {
			CreateDirectory(buf, NULL);
		}
		*p = TEXT('\\');
	}
	CreateDirectory(path, NULL);
	return (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES) ? TRUE : FALSE;
}

/*
 * delete_dir_tree - フォルダを中身ごと削除
 */
static void delete_dir_tree(const TCHAR *path)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	TCHAR buf[MAX_PATH];

	if (lstrlen(path) + 3 > MAX_PATH) {
		return;
	}
	wsprintf(buf, TEXT("%s\\*"), path);
	if ((hFind = FindFirstFile(buf, &fd)) != INVALID_HANDLE_VALUE) {
		do {
			if (lstrcmp(fd.cFileName, TEXT(".")) == 0 || lstrcmp(fd.cFileName, TEXT("..")) == 0) {
				continue;
			}
			if (lstrlen(path) + lstrlen(fd.cFileName) + 2 > MAX_PATH) {
				continue;
			}
			wsprintf(buf, TEXT("%s\\%s"), path, fd.cFileName);
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				delete_dir_tree(buf);
			} else {
				SetFileAttributes(buf, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(buf);
			}
		} while (FindNextFile(hFind, &fd) != FALSE);
		FindClose(hFind);
	}
	RemoveDirectory(path);
}

/*
 * remove_empty_dir - 空のサブフォルダを削除
 */
static void remove_empty_dir(const TCHAR *path)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	TCHAR buf[MAX_PATH];

	if (lstrlen(path) + 3 > MAX_PATH) {
		return;
	}
	wsprintf(buf, TEXT("%s\\*"), path);
	if ((hFind = FindFirstFile(buf, &fd)) != INVALID_HANDLE_VALUE) {
		do {
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ||
				lstrcmp(fd.cFileName, TEXT(".")) == 0 || lstrcmp(fd.cFileName, TEXT("..")) == 0) {
				continue;
			}
			if (lstrlen(path) + lstrlen(fd.cFileName) + 2 > MAX_PATH) {
				continue;
			}
			wsprintf(buf, TEXT("%s\\%s"), path, fd.cFileName);
			remove_empty_dir(buf);
			RemoveDirectory(buf);
		} while (FindNextFile(hFind, &fd) != FALSE);
		FindClose(hFind);
	}
}

/*
 * write_file - ファイルの書き込み (使用中の場合は終了を促す)
 */
static BOOL write_file(const HWND hWnd, const TCHAR *path, const BYTE *buf, const DWORD size)
{
	HANDLE hFile;
	DWORD ret_size;

	for (;;) {
		if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES) {
			SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);
		}
		hFile = CreateFile(path, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			break;
		}
		// 起動中で上書きできない場合は終了を促す
		if (msg_box(hWnd, IDS_FILE_LOCKED, MB_RETRYCANCEL | MB_ICONWARNING, path) != IDRETRY) {
			return FALSE;
		}
	}
	if (size > 0 && WriteFile(hFile, buf, size, &ret_size, NULL) == FALSE) {
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	return TRUE;
}

/*
 * is_running - CLCLが起動中か
 */
static BOOL is_running(void)
{
	return (FindWindow(MAIN_WND_CLASS, MAIN_WINDOW_TITLE) != NULL) ? TRUE : FALSE;
}

/*
 * check_running - CLCLが起動中なら終了を促す
 */
static BOOL check_running(const HWND hWnd)
{
	while (is_running() == TRUE) {
		if (msg_box(hWnd, IDS_RUNNING, MB_RETRYCANCEL | MB_ICONWARNING, NULL) != IDRETRY) {
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * set_language - UI言語の設定 (リソースがある言語のみ)
 */
static void set_language(void)
{
	LANGID langid = GetUserDefaultUILanguage();
	LANGID res_langid = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

	switch (PRIMARYLANGID(langid)) {
	case LANG_JAPANESE:
		res_langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
		break;
	case LANG_GERMAN:
		res_langid = MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN);
		break;
	case LANG_UKRAINIAN:
		res_langid = MAKELANGID(LANG_UKRAINIAN, SUBLANG_DEFAULT);
		break;
	case LANG_CHINESE:
		if (SUBLANGID(langid) == SUBLANG_CHINESE_SIMPLIFIED ||
			SUBLANGID(langid) == SUBLANG_CHINESE_SINGAPORE) {
			res_langid = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
		}
		break;
	}
	SetThreadUILanguage(res_langid);
}

/*
 * reg_open_uninstall - アプリの一覧のキーを開く
 */
static LONG reg_open_uninstall(const HKEY root, const REGSAM view, const REGSAM sam, HKEY *hKey)
{
	return RegOpenKeyEx(root, UNINSTALL_KEY, 0, sam | view, hKey);
}

/*
 * reg_get_string - レジストリから文字列を取得
 */
static BOOL reg_get_string(const HKEY hKey, const TCHAR *name, TCHAR *buf, const DWORD size)
{
	DWORD type;
	DWORD len = size * sizeof(TCHAR);

	*buf = TEXT('\0');
	if (RegQueryValueEx(hKey, name, NULL, &type, (BYTE *)buf, &len) != ERROR_SUCCESS ||
		(type != REG_SZ && type != REG_EXPAND_SZ)) {
		*buf = TEXT('\0');
		return FALSE;
	}
	buf[size - 1] = TEXT('\0');
	return TRUE;
}

/*
 * reg_is_clcl - CLCLの登録かどうか
 */
static BOOL reg_is_clcl(const TCHAR *display_name)
{
	int len = lstrlen(APP_NAME);

	// 「CLCL」または「CLCL 」で始まる表示名を CLCL の登録とみなす
	if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE,
		display_name, len, APP_NAME, len) != CSTR_EQUAL) {
		return FALSE;
	}
	return (*(display_name + len) == TEXT('\0') || *(display_name + len) == TEXT(' ')) ? TRUE : FALSE;
}

/*
 * reg_find_install - 既存のインストールを検索
 */
static int reg_find_install(REG_TARGET *list, const int max_cnt, TCHAR *install_loc)
{
	const HKEY root_list[] = { HKEY_LOCAL_MACHINE, HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };
	const REGSAM view_list[] = { KEY_WOW64_32KEY, KEY_WOW64_64KEY, 0 };
	HKEY hKey, hSubKey;
	TCHAR subkey[BUF_SIZE];
	TCHAR buf[MAX_PATH];
	DWORD index, len;
	int i;
	int cnt = 0;

	if (install_loc != NULL) {
		*install_loc = TEXT('\0');
	}
	for (i = 0; i < (int)(sizeof(root_list) / sizeof(root_list[0])) && cnt < max_cnt; i++) {
		if (reg_open_uninstall(root_list[i], view_list[i], KEY_READ, &hKey) != ERROR_SUCCESS) {
			continue;
		}
		for (index = 0; cnt < max_cnt; index++) {
			len = BUF_SIZE;
			if (RegEnumKeyEx(hKey, index, subkey, &len, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
				break;
			}
			if (RegOpenKeyEx(hKey, subkey, 0, KEY_READ | view_list[i], &hSubKey) != ERROR_SUCCESS) {
				continue;
			}
			if (reg_get_string(hSubKey, TEXT("DisplayName"), buf, MAX_PATH) == TRUE &&
				reg_is_clcl(buf) == TRUE) {
				(list + cnt)->root = root_list[i];
				(list + cnt)->view = view_list[i];
				lstrcpyn((list + cnt)->subkey, subkey, BUF_SIZE);
				if (cnt == 0 && install_loc != NULL) {
					if (reg_get_string(hSubKey, TEXT("InstallLocation"), buf, MAX_PATH) == TRUE) {
						lstrcpyn(install_loc, buf, MAX_PATH);
					}
				}
				cnt++;
			}
			RegCloseKey(hSubKey);
		}
		RegCloseKey(hKey);
	}
	return cnt;
}

/*
 * reg_set_string - レジストリに文字列を設定
 */
static void reg_set_string(const HKEY hKey, const TCHAR *name, const TCHAR *value)
{
	RegSetValueEx(hKey, name, 0, REG_SZ, (const BYTE *)value, (lstrlen(value) + 1) * sizeof(TCHAR));
}

/*
 * reg_set_dword - レジストリに数値を設定
 */
static void reg_set_dword(const HKEY hKey, const TCHAR *name, const DWORD value)
{
	RegSetValueEx(hKey, name, 0, REG_DWORD, (const BYTE *)&value, sizeof(DWORD));
}

/*
 * reg_delete_install - アプリの一覧から削除
 */
static void reg_delete_install(const REG_TARGET *rt)
{
	HKEY hKey;

	if (reg_open_uninstall(rt->root, rt->view, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
		return;
	}
	RegDeleteKeyEx(hKey, rt->subkey, rt->view, 0);
	RegCloseKey(hKey);
}

/*
 * reg_clear_value - キーの値をすべて削除
 */
static void reg_clear_value(const HKEY hKey)
{
	TCHAR name[BUF_SIZE];
	DWORD len;

	// 以前のインストーラが設定した値が残らないようにする
	for (;;) {
		len = BUF_SIZE;
		if (RegEnumValue(hKey, 0, name, &len, NULL, NULL, NULL, NULL) != ERROR_SUCCESS ||
			RegDeleteValue(hKey, name) != ERROR_SUCCESS) {
			break;
		}
	}
}

/*
 * reg_write_install - アプリの一覧に登録
 */
static BOOL reg_write_install(const REG_TARGET *rt)
{
	HKEY hKey, hSubKey;
	SYSTEMTIME st;
	TCHAR buf[MAX_PATH * 2];
	int i;

	if (reg_open_uninstall(rt->root, rt->view, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
		return FALSE;
	}
	if (RegCreateKeyEx(hKey, rt->subkey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_READ | KEY_WRITE | rt->view, NULL, &hSubKey, NULL) != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return FALSE;
	}
	reg_clear_value(hSubKey);
	reg_set_string(hSubKey, TEXT("DisplayName"), APP_NAME);
	reg_set_string(hSubKey, TEXT("DisplayVersion"), ver_str);
	reg_set_string(hSubKey, TEXT("Publisher"), APP_PUBLISHER);
	reg_set_string(hSubKey, TEXT("URLInfoAbout"), APP_URL);
	reg_set_string(hSubKey, TEXT("InstallLocation"), install_path);
	wsprintf(buf, TEXT("%s\\%s,0"), install_path, APP_EXE);
	reg_set_string(hSubKey, TEXT("DisplayIcon"), buf);
	wsprintf(buf, TEXT("\"%s\\%s\" %s"), install_path, UNINSTALL_EXE, CMD_UNINSTALL);
	reg_set_string(hSubKey, TEXT("UninstallString"), buf);
	reg_set_dword(hSubKey, TEXT("NoModify"), 1);
	reg_set_dword(hSubKey, TEXT("NoRepair"), 1);
	reg_set_dword(hSubKey, TEXT("EstimatedSize"), (total_size + 1023) / 1024);
	reg_set_dword(hSubKey, TEXT("VersionMajor"), INST_VER_MAJOR);
	reg_set_dword(hSubKey, TEXT("VersionMinor"), INST_VER_MINOR);
	GetLocalTime(&st);
	wsprintf(buf, TEXT("%04d%02d%02d"), st.wYear, st.wMonth, st.wDay);
	reg_set_string(hSubKey, TEXT("InstallDate"), buf);
	RegCloseKey(hSubKey);
	RegCloseKey(hKey);

	// 二重に登録されている場合は取り除く
	for (i = 0; i < reg_target_cnt; i++) {
		if ((reg_target + i)->root == rt->root && (reg_target + i)->view == rt->view &&
			lstrcmpi((reg_target + i)->subkey, rt->subkey) == 0) {
			continue;
		}
		reg_delete_install(reg_target + i);
	}
	return TRUE;
}

/*
 * create_shortcut - ショートカットの作成
 */
static BOOL create_shortcut(const TCHAR *link_path, const TCHAR *target, const TCHAR *work_dir)
{
	IShellLinkW *psl;
	IPersistFile *ppf;
	BOOL ret = FALSE;

	if (FAILED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		&IID_IShellLinkW, (void **)&psl))) {
		return FALSE;
	}
	IShellLinkW_SetPath(psl, target);
	IShellLinkW_SetWorkingDirectory(psl, work_dir);
	IShellLinkW_SetIconLocation(psl, target, 0);
	IShellLinkW_SetDescription(psl, APP_NAME);
	if (SUCCEEDED(IShellLinkW_QueryInterface(psl, &IID_IPersistFile, (void **)&ppf))) {
		ret = SUCCEEDED(IPersistFile_Save(ppf, link_path, TRUE)) ? TRUE : FALSE;
		IPersistFile_Release(ppf);
	}
	IShellLinkW_Release(psl);
	return ret;
}

/*
 * exec_as_user - エクスプローラ経由で起動 (管理者権限を引き継がない)
 */
static BOOL exec_as_user(const TCHAR *path, const TCHAR *dir)
{
	IShellWindows *psw = NULL;
	IDispatch *pdisp = NULL;
	IServiceProvider *psp = NULL;
	IShellBrowser *psb = NULL;
	IShellView *psv = NULL;
	IDispatch *pdisp_view = NULL;
	IShellFolderViewDual *psfvd = NULL;
	IDispatch *pdisp_app = NULL;
	IShellDispatch2 *psd = NULL;
	VARIANT v_empty, v_dir, v_show;
	BSTR bs_path = NULL, bs_dir = NULL;
	long hwnd = 0;
	BOOL ret = FALSE;

	VariantInit(&v_empty);
	VariantInit(&v_dir);
	VariantInit(&v_show);

	if (FAILED(CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_LOCAL_SERVER,
			&IID_IShellWindows, (void **)&psw)) ||
		FAILED(IShellWindows_FindWindowSW(psw, &v_empty, &v_empty,
			SWC_DESKTOP, &hwnd, SWFO_NEEDDISPATCH, &pdisp)) ||
		FAILED(IDispatch_QueryInterface(pdisp, &IID_IServiceProvider, (void **)&psp)) ||
		FAILED(IServiceProvider_QueryService(psp, &SID_STopLevelBrowser,
			&IID_IShellBrowser, (void **)&psb)) ||
		FAILED(IShellBrowser_QueryActiveShellView(psb, &psv)) ||
		FAILED(IShellView_GetItemObject(psv, SVGIO_BACKGROUND,
			&IID_IDispatch, (void **)&pdisp_view)) ||
		FAILED(IDispatch_QueryInterface(pdisp_view,
			&IID_IShellFolderViewDual, (void **)&psfvd)) ||
		FAILED(IShellFolderViewDual_get_Application(psfvd, &pdisp_app)) ||
		FAILED(IDispatch_QueryInterface(pdisp_app, &IID_IShellDispatch2, (void **)&psd))) {
		goto end;
	}
	if ((bs_path = SysAllocString(path)) == NULL || (bs_dir = SysAllocString(dir)) == NULL) {
		goto end;
	}
	v_dir.vt = VT_BSTR;
	v_dir.bstrVal = bs_dir;
	v_show.vt = VT_I4;
	v_show.lVal = SW_SHOWNORMAL;
	ret = SUCCEEDED(IShellDispatch2_ShellExecute(psd, bs_path,
		v_empty, v_dir, v_empty, v_show)) ? TRUE : FALSE;

end:
	if (bs_path != NULL) {
		SysFreeString(bs_path);
	}
	if (bs_dir != NULL) {
		SysFreeString(bs_dir);
	}
	if (psd != NULL) {
		IShellDispatch2_Release(psd);
	}
	if (pdisp_app != NULL) {
		IDispatch_Release(pdisp_app);
	}
	if (psfvd != NULL) {
		IShellFolderViewDual_Release(psfvd);
	}
	if (pdisp_view != NULL) {
		IDispatch_Release(pdisp_view);
	}
	if (psv != NULL) {
		IShellView_Release(psv);
	}
	if (psb != NULL) {
		IShellBrowser_Release(psb);
	}
	if (psp != NULL) {
		IServiceProvider_Release(psp);
	}
	if (pdisp != NULL) {
		IDispatch_Release(pdisp);
	}
	if (psw != NULL) {
		IShellWindows_Release(psw);
	}
	return ret;
}

/*
 * exec_app - CLCLの起動
 */
static BOOL exec_app(void)
{
	TCHAR path[MAX_PATH];

	wsprintf(path, TEXT("%s\\%s"), install_path, APP_EXE);
	if (exec_as_user(path, install_path) == TRUE) {
		return TRUE;
	}
	return ((INT_PTR)ShellExecute(NULL, NULL, path, NULL, install_path, SW_SHOWNORMAL) > 32) ? TRUE : FALSE;
}

/*
 * load_payload - リソースからインストールするファイルを取得
 */
static BOOL load_payload(void)
{
	HRSRC hRes;
	HGLOBAL hMem;
	const BYTE *data;
	ZIP_ENTRY ze;
	DWORD size;
	DWORD i;

	if ((hRes = FindResource(hInst, MAKEINTRESOURCE(IDR_PAYLOAD), RT_RCDATA)) == NULL ||
		(size = SizeofResource(hInst, hRes)) == 0 ||
		(hMem = LoadResource(hInst, hRes)) == NULL ||
		(data = (const BYTE *)LockResource(hMem)) == NULL) {
		return FALSE;
	}
	if (zip_open(data, size, &zip) == FALSE || zip.count == 0) {
		return FALSE;
	}
	// インストールサイズの取得
	total_size = 0;
	for (i = 0; i < zip.count; i++) {
		if (zip_get_entry(&zip, i, &ze) == TRUE) {
			total_size += ze.orig_size;
		}
	}
	return TRUE;
}

/*
 * log_add - インストールしたファイルを記録
 */
static void log_add(const TCHAR type, const TCHAR *path)
{
	TCHAR buf[MAX_PATH + 4];

	wsprintf(buf, TEXT("%c%s\r\n"), type, path);
	sb_add(&log_buf, buf);
}

/*
 * log_load - インストールしたファイルの一覧を読み込む
 */
static TCHAR *log_load(const TCHAR *dir_path)
{
	HANDLE hFile;
	TCHAR path[MAX_PATH];
	BYTE *buf;
	TCHAR *ret;
	DWORD size, read_size;

	wsprintf(path, TEXT("%s\\%s"), dir_path, UNINSTALL_LOG);
	if ((hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
		return NULL;
	}
	size = GetFileSize(hFile, NULL);
	if (size == 0xffffffff || size < sizeof(TCHAR) ||
		(buf = (BYTE *)HeapAlloc(GetProcessHeap(), 0, size + sizeof(TCHAR))) == NULL) {
		CloseHandle(hFile);
		return NULL;
	}
	if (ReadFile(hFile, buf, size, &read_size, NULL) == FALSE) {
		HeapFree(GetProcessHeap(), 0, buf);
		CloseHandle(hFile);
		return NULL;
	}
	CloseHandle(hFile);
	*((TCHAR *)(buf + (read_size & ~1))) = TEXT('\0');

	ret = (TCHAR *)buf;
	if (*ret == LOG_BOM) {
		ret++;
	}
	return ret;
}

/*
 * log_save - インストールしたファイルの一覧を保存
 */
static BOOL log_save(void)
{
	TCHAR path[MAX_PATH];
	TCHAR bom = LOG_BOM;
	HANDLE hFile;
	DWORD ret_size;

	wsprintf(path, TEXT("%s\\%s"), install_path, UNINSTALL_LOG);
	if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES) {
		SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);
	}
	if ((hFile = CreateFile(path, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	WriteFile(hFile, &bom, sizeof(TCHAR), &ret_size, NULL);
	if (log_buf.buf != NULL) {
		WriteFile(hFile, log_buf.buf, log_buf.len * sizeof(TCHAR), &ret_size, NULL);
	}
	CloseHandle(hFile);
	return TRUE;
}

/*
 * str_to_dword - 文字列を数値に変換
 */
static DWORD str_to_dword(const TCHAR *p)
{
	DWORD ret = 0;

	for (; *p >= TEXT('0') && *p <= TEXT('9'); p++) {
		ret = ret * 10 + (DWORD)(*p - TEXT('0'));
	}
	return ret;
}

/*
 * path_is_safe - 展開先として安全なパスかどうか
 */
static BOOL path_is_safe(const TCHAR *name)
{
	const TCHAR *p;

	if (*name == TEXT('\\') || *name == TEXT('\0') || *(name + 1) == TEXT(':')) {
		return FALSE;
	}
	for (p = name; *p != TEXT('\0'); p++) {
		if (*p != TEXT('.') || *(p + 1) != TEXT('.')) {
			continue;
		}
		if ((p == name || *(p - 1) == TEXT('\\')) &&
			(*(p + 2) == TEXT('\0') || *(p + 2) == TEXT('\\'))) {
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * create_parent_dir - 親フォルダの作成
 */
static void create_parent_dir(const TCHAR *path)
{
	TCHAR buf[MAX_PATH];
	TCHAR *p;

	lstrcpyn(buf, path, MAX_PATH);
	for (p = buf + lstrlen(buf); p > buf; p--) {
		if (*(p - 1) == TEXT('\\')) {
			*(p - 1) = TEXT('\0');
			break;
		}
	}
	if (p > buf && *buf != TEXT('\0')) {
		create_dir(buf);
	}
}

/*
 * get_shortcut_path - ショートカットのパスを取得
 */
static BOOL get_shortcut_path(const int csidl, TCHAR *path)
{
	TCHAR dir[MAX_PATH];

	if (FAILED(SHGetFolderPath(NULL, csidl, NULL, SHGFP_TYPE_CURRENT, dir))) {
		return FALSE;
	}
	if (lstrlen(dir) + lstrlen(APP_NAME) + 6 > MAX_PATH) {
		return FALSE;
	}
	wsprintf(path, TEXT("%s\\%s.lnk"), dir, APP_NAME);
	return TRUE;
}

/*
 * shortcut_exists - ショートカットがあるかどうか
 */
static BOOL shortcut_exists(const int csidl)
{
	TCHAR path[MAX_PATH];

	if (get_shortcut_path(csidl, path) == FALSE) {
		return FALSE;
	}
	return (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES) ? TRUE : FALSE;
}

/*
 * set_shortcut - ショートカットの作成と削除
 */
static void set_shortcut(const int csidl, const BOOL create)
{
	TCHAR path[MAX_PATH];
	TCHAR target[MAX_PATH];

	if (get_shortcut_path(csidl, path) == FALSE) {
		return;
	}
	if (create == FALSE) {
		DeleteFile(path);
		return;
	}
	wsprintf(target, TEXT("%s\\%s"), install_path, APP_EXE);
	if (create_shortcut(path, target, install_path) == TRUE) {
		log_add(TEXT('F'), path);
	}
}

/*
 * copy_file - ファイルのコピー (使用中の場合は終了を促す)
 */
static BOOL copy_file(const HWND hWnd, const TCHAR *from, const TCHAR *to)
{
	for (;;) {
		if (GetFileAttributes(to) != INVALID_FILE_ATTRIBUTES) {
			SetFileAttributes(to, FILE_ATTRIBUTE_NORMAL);
		}
		if (CopyFile(from, to, FALSE) != FALSE) {
			return TRUE;
		}
		if (msg_box(hWnd, IDS_FILE_LOCKED, MB_RETRYCANCEL | MB_ICONWARNING, to) != IDRETRY) {
			return FALSE;
		}
	}
}

/*
 * do_install - インストールの実行
 */
static BOOL do_install(const HWND hDlg)
{
	ZIP_ENTRY ze;
	REG_TARGET rt;
	BYTE *buf;
	DWORD size;
	DWORD i;
	TCHAR path[MAX_PATH];
	TCHAR self[MAX_PATH];
	TCHAR *old_log;
	BOOL ret;

	// CLCLが起動中なら終了を促す
	if (check_running(hDlg) == FALSE) {
		return FALSE;
	}
	// インストール先の作成
	if (create_dir(install_path) == FALSE) {
		msg_box(hDlg, IDS_ERR_CREATE_DIR, MB_OK | MB_ICONEXCLAMATION, install_path);
		return FALSE;
	}
	// 前回インストールしたファイルの一覧を引き継ぐ
	old_log = log_load(install_path);

	// ファイルの展開
	set_ctrl_text(hDlg, IDC_STATIC_STATUS, IDS_STATUS_EXTRACT);
	SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETRANGE32, 0, (LPARAM)zip.count);
	for (i = 0; i < zip.count; i++) {
		SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETPOS, (WPARAM)i, 0);
		if (zip_get_entry(&zip, i, &ze) == FALSE || path_is_safe(ze.name) == FALSE ||
			lstrlen(install_path) + lstrlen(ze.name) + 2 > MAX_PATH) {
			continue;
		}
		wsprintf(path, TEXT("%s\\%s"), install_path, ze.name);
		if (ze.is_dir == TRUE) {
			create_dir(path);
			continue;
		}
		if (zip_extract(&zip, &ze, &buf, &size) != UNZIP_OK) {
			msg_box(hDlg, IDS_ERR_EXTRACT, MB_OK | MB_ICONEXCLAMATION, ze.name);
			return FALSE;
		}
		create_parent_dir(path);
		ret = write_file(hDlg, path, buf, size);
		HeapFree(GetProcessHeap(), 0, buf);
		if (ret == FALSE) {
			return FALSE;
		}
		log_add(TEXT('F'), path);
	}
	SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETPOS, (WPARAM)zip.count, 0);

	// 以前のインストーラがインストール先に残したファイルも削除対象にする
	wsprintf(path, TEXT("%s\\%s"), install_path, OLD_INSTALL_LOG);
	if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES) {
		log_add(TEXT('F'), path);
	}

	// アンインストーラの配置
	GetModuleFileName(NULL, self, MAX_PATH);
	wsprintf(path, TEXT("%s\\%s"), install_path, UNINSTALL_EXE);
	if (copy_file(hDlg, self, path) == FALSE) {
		return FALSE;
	}

	// ショートカットの作成
	set_ctrl_text(hDlg, IDC_STATIC_STATUS, IDS_STATUS_SHORTCUT);
	set_shortcut(CSIDL_COMMON_STARTUP,
		(IsDlgButtonChecked(hDlg, IDC_CHECK_STARTUP) == BST_CHECKED) ? TRUE : FALSE);
	set_shortcut(CSIDL_COMMON_PROGRAMS,
		(IsDlgButtonChecked(hDlg, IDC_CHECK_STARTMENU) == BST_CHECKED) ? TRUE : FALSE);
	set_shortcut(CSIDL_COMMON_DESKTOPDIRECTORY,
		(IsDlgButtonChecked(hDlg, IDC_CHECK_DESKTOP) == BST_CHECKED) ? TRUE : FALSE);

	// アプリの一覧に登録
	set_ctrl_text(hDlg, IDC_STATIC_STATUS, IDS_STATUS_REGIST);
	if (reg_target_cnt > 0) {
		// 二重に登録されないように既存の登録を使用する
		rt = *reg_target;
	} else {
		rt.root = HKEY_LOCAL_MACHINE;
		rt.view = KEY_WOW64_32KEY;
		lstrcpy(rt.subkey, UNINSTALL_SUBKEY);
	}
	if (reg_write_install(&rt) == FALSE) {
		msg_box(hDlg, IDS_ERR_REGIST, MB_OK | MB_ICONEXCLAMATION, NULL);
		return FALSE;
	}

	// インストールしたファイルの一覧を保存
	if (old_log != NULL) {
		sb_add(&log_buf, old_log);
	}
	log_save();
	return TRUE;
}

/*
 * browse_proc - フォルダ選択のコールバック
 */
static int CALLBACK browse_proc(HWND hWnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
	if (msg == BFFM_INITIALIZED) {
		SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
	}
	return 0;
}

/*
 * browse_folder - インストール先の選択
 */
static void browse_folder(const HWND hDlg)
{
	BROWSEINFO bi;
	LPITEMIDLIST pidl;
	TCHAR path[MAX_PATH];
	TCHAR title[MSG_SIZE];
	TCHAR *p;

	GetDlgItemText(hDlg, IDC_EDIT_PATH, path, MAX_PATH);
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = hDlg;
	bi.lpszTitle = str_load(IDS_SELECT_FOLDER, title, MSG_SIZE);
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = browse_proc;
	bi.lParam = (LPARAM)path;
	if ((pidl = SHBrowseForFolder(&bi)) == NULL) {
		return;
	}
	if (SHGetPathFromIDList(pidl, path) != FALSE) {
		// 選択したフォルダ名が異なる場合はフォルダ名を付加
		for (p = path + lstrlen(path); p > path && *(p - 1) != TEXT('\\'); p--)
			;
		if (lstrcmpi(p, DEFAULT_FOLDER) != 0 &&
			lstrlen(path) + lstrlen(DEFAULT_FOLDER) + 2 <= MAX_PATH) {
			if (*(path + lstrlen(path) - 1) != TEXT('\\')) {
				lstrcat(path, TEXT("\\"));
			}
			lstrcat(path, DEFAULT_FOLDER);
		}
		SetDlgItemText(hDlg, IDC_EDIT_PATH, path);
	}
	CoTaskMemFree(pidl);
}

/*
 * enable_ctrl - コントロールの有効と無効
 */
static void enable_ctrl(const HWND hDlg, const BOOL enable)
{
	static const int ctrl_list[] = {
		IDC_EDIT_PATH, IDC_BUTTON_BROWSE, IDC_CHECK_STARTUP,
		IDC_CHECK_STARTMENU, IDC_CHECK_DESKTOP, IDOK, IDCANCEL };
	int i;

	for (i = 0; i < (int)(sizeof(ctrl_list) / sizeof(ctrl_list[0])); i++) {
		EnableWindow(GetDlgItem(hDlg, ctrl_list[i]), enable);
	}
}

/*
 * show_complete - インストール完了の表示
 */
static void show_complete(const HWND hDlg)
{
	static const int ctrl_list[] = {
		IDC_STATIC_FOLDER, IDC_EDIT_PATH, IDC_BUTTON_BROWSE, IDC_CHECK_STARTUP,
		IDC_CHECK_STARTMENU, IDC_CHECK_DESKTOP, IDC_PROGRESS, IDC_STATIC_STATUS, IDCANCEL };
	int i;

	for (i = 0; i < (int)(sizeof(ctrl_list) / sizeof(ctrl_list[0])); i++) {
		ShowWindow(GetDlgItem(hDlg, ctrl_list[i]), SW_HIDE);
	}
	set_ctrl_text(hDlg, IDC_STATIC_DESC, IDS_DESC_COMPLETE);
	set_ctrl_text(hDlg, IDOK, IDS_BUTTON_FINISH);
	// 起動するかの確認
	ShowWindow(GetDlgItem(hDlg, IDC_CHECK_RUN), SW_SHOW);
	CheckDlgButton(hDlg, IDC_CHECK_RUN, BST_CHECKED);
	EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
	SetFocus(GetDlgItem(hDlg, IDOK));
}

/*
 * dlg_proc - インストールのダイアログ
 */
static INT_PTR CALLBACK dlg_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR buf[MSG_SIZE];
	TCHAR fmt[MSG_SIZE];
	TCHAR *p;
	LOGFONT lf;

	switch (msg) {
	case WM_INITDIALOG:
		SendMessage(hDlg, WM_SETICON, ICON_BIG,
			(LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_MAIN)));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL,
			(LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_MAIN)));
		SetWindowText(hDlg, title_str);

		// タイトルは太字で表示
		if (GetObject((HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0), sizeof(LOGFONT), &lf) != 0) {
			lf.lfWeight = FW_BOLD;
			if ((hTitleFont = CreateFontIndirect(&lf)) != NULL) {
				SendDlgItemMessage(hDlg, IDC_STATIC_TITLE, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
			}
		}
		wsprintf(buf, str_load(IDS_HEADER, fmt, MSG_SIZE), ver_str);
		SetDlgItemText(hDlg, IDC_STATIC_TITLE, buf);
		set_ctrl_text(hDlg, IDC_STATIC_DESC,
			(update_mode == TRUE) ? IDS_DESC_UPDATE : IDS_DESC_INSTALL);
		set_ctrl_text(hDlg, IDC_STATIC_FOLDER, IDS_FOLDER);
		set_ctrl_text(hDlg, IDC_BUTTON_BROWSE, IDS_BROWSE);
		set_ctrl_text(hDlg, IDC_CHECK_STARTUP, IDS_CHECK_STARTUP);
		set_ctrl_text(hDlg, IDC_CHECK_STARTMENU, IDS_CHECK_STARTMENU);
		set_ctrl_text(hDlg, IDC_CHECK_DESKTOP, IDS_CHECK_DESKTOP);
		set_ctrl_text(hDlg, IDC_CHECK_RUN, IDS_CHECK_RUN);
		set_ctrl_text(hDlg, IDOK,
			(update_mode == TRUE) ? IDS_BUTTON_UPDATE : IDS_BUTTON_INSTALL);
		set_ctrl_text(hDlg, IDCANCEL, IDS_BUTTON_CANCEL);

		SetDlgItemText(hDlg, IDC_EDIT_PATH, install_path);
		SendDlgItemMessage(hDlg, IDC_EDIT_PATH, EM_LIMITTEXT, MAX_PATH - 1, 0);
		// 更新の場合は現在の状態を引き継ぐ
		CheckDlgButton(hDlg, IDC_CHECK_STARTUP,
			(update_mode == FALSE || shortcut_exists(CSIDL_COMMON_STARTUP) == TRUE) ?
			BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_STARTMENU,
			(update_mode == FALSE || shortcut_exists(CSIDL_COMMON_PROGRAMS) == TRUE) ?
			BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_DESKTOP,
			(update_mode == FALSE || shortcut_exists(CSIDL_COMMON_DESKTOPDIRECTORY) == TRUE) ?
			BST_CHECKED : BST_UNCHECKED);

		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_RUN), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS), SW_HIDE);
		SetDlgItemText(hDlg, IDC_STATIC_STATUS, TEXT(""));
		return TRUE;

	case WM_CLOSE:
		SendMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
		return TRUE;

	case WM_DESTROY:
		if (hTitleFont != NULL) {
			DeleteObject(hTitleFont);
			hTitleFont = NULL;
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_BROWSE:
			browse_folder(hDlg);
			break;

		case IDOK:
			if (install_end == TRUE) {
				// 確認してCLCLを起動
				if (IsDlgButtonChecked(hDlg, IDC_CHECK_RUN) == BST_CHECKED &&
					exec_app() == FALSE) {
					msg_box(hDlg, IDS_ERR_RUN, MB_OK | MB_ICONEXCLAMATION, NULL);
				}
				EndDialog(hDlg, IDOK);
				break;
			}
			GetDlgItemText(hDlg, IDC_EDIT_PATH, install_path, MAX_PATH);
			for (p = install_path + lstrlen(install_path);
				p > install_path && (*(p - 1) == TEXT(' ') || *(p - 1) == TEXT('\\')); p--) {
				*(p - 1) = TEXT('\0');
			}
			if (*install_path == TEXT('\0')) {
				msg_box(hDlg, IDS_ERR_PATH_EMPTY, MB_OK | MB_ICONEXCLAMATION, NULL);
				break;
			}
			SetDlgItemText(hDlg, IDC_EDIT_PATH, install_path);

			enable_ctrl(hDlg, FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS), SW_SHOW);
			SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETPOS, 0, 0);
			SetCursor(LoadCursor(NULL, IDC_WAIT));
			install_end = do_install(hDlg);
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			if (install_end == FALSE) {
				enable_ctrl(hDlg, TRUE);
				ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS), SW_HIDE);
				SetDlgItemText(hDlg, IDC_STATIC_STATUS, TEXT(""));
				break;
			}
			show_complete(hDlg);
			break;

		case IDCANCEL:
			if (install_end == TRUE) {
				EndDialog(hDlg, IDOK);
				break;
			}
			if (IsWindowEnabled(GetDlgItem(hDlg, IDCANCEL)) == FALSE) {
				break;
			}
			if (msg_box(hDlg, IDS_CONFIRM_CANCEL, MB_YESNO | MB_ICONQUESTION, NULL) == IDYES) {
				EndDialog(hDlg, IDCANCEL);
			}
			break;
		}
		return TRUE;
	}
	return FALSE;
}

/*
 * do_uninstall - アンインストールの実行
 */
static void do_uninstall(const TCHAR *dir_path, const DWORD pid, const BOOL delete_data)
{
	HANDLE hProcess;
	TCHAR *log;
	TCHAR *p, *r;
	TCHAR path[MAX_PATH];
	TCHAR type;
	TCHAR wk;
	int i;

	// 呼び出し元の終了を待つ
	if (pid != 0 && (hProcess = OpenProcess(SYNCHRONIZE, FALSE, pid)) != NULL) {
		WaitForSingleObject(hProcess, 30000);
		CloseHandle(hProcess);
	}
	// インストールしたファイルの削除
	if ((log = log_load(dir_path)) != NULL) {
		for (p = log; *p != TEXT('\0');) {
			type = *(p++);
			for (r = p; *p != TEXT('\0') && *p != TEXT('\r') && *p != TEXT('\n'); p++)
				;
			wk = *p;
			*p = TEXT('\0');
			if (type == TEXT('F')) {
				SetFileAttributes(r, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(r);
			} else if (type == TEXT('D')) {
				RemoveDirectory(r);
			}
			*p = wk;
			for (; *p == TEXT('\r') || *p == TEXT('\n'); p++)
				;
		}
	}
	// アンインストーラの削除
	wsprintf(path, TEXT("%s\\%s"), dir_path, UNINSTALL_LOG);
	SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);
	DeleteFile(path);
	wsprintf(path, TEXT("%s\\%s"), dir_path, UNINSTALL_EXE);
	SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);
	DeleteFile(path);
	remove_empty_dir(dir_path);
	RemoveDirectory(dir_path);

	// アプリの一覧から削除
	reg_target_cnt = reg_find_install(reg_target, REG_TARGET_MAX, NULL);
	for (i = 0; i < reg_target_cnt; i++) {
		reg_delete_install(reg_target + i);
	}
	// 設定データの削除
	if (delete_data == TRUE &&
		SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path)) &&
		lstrlen(path) + lstrlen(APP_DATA_FOLDER) + 2 <= MAX_PATH) {
		lstrcat(path, TEXT("\\"));
		lstrcat(path, APP_DATA_FOLDER);
		delete_dir_tree(path);
	}
	msg_box(NULL, IDS_UNINSTALL_COMPLETE, MB_OK | MB_ICONINFORMATION, NULL);

	// 一時フォルダにコピーした自分自身を削除
	GetModuleFileName(NULL, path, MAX_PATH);
	MoveFileEx(path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
}

/*
 * get_uninstall_dir - アンインストールするフォルダの取得
 */
static BOOL get_uninstall_dir(TCHAR *dir_path)
{
	TCHAR path[MAX_PATH];

	// 実行ファイルと同じ場所にファイルの一覧があるか
	get_module_dir(dir_path);
	wsprintf(path, TEXT("%s\\%s"), dir_path, UNINSTALL_LOG);
	if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES) {
		return TRUE;
	}
	// 登録されているインストール先を使用する
	reg_target_cnt = reg_find_install(reg_target, REG_TARGET_MAX, path);
	if (reg_target_cnt > 0 && *path != TEXT('\0') &&
		GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES) {
		lstrcpyn(dir_path, path, MAX_PATH);
		return TRUE;
	}
	return FALSE;
}

/*
 * start_uninstall - アンインストールの開始
 */
static BOOL start_uninstall(void)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	TCHAR dir_path[MAX_PATH];
	TCHAR self[MAX_PATH];
	TCHAR temp_path[MAX_PATH];
	TCHAR cmd[MAX_PATH * 3];
	BOOL delete_data = FALSE;

	if (get_uninstall_dir(dir_path) == FALSE) {
		msg_box(NULL, IDS_ERR_NOT_INSTALLED, MB_OK | MB_ICONEXCLAMATION, NULL);
		return FALSE;
	}
	if (msg_box(NULL, IDS_CONFIRM_UNINSTALL, MB_YESNO | MB_ICONQUESTION, NULL) != IDYES) {
		return FALSE;
	}
	// CLCLが起動中なら終了を促す
	if (check_running(NULL) == FALSE) {
		return FALSE;
	}
	if (msg_box(NULL, IDS_CONFIRM_DELETE_DATA, MB_YESNO | MB_ICONQUESTION, NULL) == IDYES) {
		delete_data = TRUE;
	}
	// 実行中のファイルを削除できるように一時フォルダから実行する
	GetModuleFileName(NULL, self, MAX_PATH);
	GetTempPath(MAX_PATH, temp_path);
	wsprintf(temp_path + lstrlen(temp_path), TEXT("clcl_uninst_%08x.exe"), GetTickCount());
	if (CopyFile(self, temp_path, FALSE) == FALSE) {
		return FALSE;
	}
	wsprintf(cmd, TEXT("\"%s\" %s \"%s\" %s%u"),
		temp_path, CMD_UNINSTALL_RUN, dir_path, CMD_PID, GetCurrentProcessId());
	if (delete_data == TRUE) {
		lstrcat(cmd, TEXT(" "));
		lstrcat(cmd, CMD_DELETE_DATA);
	}
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) == FALSE) {
		DeleteFile(temp_path);
		return FALSE;
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return TRUE;
}

/*
 * WinMain - メイン
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX ic;
	LPWSTR *argv;
	TCHAR dir_path[MAX_PATH];
	TCHAR buf[MAX_PATH];
	DWORD pid = 0;
	int argc = 0;
	int i;
	int mode = 0;
	BOOL delete_data = FALSE;

	hInst = hInstance;
	set_language();
	wsprintf(ver_str, TEXT("%d.%d.%d"), INST_VER_MAJOR, INST_VER_MINOR, INST_VER_REVISION);

	*dir_path = TEXT('\0');
	if ((argv = CommandLineToArgvW(GetCommandLineW(), &argc)) != NULL) {
		for (i = 1; i < argc; i++) {
			if (lstrcmpi(*(argv + i), CMD_UNINSTALL) == 0) {
				mode = 1;
			} else if (lstrcmpi(*(argv + i), CMD_UNINSTALL_RUN) == 0) {
				mode = 2;
				if (i + 1 < argc) {
					lstrcpyn(dir_path, *(argv + (++i)), MAX_PATH);
				}
			} else if (lstrcmpi(*(argv + i), CMD_DELETE_DATA) == 0) {
				delete_data = TRUE;
			} else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE,
				*(argv + i), lstrlen(CMD_PID), CMD_PID, lstrlen(CMD_PID)) == CSTR_EQUAL) {
				pid = str_to_dword(*(argv + i) + lstrlen(CMD_PID));
			}
		}
		LocalFree(argv);
	}

	str_load((mode == 0) ? IDS_SETUP_TITLE : IDS_UNINSTALL_TITLE, title_str, BUF_SIZE);
	CoInitialize(NULL);
	ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ic.dwICC = ICC_STANDARD_CLASSES | ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&ic);

	switch (mode) {
	case 1:
		start_uninstall();
		break;

	case 2:
		if (*dir_path != TEXT('\0')) {
			do_uninstall(dir_path, pid, delete_data);
		}
		break;

	default:
		// インストールするファイルの取得
		if (load_payload() == FALSE) {
			msg_box(NULL, IDS_ERR_PAYLOAD, MB_OK | MB_ICONEXCLAMATION, NULL);
			break;
		}
		// 既にインストールされている場合は更新
		reg_target_cnt = reg_find_install(reg_target, REG_TARGET_MAX, buf);
		if (reg_target_cnt > 0) {
			update_mode = TRUE;
		}
		if (*buf != TEXT('\0')) {
			lstrcpyn(install_path, buf, MAX_PATH);
		} else {
			if (FAILED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL,
				SHGFP_TYPE_CURRENT, install_path))) {
				lstrcpy(install_path, TEXT("C:\\Program Files"));
			}
			lstrcat(install_path, TEXT("\\"));
			lstrcat(install_path, DEFAULT_FOLDER);
		}
		DialogBox(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, dlg_proc);
		sb_free(&log_buf);
		break;
	}
	CoUninitialize();
	return 0;
}
/* End of source */
