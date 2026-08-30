/*
 * CLCL
 *
 * dynamic_popup.c
 *
 * Copyright (C) 2026 by Wilf Zimmermann. MIT License.
 *		https://linguversa.de/clcl
 *		https://github.com/wilfz/clcl
 */

/* Include Files */
#include "dynamic_popup.h"
#include <tchar.h>
#include <stdio.h>

#define SUBCLASS_ID_POPUP 202
#define ITEM_HEIGHT 20
#define MAX_VISIBLE_ITEMS 10
#define TOOLTIP_DELAY_MS 800
#ifndef IDC_TOOLTIP_WINDOW
#define IDC_TOOLTIP_WINDOW 51004
#endif

// Zustandskontrolle für das dynamische Popup
typedef struct {
    HWND hwndFrame;           // Das neue unsichtbare Container-Fenster (hat den Schatten!)
    HWND hwndEdit;            // Das eigentliche Edit Control (als Child)
    HWND hwndList;            // Das Listbox Control (als Child)
    HWND hwndTooltip;         // Tooltip-Fenster
    HWND hwndOwner;
    HFONT hFont;
    OnPopupPopulateCallback populateCallback;
    OnPopupSelectCallback selectCallback;
    OnPopupTooltipCallback tooltipCallback;  // Neuer Callback für Multiline-Tooltips
    unsigned int max_visible_items;
    void* pUserData;
    BOOL isClosing;
    BOOL listAboveEdit;
    RECT monitorRect;
    HIMAGELIST hImageList;    // Gespeicherte ImageList für das Zeichnen der Icons
    int icon_size;
    int icon_margin;
    int text_margin;
    int item_height;
    int lastHoveredItem;      // Verfolgung des zuletzt angezeigten Tooltip-Elements
    UINT_PTR uiTooltipTimer;  // Timer-ID für Tooltip-Verzögerung
    TCHAR* currentTooltipText; // Aktueller Tooltip-Text für WM_PAINT
} DynamicPopupData;

// Vorwärtsdeklarationen
LRESULT CALLBACK PopupFrameWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DynamicEditSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK DynamicListSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK TooltipWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Hilfsfunktion: Versteckt das Tooltip-Fenster
static void HideTooltip(DynamicPopupData* pData)
{
    if (pData && pData->hwndTooltip) {
        ShowWindow(pData->hwndTooltip, SW_HIDE);
        if (pData->uiTooltipTimer) {
            KillTimer(pData->hwndList, pData->uiTooltipTimer);
            pData->uiTooltipTimer = 0;
        }
        pData->lastHoveredItem = -1;
    }
}

