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
 * @param hImageList        Optionale ImageList für die Icons.
 * @param populateCallback  Funktion, die die Listbox basierend auf dem Text befüllt.
 * @param selectCallback    Funktion, die aufgerufen wird, wenn der User ein Element wählt.
 * @param tooltipCallback   Funktion, die aufgerufen wird, um einen Tooltip beim Hovern zu erhalten (kann NULL sein).
 * @param pUserData         Beliebiger Datenzeiger (wird an alle Callbacks durchgereicht).
 * @return HWND             Das Handle des erstellten Edit-Popups.
 */
HWND CreateDynamicPopupMenu(HWND hwndOwner, int x, int y, int width,
    HIMAGELIST hImageList, // Optionale ImageList für die Icons
    OnPopupPopulateCallback populateCallback,
    OnPopupSelectCallback selectCallback,
    OnPopupTooltipCallback tooltipCallback,
    void* pUserData);

/**
 * Hilfsfunktion: Fügt der Owner-Draw Listbox einen Eintrag hinzu.
 * Muss innerhalb des OnPopupPopulateCallback aufgerufen werden.
 */
void PopupAddString(HWND hwndListBox, const TCHAR* pszText, int iIconIndex, UINT_PTR itemData);

#endif // DYNAMIC_POPUP_H
