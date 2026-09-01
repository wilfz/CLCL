/*
 * CLCL
 *
 * dynamic_popup.h
 *
 * Copyright (C) 2026 by Wilf Zimmermann. MIT License.
 *		https://linguversa.de/clcl
 *		https://github.com/wilfz/clcl
 */

#pragma once

#ifndef DYNAMIC_POPUP_H
#define DYNAMIC_POPUP_H

#include <windows.h>
#include <commctrl.h>

// Eindeutige IDs für die Controls
#define IDC_DYNAMIC_EDIT    51001
#define IDC_DYNAMIC_LISTBOX 51002

// Struktur für jedes Element in der Owner-Draw Listbox
typedef struct {
    TCHAR* pszText;       // Der anzuzeigende Text
    int iIconIndex;       // Index des Icons in einer ImageList (oder -1 für kein Icon)
    UINT_PTR itemData;    // Anwenderspezifische Daten (z.B. ID oder Pointer)
} PopupItemData;

// Callback für die Anzeige von Tooltips beim Hovern
// Rückgabe: Zeiger auf Tooltip-Text (wird vom Aufrufer NICHT freigegeben)
// oder NULL falls kein Tooltip
typedef TCHAR* (*OnPopupTooltipCallback)(const PopupItemData* pItem, void* pUserData);

// Callback für die Auswahl eines Elements
typedef void (*OnPopupSelectCallback)(const PopupItemData* pSelectedItem, void* pUserData);

// Callback zum dynamischen Befüllen der Listbox abhängig vom Edit-Text
typedef void (*OnPopupPopulateCallback)(const TCHAR* editText, HWND hwndListBox, void* pUserData);

/**
 * Erzeugt ein temporäres, kontextabhängiges Popup-Menü mit Edit-Feld und Listbox.
 *
 * @param hwndOwner         Das Hauptfenster, das dieses Popup besitzt.
 * @param x                 X-Koordinate auf dem Bildschirm (Bildschirmkoordinaten).
 * @param y                 Y-Koordinate auf dem Bildschirm.
 * @param width             Breite des Popups.
 * @return HWND             Das Handle des erstellten Edit-Popups.
 */
HWND CreateDynamicPopupMenu(HWND hwndOwner, int x, int y, int width);

void SetPopulateCallback(HWND hwndFrame, OnPopupPopulateCallback populateCallback);

void SetSelectCallback(HWND hwndFrame, OnPopupSelectCallback selectCallback);

void SetTooltipCallback(HWND hwndFrame, OnPopupTooltipCallback tooltipCallback);

void SetImageList(HWND hwndFrame, HIMAGELIST hImageList, int icon_size);

void SetIconSize(HWND hwndFrame, int icon_size);

void SetIconMargin(HWND hwndFrame, int icon_margin);

void SetTextMargin(HWND hwndFrame, int text_margin);

void SetMaxVisibleItems(HWND hwndFrame, unsigned int max_visible_items);

void SetMenuFont(HWND hwndFrame, HFONT hFont);

void SetUserData(HWND hwndFrame, void* pUserData);

void ActivateDynamicPopup(HWND hwndFrame);

/**
 * Zeigt ein Popup-Menü modal an und wartet auf eine Auswahl oder Abbruch.
 * Diese Funktion blockiert, bis der Benutzer eine Auswahl trifft oder das Popup verlässt.
 * 
 * Der Workflow:
 * 1. Rufe CreateDynamicPopupMenu() auf, um das Popup zu erstellen
 * 2. Verwende SetImageList(), SetIconSize(), SetIconMargin() etc. zum Konfigurieren
 * 3. Rufe TrackDynamicPopup() auf, um modal auf eine Auswahl zu warten
 *
 * @param hwndFrame         Das von CreateDynamicPopupMenu() zurückgegebene Fenster-Handle.
 * @return UINT_PTR         Die itemData der ausgewählten Zeile oder 0 bei Abbruch.
 *
 * Beispiel:
 *   HWND hwndPopup = CreateDynamicPopupMenu(hWnd, x, y, width);
 *   SetImageList(hwndPopup, hImageList, 16);
 *   SetIconSize(hwndPopup, 16);
 *   UINT_PTR result = TrackDynamicPopup(hwndPopup);
 */
UINT_PTR TrackDynamicPopup(HWND hwndFrame);

/**
 * Hilfsfunktion: Fügt der Owner-Draw Listbox einen Eintrag hinzu.
 * Muss innerhalb des OnPopupPopulateCallback aufgerufen werden.
 */
void PopupAddString(HWND hwndListBox, const TCHAR* pszText, int iIconIndex, UINT_PTR itemData);

#endif // DYNAMIC_POPUP_H