// Hilfsfunktion: Zeigt ein Multiline-Tooltip an
static void ShowTooltipForItem(DynamicPopupData* pData, int itemIndex, POINT ptMouse)
{
    if (!pData || !pData->hwndList || !pData->tooltipCallback) {
        return;
    }

    if (itemIndex == LB_ERR) {
        HideTooltip(pData);
        return;
    }

    // Beende bestehenden Timer
    if (pData->uiTooltipTimer) {
        KillTimer(pData->hwndList, pData->uiTooltipTimer);
        pData->uiTooltipTimer = 0;
    }

    // Nicht erneut anzeigen, wenn über demselben Element
    if (pData->lastHoveredItem == itemIndex) {
        return;
    }

    pData->lastHoveredItem = itemIndex;

    PopupItemData* pItem = (PopupItemData*)SendMessage(pData->hwndList, LB_GETITEMDATA, itemIndex, 0);
    if (!pItem || pItem == (PopupItemData*)LB_ERR) {
        HideTooltip(pData);
        return;
    }

    // Tooltip-Text vom Callback abrufen
    TCHAR* tooltipText = pData->tooltipCallback(pItem, pData->pUserData);
    if (!tooltipText || *tooltipText == TEXT('\0')) {
        HideTooltip(pData);
        return;
    }

    // Tooltip-Fenster erstellen, falls nicht vorhanden
    if (!pData->hwndTooltip) {
        const TCHAR* szTooltipClass = TEXT("HCP_TooltipClass");
        static BOOL tooltipClassRegistered = FALSE;

        if (!tooltipClassRegistered) {
            WNDCLASS wc = { 0 };
            wc.lpfnWndProc = TooltipWindowProc;
            wc.hInstance = (HINSTANCE)GetWindowLongPtr(pData->hwndList, GWLP_HINSTANCE);
            wc.hbrBackground = (HBRUSH)(COLOR_INFOBK + 1);
            wc.lpszClassName = szTooltipClass;
            wc.style = CS_SAVEBITS;
            if (RegisterClass(&wc)) {
                tooltipClassRegistered = TRUE;
            }
        }

        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(pData->hwndList, GWLP_HINSTANCE);
        pData->hwndTooltip = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
            szTooltipClass, NULL,
            WS_POPUP | WS_BORDER,
            ptMouse.x + 15, ptMouse.y + 15, 300, 100,
            NULL, NULL, hInstance, NULL
        );

        if (!pData->hwndTooltip) {
            return;
        }

        // Speichere Pointer zu pData im Fenster
        SetWindowLongPtr(pData->hwndTooltip, GWLP_USERDATA, (LONG_PTR)pData);

        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(pData->hwndTooltip, WM_SETFONT, (WPARAM)hFont, FALSE);
    }

    // Speichere den Tooltip-Text in pData
    if (pData->currentTooltipText) {
        HeapFree(GetProcessHeap(), 0, pData->currentTooltipText);
    }
    size_t len = _tcslen(tooltipText) + 1;
    pData->currentTooltipText = (TCHAR*)HeapAlloc(GetProcessHeap(), 0, len * sizeof(TCHAR));
    if (pData->currentTooltipText) {
        _tcscpy_s(pData->currentTooltipText, len, tooltipText);
    }

    // Tooltip-Größe berechnen
    HDC hdc = GetDC(pData->hwndTooltip);
    HFONT hFont = (HFONT)SendMessage(pData->hwndTooltip, WM_GETFONT, 0, 0);
    HFONT oldFont = hFont ? (HFONT)SelectObject(hdc, hFont) : NULL;

    if (!pData->currentTooltipText) {
        ReleaseDC(pData->hwndTooltip, hdc);
        return;
	}
    RECT rcText = { 0, 0, 280, 1000 };
    DrawText(hdc, pData->currentTooltipText, -1, &rcText, DT_CALCRECT | DT_WORDBREAK | DT_LEFT);

    if (oldFont) SelectObject(hdc, oldFont);
    ReleaseDC(pData->hwndTooltip, hdc);

    int tooltipWidth = rcText.right - rcText.left + 10;
    int tooltipHeight = rcText.bottom - rcText.top + 10;

    // Tooltip positionieren und anzeigen
    SetWindowPos(pData->hwndTooltip, HWND_TOPMOST,
        ptMouse.x + 15, ptMouse.y + 15,
        tooltipWidth, tooltipHeight,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
 
    // Force repaint
    InvalidateRect(pData->hwndTooltip, NULL, TRUE);
    UpdateWindow(pData->hwndTooltip);
}

