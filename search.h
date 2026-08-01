/*
 * CLCL
 *
 * search.h
 *
 * Copyright (C) 2026 by Wilf Zimmermann. MIT License.
 *		https://linguversa.de/clcl
 *		https://github.com/wilfz/clcl
 */
#pragma once

#ifndef _INC_CLCL_SEARCH_H
#define _INC_CLCL_SEARCH_H

/* Include Files */
#include "Data.h"

/* Define */

/* Struct */

/* Function Prototypes */
void search_item_dlg( HWND hWnd, DATA_INFO* item);
void search_next( HWND hWnd);
void search_del(DATA_INFO* del_di);
void search_free(void);

#endif
/* End of source */
