/*
 * CLCL Installer
 *
 * unzip.h
 *
 * Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_CLCL_UNZIP_H
#define _INC_CLCL_UNZIP_H

/* Define */
#define UNZIP_OK					0
#define UNZIP_ERR_FORMAT			1
#define UNZIP_ERR_MEMORY			2
#define UNZIP_ERR_METHOD			3
#define UNZIP_ERR_DATA				4
#define UNZIP_ERR_CRC				5

/* Struct */
typedef struct _ZIP_ARCHIVE {
	const BYTE *data;					// ZIP全体の先頭
	DWORD size;							// ZIP全体のサイズ
	const BYTE *central;				// セントラルディレクトリの先頭
	DWORD count;						// エントリ数
} ZIP_ARCHIVE;

typedef struct _ZIP_ENTRY {
	TCHAR name[MAX_PATH];				// 格納パス (区切りは '\\' に正規化)
	DWORD method;						// 圧縮方式
	DWORD comp_size;					// 圧縮後サイズ
	DWORD orig_size;					// 圧縮前サイズ
	DWORD crc;							// CRC32
	DWORD local_offset;					// ローカルヘッダの位置
	BOOL is_dir;						// フォルダ
} ZIP_ENTRY;

/* Function Prototypes */
BOOL zip_open(const BYTE *data, const DWORD size, ZIP_ARCHIVE *za);
BOOL zip_get_entry(const ZIP_ARCHIVE *za, const DWORD index, ZIP_ENTRY *ze);
int zip_extract(const ZIP_ARCHIVE *za, const ZIP_ENTRY *ze, BYTE **buf, DWORD *buf_size);

#endif
/* End of source */