// Custom Window Procedure für Tooltip
LRESULT CALLBACK TooltipWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps = { 0 };
        HDC hdc = BeginPaint(hWnd, &ps);

        if (pData && pData->currentTooltipText) {
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            // Hintergrund füllen
            HBRUSH hBg = GetSysColorBrush(COLOR_INFOBK);
            FillRect(hdc, &rcClient, hBg);

            // Text zeichnen
            HFONT hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
            HFONT oldFont = hFont ? (HFONT)SelectObject(hdc, hFont) : NULL;

            COLORREF oldTextColor = SetTextColor(hdc, GetSysColor(COLOR_INFOTEXT));
            int oldBkMode = SetBkMode(hdc, TRANSPARENT);

            RECT rcText = rcClient;
            rcText.left += 5;
            rcText.top += 5;
            rcText.right -= 5;
            rcText.bottom -= 5;

            DrawText(hdc, pData->currentTooltipText, -1, &rcText, DT_WORDBREAK | DT_LEFT);

            SetBkMode(hdc, oldBkMode);
            SetTextColor(hdc, oldTextColor);
            if (oldFont) SelectObject(hdc, oldFont);
        }

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        if (pData && pData->currentTooltipText) {
            HeapFree(GetProcessHeap(), 0, pData->currentTooltipText);
            pData->currentTooltipText = NULL;
        }
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Hilfsfunktion: Gibt den Speicher aller Listbox-Einträge frei
static void ClearListBoxItems(HWND hwndList) 
{
    int count = (int)SendMessage(hwndList, LB_GETCOUNT, 0, 0);
    if (count == LB_ERR) return;

    for (int i = 0; i < count; i++) {
        PopupItemData* pItem = (PopupItemData*)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
        if (pItem && pItem != (PopupItemData*)LB_ERR) {
            if (pItem->pszText) HeapFree(GetProcessHeap(), 0, pItem->pszText);
            HeapFree(GetProcessHeap(), 0, pItem);
        }
    }
    SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
}

void PopupAddString(HWND hwndListBox, const TCHAR* pszText, int iIconIndex, UINT_PTR itemData) 
{
    PopupItemData* pItem = (PopupItemData*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PopupItemData));
    if (!pItem) return;

    if (pszText) {
        size_t len = _tcslen(pszText) + 1;
        pItem->pszText = (TCHAR*)HeapAlloc(GetProcessHeap(), 0, len * sizeof(TCHAR));
        if (pItem->pszText) {
            _tcscpy_s(pItem->pszText, len, pszText);
        }
    }
    pItem->iIconIndex = iIconIndex;
    pItem->itemData = itemData;

    // Da LBS_OWNERDRAWFIXED aktiv ist, übergeben wir den Pointer als String-Parameter.
    // Windows speichert diesen automatisch als ItemData, da LBS_HASSTRINGS NICHT gesetzt ist.
    SendMessage(hwndListBox, LB_ADDSTRING, 0, (LPARAM)pItem);
}

static void DestroyPopupLayout(DynamicPopupData* pData) 
{
    if (!pData || pData->isClosing) return;
    pData->isClosing = TRUE;

    HWND hFrame = pData->hwndFrame;
    HWND hEdit = pData->hwndEdit;
    HWND hList = pData->hwndList;
    HWND hTooltip = pData->hwndTooltip;

    HideTooltip(pData);

    RemoveWindowSubclass(hEdit, DynamicEditSubclass, SUBCLASS_ID_POPUP);
    RemoveWindowSubclass(hList, DynamicListSubclass, SUBCLASS_ID_POPUP);

    ClearListBoxItems(hList);

    if (hTooltip) DestroyWindow(hTooltip);
    ShowWindow(hFrame, SW_HIDE);
    DestroyWindow(hFrame);

    if (pData->currentTooltipText) {
        HeapFree(GetProcessHeap(), 0, pData->currentTooltipText);
    }

    HeapFree(GetProcessHeap(), 0, pData);
}

static void RepositionListbox(DynamicPopupData* pData, int editHeight) 
{
	if (!pData || !pData->hwndFrame || !pData->hwndEdit || !pData->hwndList)
        return;

    int itemCount = (int)SendMessage(pData->hwndList, LB_GETCOUNT, 0, 0);
    int visibleItems = itemCount > MAX_VISIBLE_ITEMS ? MAX_VISIBLE_ITEMS : itemCount;
    int listHeight = 0;
	int maxListHeight = ITEM_HEIGHT * MAX_VISIBLE_ITEMS;
    if (visibleItems > 0 && pData && pData->item_height > 0) {
        listHeight = pData->item_height * visibleItems;
		maxListHeight = pData->item_height * MAX_VISIBLE_ITEMS;
    }
    
    // Get screen coordinates of the frame window
    RECT frameRect;
    GetWindowRect(pData->hwndFrame, &frameRect);
    int width = frameRect.right - frameRect.left + 1;
    
    RECT editRect;
    GetWindowRect(pData->hwndEdit, &editRect);

    // Get monitor info to check available space
    //HMONITOR hMonitor = MonitorFromRect(&frameRect, MONITOR_DEFAULTTONEAREST);
    //MONITORINFO miInfo = { 0 };
    //miInfo.cbSize = sizeof(MONITORINFO);
    //GetMonitorInfo(hMonitor, &miInfo);
    int monitorBottom = pData->monitorRect.bottom;

    if (frameRect.left < pData->monitorRect.left) {
        // move the frame window to the right to fit within the monitor
		frameRect.left = pData->monitorRect.left;
		frameRect.right = pData->monitorRect.left + width + 1;
    } 
    else if (frameRect.right > pData->monitorRect.right) {
		// move the frame window to the left to fit within the monitor
        frameRect.left = pData->monitorRect.right - width  - 1;
		frameRect.right = pData->monitorRect.right;
	}
    if (editRect.top < pData->monitorRect.top) {
        // move the frame window down to fit within the monitor
        frameRect.top = pData->monitorRect.top;
    } else if (editRect.bottom > pData->monitorRect.bottom) {
        // move the frame window up to fit within the monitor
        frameRect.bottom = pData->monitorRect.bottom;
	}  
    
    // Adjust frame window size
    int totalHeight = editHeight + listHeight;

    // Check if listbox fits below the edit control
    BOOL fitsBelowEdit = (monitorBottom >= editRect.top + editHeight + maxListHeight);
    
    // If doesn't fit below and there's more space above, position above
    if (!fitsBelowEdit) {
        // Position listbox above the edit control
        SetWindowPos(pData->hwndList, NULL, 0, 0, width, listHeight, SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowPos(pData->hwndEdit, NULL, 0, listHeight + 1, width, editHeight, SWP_NOZORDER);
        pData->listAboveEdit = TRUE;
    } else {
        // Position listbox below the edit control (default)
        SetWindowPos(pData->hwndEdit, NULL, 0, 0, width, editHeight, SWP_NOZORDER);
        SetWindowPos(pData->hwndList, NULL, 0, editHeight + 1, width, listHeight, SWP_NOZORDER | SWP_NOACTIVATE);
        pData->listAboveEdit = FALSE;
    }
    
    int frameY = pData->listAboveEdit ? frameRect.bottom - totalHeight : frameRect.top;
    SetWindowPos(pData->hwndFrame, NULL, frameRect.left, frameY, width, totalHeight, SWP_NOZORDER | SWP_NOACTIVATE);
}

static void UpdateListContent(DynamicPopupData* pData) 
{
    int len = GetWindowTextLength(pData->hwndEdit);
    TCHAR* text = (TCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (len + 1) * sizeof(TCHAR));
    if (text) {
        GetWindowText(pData->hwndEdit, text, len + 1);

        // Vor dem Befüllen alten Inhalt und dessen Heap-Objekte löschen
        ClearListBoxItems(pData->hwndList);
        HideTooltip(pData);

        if (pData->populateCallback) {
            pData->populateCallback(text, pData->hwndList, pData->pUserData);
        }
        HeapFree(GetProcessHeap(), 0, text);
    }

    if (SendMessage(pData->hwndList, LB_GETCOUNT, 0, 0) > 0) {
        SendMessage(pData->hwndList, LB_SETCURSEL, 0, 0);
    }
    
    // Reposition listbox based on available space
    RepositionListbox(pData, 26);
}

HWND CreateDynamicPopupMenu(HWND hwndOwner, int x, int y, int width)
 {
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwndOwner, GWLP_HINSTANCE);
    const TCHAR* szClassName = TEXT("HCP_PopupMenuFrameClass");

    // Eigene, saubere Fensterklasse registrieren (NUR EINMAL)
    static BOOL classRegistered = FALSE;
    if (!classRegistered) {
        WNDCLASS wc = { 0 };
        wc.lpfnWndProc = PopupFrameWndProc;
        wc.hInstance = hInstance;
        wc.hbrBackground = (HBRUSH)(COLOR_MENU + 1); // Menü-Hintergrundfarbe
        wc.lpszClassName = szClassName;
        wc.style = CS_DROPSHADOW;

        if (RegisterClass(&wc)) {
            classRegistered = TRUE;
        }
    }

    // Get monitor info to check available space
    POINT pt = { x, y };
    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO miInfo = { 0 };
    miInfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, &miInfo);

    int editHeight = 26;
    int initialListHeight = 0; //ITEM_HEIGHT * 3;
    int totalHeight = editHeight + initialListHeight;

    // 1. Das übergeordnete POPUP-Fenster erstellen
    HWND hwndFrame = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        szClassName, NULL,
        WS_POPUP | WS_BORDER,
        x, y, width, totalHeight,
        hwndOwner, NULL, hInstance, NULL
    );

    if (!hwndFrame) return NULL;

    // Speicher für Daten reservieren
    DynamicPopupData* pData = (DynamicPopupData*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DynamicPopupData));
    if (!pData) {
        DestroyWindow(hwndFrame);
        return NULL;
    }

    // 2. Das Edit-Control als CHILD im Frame erstellen
    HWND hwndEdit = CreateWindowEx(
        0, TEXT("EDIT"), TEXT(""),
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0, 0, width, editHeight, 
        hwndFrame, (HMENU)IDC_DYNAMIC_EDIT, hInstance, NULL
    );

    // WICHTIG: LBS_OWNERDRAWFIXED ohne LBS_HASSTRINGS. Dadurch ist der "String" die Pointer-Adresse.
    HWND hwndList = CreateWindowEx(
        0, TEXT("LISTBOX"), NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_OWNERDRAWFIXED,
        0, editHeight, width, initialListHeight,
        hwndFrame, (HMENU)IDC_DYNAMIC_LISTBOX, hInstance, NULL
    );

    if (!hwndEdit || !hwndList) {
        DestroyWindow(hwndFrame);
        HeapFree(GetProcessHeap(), 0, pData);
        return NULL;
    }

    // Zeiger auf Datenstruktur im Frame-Fenster hinterlegen
    SetWindowLongPtr(hwndFrame, GWLP_USERDATA, (LONG_PTR)pData);

    pData->hwndFrame = hwndFrame;
    pData->hwndEdit = hwndEdit;
    pData->hwndList = hwndList;
    pData->hwndTooltip = NULL;
    pData->hwndOwner = hwndOwner;
    pData->hFont = (HFONT)NULL;
    pData->isClosing = FALSE;
    pData->listAboveEdit = FALSE;
    pData->monitorRect = miInfo.rcWork;
    pData->populateCallback = NULL;
    pData->selectCallback = NULL;
    pData->tooltipCallback = NULL;
    pData->max_visible_items = MAX_VISIBLE_ITEMS;
    pData->hImageList = (HIMAGELIST) NULL;
    pData->icon_size = 0;
    pData->item_height = ITEM_HEIGHT;
    pData->text_margin = 4;
	pData->pUserData = NULL;
    pData->lastHoveredItem = -1;
    pData->uiTooltipTimer = 0;
    pData->currentTooltipText = NULL;

    // Subclassing für Controls aktivieren
    SetWindowSubclass(pData->hwndEdit, DynamicEditSubclass, SUBCLASS_ID_POPUP, (DWORD_PTR)pData);
    SetWindowSubclass(pData->hwndList, DynamicListSubclass, SUBCLASS_ID_POPUP, (DWORD_PTR)pData);

    return hwndFrame;
}

void SetPopulateCallback(HWND hwndFrame, OnPopupPopulateCallback populateCallback)
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (pData) {
        pData->populateCallback = populateCallback;
    }
}

void SetSelectCallback(HWND hwndFrame, OnPopupSelectCallback selectCallback)
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (pData) {
        pData->selectCallback = selectCallback;
    }
}

void SetTooltipCallback(HWND hwndFrame, OnPopupTooltipCallback tooltipCallback)
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (pData) {
        pData->tooltipCallback = tooltipCallback;
    }
}

void SetImageList(HWND hwndFrame, HIMAGELIST hImageList, int icon_size)
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (!pData)
        return;

        pData->hImageList = hImageList;
        pData->icon_size = icon_size;

    if (icon_size == 0) {
        int cx = 0, cy = 0;
        if (pData && pData->hImageList && ImageList_GetIconSize(pData->hImageList, &cx, &cy))
            icon_size = (cy > 0) ? cy : 16;
    }

    pData->hImageList = hImageList;
    pData->icon_size = icon_size;
}

void SetIconSize(HWND hwndFrame, int icon_size)
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (pData)
        pData->icon_size = icon_size;
}

void SetIconMargin(HWND hwndFrame, int icon_margin)
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (pData)
        pData->icon_margin = icon_margin;
}

void SetItemHeight(HWND hwndFrame, int item_height) 
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (pData)
        pData->item_height = item_height;
}

void SetTextMargin(HWND hwndFrame, int text_margin)
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (pData)
        pData->text_margin = text_margin;
}

void SetMaxVisibleItems(HWND hwndFrame, unsigned int max_visible_items)
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (pData)
        pData->max_visible_items = max_visible_items;
}

void SetMenuFont(HWND hwndFrame, HFONT hFont)
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (pData) {
        pData->hFont = hFont;
    }
}

void SetUserData(HWND hwndFrame, void* pUserData) 
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (pData) {
        pData->pUserData = pUserData;
    }
}

void ActivateDynamicPopup(HWND hwndFrame) 
{
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
	if (pData == NULL)
        return;

    // Modernere System-Schriftart zuweisen
    HFONT hFont = pData->hFont ? pData->hFont : (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(pData->hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(pData->hwndList, WM_SETFONT, (WPARAM)hFont, TRUE);

    if (pData->item_height < pData->icon_margin + pData->icon_size + pData->icon_margin) {
        // Höhe des Listbox-Items basierend auf Icon-Größe
        pData->item_height = pData->icon_margin + pData->icon_size + pData->icon_margin;
        }
    SendMessage(pData->hwndList, LB_SETITEMHEIGHT, 0, (LPARAM)pData->item_height);

    // Initial befüllen
    UpdateListContent(pData);

    ShowWindow(pData->hwndFrame, SW_SHOW);
    SetFocus(pData->hwndEdit);

    // Select the first item
    int sel = 0;
    LRESULT ret = SendMessage(pData->hwndList, LB_SETCURSEL, (WPARAM)sel, (LPARAM)0);
    if (ret >= 0 && pData->tooltipCallback != NULL) {
        // Show tooltip for the selected item
        RECT itemrect;
        ret = SendMessage(pData->hwndList, LB_GETITEMRECT, (WPARAM)sel, (LPARAM)&itemrect);
        if (ret >= 0) {
            POINT pt;
            pt.x = (itemrect.left + itemrect.right) / 2;
            pt.y = (itemrect.top + itemrect.bottom) / 2;
            if (ClientToScreen(pData->hwndList, &pt))
                ShowTooltipForItem(pData, sel, pt);
        }
    }

    return;
}

// Fensterprozedur für das äußere Frame-Fenster
LRESULT CALLBACK PopupFrameWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (uMsg) {
        case WM_COMMAND:
            if (pData && pData->hwndEdit
                && LOWORD(wParam) == IDC_DYNAMIC_EDIT
                && HIWORD(wParam) == EN_CHANGE)
            {
                UpdateListContent(pData);
            }
            break;

        case WM_DRAWITEM: {
            DRAWITEMSTRUCT* pdis = (DRAWITEMSTRUCT*)lParam;
            if (pdis->CtlID == IDC_DYNAMIC_LISTBOX && pdis->itemID != -1) {
                PopupItemData* pItem = (PopupItemData*)pdis->itemData;
                if (!pItem) return TRUE;

                HDC hdc = pdis->hDC;
                RECT rc = pdis->rcItem;
                BOOL isSelected = (pdis->itemState & ODS_SELECTED);

                // 1. Hintergrund zeichnen (Selektiert vs. Standard)
                HBRUSH hBg = GetSysColorBrush(isSelected ? COLOR_HIGHLIGHT : COLOR_MENU);
                FillRect(hdc, &rc, hBg);

                // 2. Icon zeichnen (falls ImageList und gültiger Index vorhanden)
                if (pData && pData->hImageList && pItem->iIconIndex >= 0 && pData->icon_size > 0) {
                    // Vertikal zentrieren
                    int cy = rc.top + (rc.bottom - rc.top - pData->icon_size) / 2;
                    ImageList_Draw(pData->hImageList, pItem->iIconIndex, hdc, rc.left + pData->icon_margin, cy, ILD_TRANSPARENT);
                }
                int iconOffset = (pData && pData->icon_size > 0) ? pData->icon_margin + pData->icon_size + pData->icon_margin : 0;

                // 3. Text zeichnen
                COLORREF oldTextCol = SetTextColor(hdc, GetSysColor(isSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));
                int oldBkMode = SetBkMode(hdc, TRANSPARENT);

                RECT rcText = rc;
                rcText.left += iconOffset + (pData ? pData->text_margin : 2);

                HFONT hFont = (HFONT)SendMessage(pdis->hwndItem, WM_GETFONT, 0, 0);
                HFONT oldFont = hFont ? (HFONT)SelectObject(hdc, hFont) : NULL;

                DrawText(hdc, pItem->pszText, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

                if (oldFont) SelectObject(hdc, oldFont);
                SetBkMode(hdc, oldBkMode);
                SetTextColor(hdc, oldTextCol);

                // 4. Fokus-Rechteck unterdrücken (Menüs haben keinen gestrichelten Rahmen)
                return TRUE;
            }
            break;
        }

        case WM_SETFOCUS:
            if (pData) SetFocus(pData->hwndEdit);
            return 0;

        case WM_KILLFOCUS: {
            HWND hwndNewFocus = (HWND)wParam;
            if (pData && hwndNewFocus != pData->hwndFrame &&
                hwndNewFocus != pData->hwndEdit && hwndNewFocus != pData->hwndList) 
            {
                DestroyPopupLayout(pData);
            }
            return 0;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Subclass für das Edit-Control
LRESULT CALLBACK DynamicEditSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    DynamicPopupData* pData = (DynamicPopupData*)dwRefData;

    switch (uMsg) {
        case WM_COMMAND:
            if (HIWORD(wParam) == EN_CHANGE) {
                UpdateListContent(pData);
            }
            break;

        case WM_KILLFOCUS: {
            HWND hwndNewFocus = (HWND)wParam;
            if (hwndNewFocus != pData->hwndFrame && hwndNewFocus != pData->hwndList && hwndNewFocus != hWnd) {
                DestroyPopupLayout(pData);
            }
            break;
        }

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyPopupLayout(pData);
                return 0;
            }
            if (wParam == VK_DOWN || wParam == VK_UP) {
                HideTooltip(pData);
                SendMessage(pData->hwndList, WM_KEYDOWN, wParam, lParam);
                int sel = (int)SendMessage(pData->hwndList, LB_GETCURSEL, 0, 0);
                if (pData->tooltipCallback == NULL || sel < 0)
                    return 0;

                // Show tooltip for the new selected item
                RECT itemrect;
                LRESULT ret = SendMessage(pData->hwndList, LB_GETITEMRECT, (WPARAM)sel, (LPARAM)&itemrect);
                if (ret >= 0) {
                    POINT pt;
                    pt.x = (itemrect.left + itemrect.right) / 2;
                    pt.y = (itemrect.top + itemrect.bottom) / 2;
                    if (ClientToScreen(pData->hwndList,&pt))
                         ShowTooltipForItem(pData, sel, pt);
                }

                return 0;
            }
            if (wParam == VK_RETURN) {
                int index = (int)SendMessage(pData->hwndList, LB_GETCURSEL, 0, 0);
                if (index != LB_ERR) {
                    PopupItemData* pItem = (PopupItemData*)SendMessage(pData->hwndList, LB_GETITEMDATA, index, 0);
                    if (pItem && pData->selectCallback) {
                        pData->selectCallback(pItem, pData->pUserData);
                    }
                }
                DestroyPopupLayout(pData);
                return 0;
            }
            break;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Subclass für die Listbox
LRESULT CALLBACK DynamicListSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    DynamicPopupData* pData = (DynamicPopupData*)dwRefData;

    switch (uMsg) {
    case WM_MOUSEMOVE: {
        // Apparently tis message comes in even when mouse has not been moved.
        // Therefore we comapare with the previous position, to avoid false alarms. 
        static POINT oldpos = { 0, 0 };
        POINT pt;
        GetCursorPos(&pt);
        if (pt.x == oldpos.x && pt.y == oldpos.y)
            break;
        // memorize the position
        oldpos = pt;
        int sel = (int)SendMessage(hWnd, LB_GETCURSEL, 0, 0);
        int index = (int)SendMessage(hWnd, LB_ITEMFROMPOINT, 0, lParam);
        // If mouse has really been moved and a different item is under the mouse cursor, select that item.
        if (index != LB_ERR && index != sel) {
            SendMessage(hWnd, LB_SETCURSEL, index, 0);
        }
        // Show tooltip for the new selected item
        if (pData && pData->tooltipCallback && index != LB_ERR) {
            ShowTooltipForItem(pData, index, pt);
        }
        break;
    }

    case WM_MOUSELEAVE: {
        HideTooltip(pData);
        break;
    }

    //case WM_KILLFOCUS: {
    //    HWND hwndNewFocus = (HWND)wParam;
    //    if (hwndNewFocus != pData->hwndFrame && hwndNewFocus != pData->hwndEdit && hwndNewFocus != hWnd) {
    //        DestroyPopupLayout(pData);
    //    }
    //    break;
    //}

    case WM_LBUTTONUP: {
        LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        int index = (int)SendMessage(hWnd, LB_GETCURSEL, 0, 0);
        if (index != LB_ERR) {
            PopupItemData* pItem = (PopupItemData*)SendMessage(hWnd, LB_GETITEMDATA, index, 0);
            if (pItem && pData->selectCallback) {
                pData->selectCallback(pItem, pData->pUserData);
            }
        }
        DestroyPopupLayout(pData);
        return res;
    }

    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

typedef struct {
    UINT_PTR selectedItemData;
    BOOL selectionMade;
} ModalPopupState;

static void TrackDynamicPopup_InternalSelectCallback(const PopupItemData* pSelectedItem, void* pUserData)
{
    ModalPopupState* pState = (ModalPopupState*)pUserData;
    if (pState && pSelectedItem) {
        pState->selectedItemData = pSelectedItem->itemData;
        pState->selectionMade = TRUE;
    }
}

UINT_PTR TrackDynamicPopup(HWND hwndFrame)
{
    if (!IsWindow(hwndFrame)) {
        return 0;
    }

    DynamicPopupData* pData = (DynamicPopupData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
    if (!pData) {
        return 0;
    }

    ModalPopupState modalState = { 0 };
    modalState.selectedItemData = 0;
    modalState.selectionMade = FALSE;

    OnPopupSelectCallback pOriginalSelectCallback = pData->selectCallback;
    pData->selectCallback = TrackDynamicPopup_InternalSelectCallback;
    
    void* pOriginalUserData = pData->pUserData;
    pData->pUserData = (void*)&modalState;

    ActivateDynamicPopup(hwndFrame);

    MSG msg = { 0 };
    BOOL bContinue = TRUE;

    while (bContinue && GetMessage(&msg, NULL, 0, 0)) {
        if (!IsWindow(hwndFrame)) {
            bContinue = FALSE;
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (modalState.selectionMade) {
            bContinue = FALSE;
            break;
        }
    }

    if (IsWindow(hwndFrame)) {
        DestroyWindow(hwndFrame);
    }

    UINT_PTR result = modalState.selectedItemData;
        
    return result;
}
