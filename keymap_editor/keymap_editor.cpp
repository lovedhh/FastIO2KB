#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define KEYMAP_INI_FILE "keymap.ini"
#define ID_LISTVIEW 101
#define ID_BTN_EDIT 102
#define ID_BTN_SAVE 103
#define ID_BTN_LOAD 104
#define ID_BTN_PRESET 105
#define ID_BTN_RESET 106
#define ID_STATUS 107
#define ID_DLG_CAPTURE 201
#define IDC_STATIC_PROMPT 301
#define APP_NAME L"FastIO2KB Keymap Editor"

struct KeyBindingData {
    const char* section;
    const char* display;
    WORD scanCode;
    DWORD flags;
    BOOL defined;
    WORD defaultScan;
    DWORD defaultFlags;
    BOOL defaultDefined;
};

struct KeyNameEntry {
    WORD scanCode;
    DWORD flags;
    const char* name;
};

static const KeyNameEntry g_keyNames[] = {
    {0x01, 0, "Esc"}, {0x02, 0, "1"}, {0x03, 0, "2"}, {0x04, 0, "3"},
    {0x05, 0, "4"}, {0x06, 0, "5"}, {0x07, 0, "6"}, {0x08, 0, "7"},
    {0x09, 0, "8"}, {0x0A, 0, "9"}, {0x0B, 0, "0"}, {0x0C, 0, "-"},
    {0x0D, 0, "="}, {0x0E, 0, "Backspace"}, {0x0F, 0, "Tab"},
    {0x10, 0, "Q"}, {0x11, 0, "W"}, {0x12, 0, "E"}, {0x13, 0, "R"},
    {0x14, 0, "T"}, {0x15, 0, "Y"}, {0x16, 0, "U"}, {0x17, 0, "I"},
    {0x18, 0, "O"}, {0x19, 0, "P"}, {0x1A, 0, "["}, {0x1B, 0, "]"},
    {0x1C, 0, "Enter"}, {0x1D, 0, "Left Ctrl"}, {0x1E, 0, "A"},
    {0x1F, 0, "S"}, {0x20, 0, "D"}, {0x21, 0, "F"}, {0x22, 0, "G"},
    {0x23, 0, "H"}, {0x24, 0, "J"}, {0x25, 0, "K"}, {0x26, 0, "L"},
    {0x27, 0, ";"}, {0x28, 0, "'"}, {0x29, 0, "`"},
    {0x2B, 0, "\\"}, {0x2C, 0, "Z"}, {0x2D, 0, "X"}, {0x2E, 0, "C"},
    {0x2F, 0, "V"}, {0x30, 0, "B"}, {0x31, 0, "N"}, {0x32, 0, "M"},
    {0x33, 0, ","}, {0x34, 0, "."}, {0x35, 0, "/"},
    {0x36, 0, "Right Shift"}, {0x37, 0, "Numpad *"},
    {0x38, 0, "Left Alt"}, {0x39, 0, "Space"}, {0x3A, 0, "Caps Lock"},
    {0x3B, 0, "F1"}, {0x3C, 0, "F2"}, {0x3D, 0, "F3"}, {0x3E, 0, "F4"},
    {0x3F, 0, "F5"}, {0x40, 0, "F6"}, {0x41, 0, "F7"}, {0x42, 0, "F8"},
    {0x43, 0, "F9"}, {0x44, 0, "F10"}, {0x45, 0, "F11"}, {0x46, 0, "F12"},
    {0x47, 0, "Numpad 7"}, {0x48, 0, "Numpad 8"}, {0x49, 0, "Numpad 9"},
    {0x4A, 0, "Numpad -"}, {0x4B, 0, "Numpad 4"}, {0x4C, 0, "Numpad 5"},
    {0x4D, 0, "Numpad 6"}, {0x4E, 0, "Numpad +"}, {0x4F, 0, "Numpad 1"},
    {0x50, 0, "Numpad 2"}, {0x51, 0, "Numpad 3"}, {0x52, 0, "Numpad 0"},
    {0x53, 0, "Numpad ."},
    {0x56, 0, "\\ (102)"},
    {0x57, 0, "F13"}, {0x58, 0, "F14"}, {0x59, 0, "F15"},
    {0x5A, 0, "F16"}, {0x5B, 0, "F17"}, {0x5C, 0, "F18"},
    {0x5D, 0, "F19"}, {0x5E, 0, "F20"}, {0x5F, 0, "F21"},
    {0x60, 0, "F22"}, {0x61, 0, "F23"}, {0x62, 0, "F24"},
    {0x63, 0, "F24"},
    {0x64, 0, "F24"},
    {0xE0, 0, "Left Ctrl (2nd)"},
    {0xE1, 0, "Left Alt (2nd)"},
    {0xE2, 0, "Right Alt (2nd)"},
    {0x47, KEYEVENTF_EXTENDEDKEY, "Home"},
    {0x48, KEYEVENTF_EXTENDEDKEY, "Up"},
    {0x49, KEYEVENTF_EXTENDEDKEY, "Page Up"},
    {0x4B, KEYEVENTF_EXTENDEDKEY, "Left"},
    {0x4D, KEYEVENTF_EXTENDEDKEY, "Right"},
    {0x4F, KEYEVENTF_EXTENDEDKEY, "End"},
    {0x50, KEYEVENTF_EXTENDEDKEY, "Down"},
    {0x51, KEYEVENTF_EXTENDEDKEY, "Page Down"},
    {0x52, KEYEVENTF_EXTENDEDKEY, "Insert"},
    {0x53, KEYEVENTF_EXTENDEDKEY, "Delete"},
    {0x9C, KEYEVENTF_EXTENDEDKEY, "Numpad Enter"},
    {0xB5, KEYEVENTF_EXTENDEDKEY, "Numpad /"},
    {0xB7, KEYEVENTF_EXTENDEDKEY, "Print Screen"},
    {0xB8, KEYEVENTF_EXTENDEDKEY, "Right Alt"},
    {0xBB, KEYEVENTF_EXTENDEDKEY, "Menu"},
    {0xBD, KEYEVENTF_EXTENDEDKEY, "Right Ctrl"},
    {0xDB, KEYEVENTF_EXTENDEDKEY, "Left Win"},
    {0xDC, KEYEVENTF_EXTENDEDKEY, "Right Win"},
    {0, 0, NULL}
};

struct BindingDef {
    const char* section;
    const char* display;
    WORD defScan;
    DWORD defFlags;
    BOOL defDefined;
};

static KeyBindingData g_bindings[68];
static int g_bindingCount = 0;
static HWND g_hListView = NULL;
static HWND g_hStatus = NULL;
static WCHAR g_statusText[256] = L"Ready";
static BOOL g_modified = FALSE;

static const BindingDef g_defaults[] = {
    {"P1_START", "Player 1 Start", 0x02, 0x0008, TRUE},
    {"P1_UP", "Player 1 Up", 0x48, 0x0009, TRUE},
    {"P1_DOWN", "Player 1 Down", 0x50, 0x0009, TRUE},
    {"P1_LEFT", "Player 1 Left", 0x4B, 0x0009, TRUE},
    {"P1_RIGHT", "Player 1 Right", 0x4D, 0x0009, TRUE},
    {"P1_BTN1", "P1 Button 1", 0x1D, 0x0008, TRUE},
    {"P1_BTN2", "P1 Button 2", 0x38, 0x0008, TRUE},
    {"P1_BTN3", "P1 Button 3", 0x39, 0x0008, TRUE},
    {"P1_BTN4", "P1 Button 4", 0x2A, 0x0008, TRUE},
    {"P1_BTN5", "P1 Button 5", 0x2C, 0x0008, TRUE},
    {"P1_BTN6", "P1 Button 6", 0x2D, 0x0008, TRUE},
    {"P1_BTN7", "P1 Button 7", 0x2E, 0x0008, TRUE},
    {"P1_BTN8", "P1 Button 8", 0x2F, 0x0008, TRUE},
    {"P2_START", "Player 2 Start", 0x03, 0x0008, TRUE},
    {"P2_UP", "Player 2 Up", 0x13, 0x0008, TRUE},
    {"P2_DOWN", "Player 2 Down", 0x21, 0x0008, TRUE},
    {"P2_LEFT", "Player 2 Left", 0x20, 0x0008, TRUE},
    {"P2_RIGHT", "Player 2 Right", 0x22, 0x0008, TRUE},
    {"P2_BTN1", "P2 Button 1", 0x1E, 0x0008, TRUE},
    {"P2_BTN2", "P2 Button 2", 0x1F, 0x0008, TRUE},
    {"P2_BTN3", "P2 Button 3", 0x10, 0x0008, TRUE},
    {"P2_BTN4", "P2 Button 4", 0x11, 0x0008, TRUE},
    {"P2_BTN5", "P2 Button 5", 0x12, 0x0008, TRUE},
    {"P2_BTN6", "P2 Button 6", 0x15, 0x0008, TRUE},
    {"P2_BTN7", "P2 Button 7", 0x23, 0x0008, TRUE},
    {"P2_BTN8", "P2 Button 8", 0x16, 0x0008, TRUE},
    {"P3_START", "Player 3 Start", 0x04, 0x0008, TRUE},
    {"P3_UP", "Player 3 Up", 0x17, 0x0008, TRUE},
    {"P3_DOWN", "Player 3 Down", 0x25, 0x0008, TRUE},
    {"P3_LEFT", "Player 3 Left", 0x24, 0x0008, TRUE},
    {"P3_RIGHT", "Player 3 Right", 0x26, 0x0008, TRUE},
    {"P3_BTN1", "P3 Button 1", 0x9D, 0x0009, TRUE},
    {"P3_BTN2", "P3 Button 2", 0x36, 0x0008, TRUE},
    {"P3_BTN3", "P3 Button 3", 0x1C, 0x0008, TRUE},
    {"P3_BTN4", "P3 Button 4", 0xB8, 0x0009, TRUE},
    {"P4_START", "Player 4 Start", 0x05, 0x0008, TRUE},
    {"P4_UP", "Player 4 Up", 0x48, 0x0008, TRUE},
    {"P4_DOWN", "Player 4 Down", 0x50, 0x0008, TRUE},
    {"P4_LEFT", "Player 4 Left", 0x4B, 0x0008, TRUE},
    {"P4_RIGHT", "Player 4 Right", 0x4D, 0x0008, TRUE},
    {"P4_BTN1", "P4 Button 1", 0x52, 0x0008, TRUE},
    {"P4_BTN2", "P4 Button 2", 0x53, 0x0008, TRUE},
    {"P4_BTN3", "P4 Button 3", 0x9C, 0x0009, TRUE},
    {"P4_BTN4", "P4 Button 4", 0x4E, 0x0008, TRUE},
    {"S_COIN1", "Coin 1", 0x06, 0x0008, TRUE},
    {"S_COIN2", "Coin 2", 0x07, 0x0008, TRUE},
    {"S_COIN3", "Coin 3", 0x08, 0x0008, TRUE},
    {"S_COIN4", "Coin 4", 0x09, 0x0008, TRUE},
    {"S_TEST1", "Test 1", 0x3C, 0x0008, TRUE},
    {"S_TEST2", "Test 2", 0x3C, 0x0008, TRUE},
    {"S_TILT1", "Tilt 1", 0x14, 0x0008, TRUE},
    {"S_TILT2", "Tilt 2", 0x14, 0x0008, TRUE},
    {"S_SERVICE1", "Service 1", 0x0A, 0x0008, TRUE},
    {"S_SERVICE2", "Service 2", 0x0B, 0x0008, TRUE},
    {"S_SERVICE3", "Service 3", 0x0C, 0x0008, TRUE},
    {"S_SERVICE4", "Service 4", 0x0D, 0x0008, TRUE},
    {"S_ESC1", "Escape 1 (P1 Start+Btn1+Btn3)", 0x01, 0x0008, TRUE},
    {"S_ESC2", "Escape 2 (P2 Start+Btn1+Btn3)", 0x01, 0x0008, FALSE},
    {"S_ESC3", "Escape 3 (P3 Start+Btn1+Btn3)", 0x01, 0x0008, FALSE},
    {"S_ESC4", "Escape 4 (P4 Start+Btn1+Btn3)", 0x01, 0x0008, FALSE},
};

static const char* GetKeyName(WORD scanCode, DWORD flags) {
    for (int i = 0; g_keyNames[i].name; i++) {
        if (g_keyNames[i].scanCode == scanCode && g_keyNames[i].flags == (flags & KEYEVENTF_EXTENDEDKEY))
            return g_keyNames[i].name;
    }
    static char hexName[16];
    sprintf_s(hexName, sizeof(hexName), "0x%02X%s", scanCode,
        (flags & KEYEVENTF_EXTENDEDKEY) ? " (ext)" : "");
    return hexName;
}

static void InitBindings(void) {
    g_bindingCount = sizeof(g_defaults) / sizeof(g_defaults[0]);
    for (int i = 0; i < g_bindingCount; i++) {
        g_bindings[i].section = g_defaults[i].section;
        g_bindings[i].display = g_defaults[i].display;
        g_bindings[i].scanCode = g_defaults[i].defScan;
        g_bindings[i].flags = g_defaults[i].defFlags;
        g_bindings[i].defined = g_defaults[i].defDefined;
        g_bindings[i].defaultScan = g_defaults[i].defScan;
        g_bindings[i].defaultFlags = g_defaults[i].defFlags;
        g_bindings[i].defaultDefined = g_defaults[i].defDefined;
    }
    g_modified = FALSE;
}

static void LoadFromINI(const char* filename) {
    InitBindings();
    if (GetFileAttributesA(filename) == INVALID_FILE_ATTRIBUTES) return;

    for (int i = 0; i < g_bindingCount; i++) {
        char buf[32];
        const char* sec = g_bindings[i].section;

        DWORD ret = GetPrivateProfileStringA(sec, "ScanCode", "", buf, sizeof(buf), filename);
        if (ret > 0) g_bindings[i].scanCode = (WORD)strtoul(buf, NULL, 0);

        ret = GetPrivateProfileStringA(sec, "Flags", "", buf, sizeof(buf), filename);
        if (ret > 0) g_bindings[i].flags = (DWORD)strtoul(buf, NULL, 0);

        ret = GetPrivateProfileStringA(sec, "Defined", "", buf, sizeof(buf), filename);
        if (ret > 0) g_bindings[i].defined = (buf[0] == '1' || buf[0] == 't' || buf[0] == 'T');
    }
    g_modified = FALSE;
}

static void SaveToINI(const char* filename) {
    for (int i = 0; i < g_bindingCount; i++) {
        char buf[32];
        const char* sec = g_bindings[i].section;

        sprintf_s(buf, sizeof(buf), "0x%04X", g_bindings[i].scanCode);
        WritePrivateProfileStringA(sec, "ScanCode", buf, filename);

        sprintf_s(buf, sizeof(buf), "0x%08X", g_bindings[i].flags);
        WritePrivateProfileStringA(sec, "Flags", buf, filename);

        WritePrivateProfileStringA(sec, "Defined", g_bindings[i].defined ? "1" : "0", filename);
    }
    g_modified = FALSE;
}

static void LoadPreset(const char* name) {
    InitBindings();
    SetWindowTextW(g_hStatus, L"Ready");
    swprintf_s(g_statusText, L"Preset: %s", L"Custom");
}

static void UpdateListView(void) {
    ListView_DeleteAllItems(g_hListView);
    LVITEMW lvi = {0};
    WCHAR buf[256];
    for (int i = 0; i < g_bindingCount; i++) {
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;

        lvi.iSubItem = 0;
        MultiByteToWideChar(CP_UTF8, 0, g_bindings[i].section, -1, buf, 256);
        lvi.pszText = buf;
        ListView_InsertItem(g_hListView, &lvi);

        lvi.iSubItem = 1;
        MultiByteToWideChar(CP_UTF8, 0, g_bindings[i].display, -1, buf, 256);
        lvi.pszText = buf;
        ListView_SetItem(g_hListView, &lvi);

        lvi.iSubItem = 2;
        if (g_bindings[i].defined) {
            const char* keyName = GetKeyName(g_bindings[i].scanCode, g_bindings[i].flags);
            MultiByteToWideChar(CP_UTF8, 0, keyName, -1, buf, 256);
        } else {
            wcscpy_s(buf, L"<Disabled>");
        }
        lvi.pszText = buf;
        ListView_SetItem(g_hListView, &lvi);
    }
}

static void SetStatus(const WCHAR* text) {
    wcscpy_s(g_statusText, text);
    SetWindowTextW(g_hStatus, g_statusText);
}

static void StartKeyCapture(HWND hParent, int index) {
    if (index < 0 || index >= g_bindingCount) return;

    HWND hDlg = CreateDialogParamW(GetModuleHandle(NULL),
        MAKEINTRESOURCEW(-1), hParent, NULL, 0);
    if (!hDlg) {
        // Create a simple dialog manually
        hDlg = CreateWindowW(L"#32770", L"Capture Key",
            WS_CAPTION | WS_SYSMENU | WS_VISIBLE | DS_CENTER,
            CW_USEDEFAULT, CW_USEDEFAULT, 350, 120,
            hParent, NULL, GetModuleHandle(NULL), NULL);
    }

    // Not a proper dialog, so create a simple popup window
    if (hDlg) DestroyWindow(hDlg);

    HWND hCapture = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        L"STATIC", L"",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 360, 120,
        hParent, NULL, GetModuleHandle(NULL), NULL);

    if (!hCapture) return;

    WCHAR title[256];
    MultiByteToWideChar(CP_UTF8, 0, g_bindings[index].display, -1, title, 256);
    SetWindowTextW(hCapture, title);

    HWND hText = CreateWindowW(L"STATIC",
        L"Press a key to assign...\n(Press Esc to cancel)",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        10, 20, 340, 50, hCapture, NULL, GetModuleHandle(NULL), NULL);

    SetCapture(hCapture);
    SetWindowLongPtrW(hCapture, GWLP_USERDATA, (LONG_PTR)index);

    // Store in global for the hook
    HWND oldCapture = hCapture;
    MSG msg;
    BOOL done = FALSE;
    while (!done && GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            WORD scan = (WORD)((msg.lParam >> 16) & 0xFF);
            BOOL extended = (msg.lParam >> 24) & 1;
            DWORD flags = KEYEVENTF_SCANCODE;
            if (extended) flags |= KEYEVENTF_EXTENDEDKEY;

            if (scan == 0x01) { // Esc -> cancel, set to disabled
                g_bindings[index].defined = FALSE;
                wcscpy_s(g_statusText, L"Cancelled");
            } else {
                g_bindings[index].scanCode = scan;
                g_bindings[index].flags = flags;
                g_bindings[index].defined = TRUE;
                const char* kname = GetKeyName(scan, flags);
                WCHAR wname[64];
                MultiByteToWideChar(CP_UTF8, 0, kname, -1, wname, 64);
                swprintf_s(g_statusText, L"Set %S -> %s",
                    g_bindings[index].display, wname);
            }
            g_modified = TRUE;
            done = TRUE;
        }
        if (msg.message == WM_ACTIVATE && LOWORD(msg.wParam) == WA_INACTIVE) {
            // Clicked outside, cancel
            done = TRUE;
        }
        if (msg.message == WM_NCLBUTTONDOWN || msg.message == WM_NCLBUTTONUP) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
    }

    ReleaseCapture();
    UpdateListView();
    SetWindowTextW(g_hStatus, g_statusText);
    DestroyWindow(hCapture);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        InitCommonControls();

        RECT rc;
        GetClientRect(hWnd, &rc);

        g_hListView = CreateWindowW(WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
            10, 10, rc.right - 20, rc.bottom - 100,
            hWnd, (HMENU)ID_LISTVIEW, GetModuleHandle(NULL), NULL);

        LVCOLUMNW lvc = {0};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;

        lvc.cx = 120;
        lvc.pszText = L"Section";
        ListView_InsertColumn(g_hListView, 0, &lvc);

        lvc.cx = 240;
        lvc.pszText = L"Input";
        ListView_InsertColumn(g_hListView, 1, &lvc);

        lvc.cx = 160;
        lvc.pszText = L"Bound Key";
        ListView_InsertColumn(g_hListView, 2, &lvc);

        ListView_SetExtendedListViewStyle(g_hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        CreateWindowW(L"BUTTON", L"Edit Key",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, rc.bottom - 75, 90, 30,
            hWnd, (HMENU)ID_BTN_EDIT, GetModuleHandle(NULL), NULL);

        CreateWindowW(L"BUTTON", L"Save",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            110, rc.bottom - 75, 80, 30,
            hWnd, (HMENU)ID_BTN_SAVE, GetModuleHandle(NULL), NULL);

        CreateWindowW(L"BUTTON", L"Load",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            200, rc.bottom - 75, 80, 30,
            hWnd, (HMENU)ID_BTN_LOAD, GetModuleHandle(NULL), NULL);

        CreateWindowW(L"BUTTON", L"Reset Defaults",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            290, rc.bottom - 75, 110, 30,
            hWnd, (HMENU)ID_BTN_RESET, GetModuleHandle(NULL), NULL);

        CreateWindowW(L"BUTTON", L"Presets...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            410, rc.bottom - 75, 90, 30,
            hWnd, (HMENU)ID_BTN_PRESET, GetModuleHandle(NULL), NULL);

        g_hStatus = CreateWindowW(L"STATIC", L"Ready",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, rc.bottom - 30, rc.right - 20, 20,
            hWnd, (HMENU)ID_STATUS, GetModuleHandle(NULL), NULL);

        InitBindings();
        LoadFromINI(KEYMAP_INI_FILE);
        UpdateListView();
        break;
    }

    case WM_SIZE: {
        RECT rc;
        GetClientRect(hWnd, &rc);
        if (g_hListView) {
            SetWindowPos(g_hListView, NULL, 10, 10, rc.right - 20, rc.bottom - 100, SWP_NOZORDER);
        }
        if (g_hStatus) {
            SetWindowPos(g_hStatus, NULL, 10, rc.bottom - 30, rc.right - 20, 20, SWP_NOZORDER);
        }
        // Reposition buttons
        HWND hBtnEdit = GetDlgItem(hWnd, ID_BTN_EDIT);
        HWND hBtnSave = GetDlgItem(hWnd, ID_BTN_SAVE);
        HWND hBtnLoad = GetDlgItem(hWnd, ID_BTN_LOAD);
        HWND hBtnReset = GetDlgItem(hWnd, ID_BTN_RESET);
        HWND hBtnPreset = GetDlgItem(hWnd, ID_BTN_PRESET);
        if (hBtnEdit) SetWindowPos(hBtnEdit, NULL, 10, rc.bottom - 75, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if (hBtnSave) SetWindowPos(hBtnSave, NULL, 110, rc.bottom - 75, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if (hBtnLoad) SetWindowPos(hBtnLoad, NULL, 200, rc.bottom - 75, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if (hBtnReset) SetWindowPos(hBtnReset, NULL, 290, rc.bottom - 75, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if (hBtnPreset) SetWindowPos(hBtnPreset, NULL, 410, rc.bottom - 75, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        break;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        switch (id) {
        case ID_BTN_EDIT: {
            int sel = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
            if (sel < 0) {
                SetStatus(L"Select a binding first");
                break;
            }
            StartKeyCapture(hWnd, sel);
            break;
        }
        case ID_BTN_SAVE:
            SaveToINI(KEYMAP_INI_FILE);
            SetStatus(L"Saved to keymap.ini");
            break;
        case ID_BTN_LOAD:
            LoadFromINI(KEYMAP_INI_FILE);
            UpdateListView();
            SetStatus(L"Loaded from keymap.ini");
            break;
        case ID_BTN_RESET: {
            if (g_modified) {
                int ret = MessageBoxW(hWnd,
                    L"Reset all bindings to defaults? Unsaved changes will be lost.",
                    APP_NAME, MB_YESNO | MB_ICONWARNING);
                if (ret != IDYES) break;
            }
            InitBindings();
            UpdateListView();
            SetStatus(L"Reset to defaults");
            break;
        }
        case ID_BTN_PRESET: {
            HMENU hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, MF_STRING, 1001, L"MAME (Player 1-4)");
            AppendMenuW(hMenu, MF_STRING, 1002, L"PPSSPP (Player 1-2)");
            AppendMenuW(hMenu, MF_STRING, 1003, L"eX-BOARD (Player 1-2)");
            AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, 1004, L"Custom (from current INI)");

            RECT btnRect;
            GetWindowRect(GetDlgItem(hWnd, ID_BTN_PRESET), &btnRect);
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN,
                btnRect.left, btnRect.bottom, 0, hWnd, NULL);
            DestroyMenu(hMenu);

            if (cmd == 1001 || cmd == 1002 || cmd == 1003) {
                if (g_modified) {
                    int ret = MessageBoxW(hWnd,
                        L"Load preset? Unsaved changes will be lost.",
                        APP_NAME, MB_YESNO | MB_ICONWARNING);
                    if (ret != IDYES) break;
                }
                InitBindings();
                // Apply preset-specific overrides
                if (cmd == 1001) { // MAME
                    // Defaults already match MAME
                    SetStatus(L"Loaded MAME preset");
                } else if (cmd == 1002) { // PPSSPP
                    g_bindings[0].scanCode = 0x39; g_bindings[0].flags = 0x0008; // P1_START -> Space
                    g_bindings[1].scanCode = 0x48; g_bindings[1].flags = 0x0009; // P1_UP -> Up (ext)
                    g_bindings[2].scanCode = 0x50; g_bindings[2].flags = 0x0009; // P1_DOWN -> Down (ext)
                    g_bindings[3].scanCode = 0x4B; g_bindings[3].flags = 0x0009; // P1_LEFT -> Left (ext)
                    g_bindings[4].scanCode = 0x4D; g_bindings[4].flags = 0x0009; // P1_RIGHT -> Right (ext)
                    g_bindings[5].scanCode = 0x10; g_bindings[5].flags = 0x0008; // P1_BTN1 -> Q
                    g_bindings[6].scanCode = 0x11; g_bindings[6].flags = 0x0008; // P1_BTN2 -> W
                    g_bindings[7].scanCode = 0x1F; g_bindings[7].flags = 0x0008; // P1_BTN3 -> S
                    g_bindings[8].scanCode = 0x2D; g_bindings[8].flags = 0x0008; // P1_BTN4 -> X
                    g_bindings[9].scanCode = 0x1E; g_bindings[9].flags = 0x0008; // P1_BTN5 -> A
                    g_bindings[10].scanCode = 0x2C; g_bindings[10].flags = 0x0008; // P1_BTN6 -> Z
                    g_bindings[11].scanCode = 0x20; g_bindings[11].flags = 0x0008; // P1_BTN7 -> D
                    g_bindings[12].scanCode = 0x2E; g_bindings[12].flags = 0x0008; // P1_BTN8 -> C
                    g_bindings[13].scanCode = 0x2F; g_bindings[13].flags = 0x0008; // P2_START -> V
                    g_bindings[14].scanCode = 0x17; g_bindings[14].flags = 0x0008; // P2_UP -> I
                    g_bindings[15].scanCode = 0x25; g_bindings[15].flags = 0x0008; // P2_DOWN -> K
                    g_bindings[16].scanCode = 0x24; g_bindings[16].flags = 0x0008; // P2_LEFT -> J
                    g_bindings[17].scanCode = 0x26; g_bindings[17].flags = 0x0008; // P2_RIGHT -> L
                    g_bindings[18].scanCode = 0x01; g_bindings[18].flags = 0x0008; // P2_BTN1 -> Esc
                    g_bindings[19].scanCode = 0x02; g_bindings[19].flags = 0x0008; // P2_BTN2 -> 1
                    g_bindings[20].scanCode = 0x03; g_bindings[20].flags = 0x0008; // P2_BTN3 -> 2
                    g_bindings[21].scanCode = 0x04; g_bindings[21].flags = 0x0008; // P2_BTN4 -> 3
                    g_bindings[22].scanCode = 0x05; g_bindings[22].flags = 0x0008; // P2_BTN5 -> 4
                    g_bindings[23].scanCode = 0x06; g_bindings[23].flags = 0x0008; // P2_BTN6 -> 5
                    g_bindings[24].scanCode = 0x1A; g_bindings[24].flags = 0x0008; // P2_BTN7 -> [
                    g_bindings[25].scanCode = 0x1B; g_bindings[25].flags = 0x0008; // P2_BTN8 -> ]
                    g_bindings[44].scanCode = 0x29; g_bindings[44].flags = 0x0008; // S_COIN1 -> `
                    g_bindings[45].scanCode = 0x07; g_bindings[45].flags = 0x0008; // S_COIN2 -> 6
                    g_bindings[48].scanCode = 0x08; g_bindings[48].flags = 0x0008; // S_TEST1 -> 7
                    g_bindings[50].scanCode = 0x2B; g_bindings[50].flags = 0x0008; // S_TILT1 -> \
                    g_bindings[52].scanCode = 0x09; g_bindings[52].flags = 0x0008; // S_SERVICE1 -> 8
                    g_bindings[53].scanCode = 0x0A; g_bindings[53].flags = 0x0008; // S_SERVICE2 -> 9
                    g_bindings[56].scanCode = 0x0B; g_bindings[56].flags = 0x0008; // S_ESC1 -> 0
                    // Disable P3/P4 for PPSSPP
                    for (int i = 26; i < 44; i++) g_bindings[i].defined = FALSE;
                    SetStatus(L"Loaded PPSSPP preset");
                } else if (cmd == 1003) { // eX-BOARD
                    g_bindings[0].scanCode = 0x11; g_bindings[0].flags = 0x0008; // P1_START -> W
                    g_bindings[1].scanCode = 0x48; g_bindings[1].flags = 0x0009; // P1_UP -> Up (ext)
                    g_bindings[2].scanCode = 0x50; g_bindings[2].flags = 0x0009; // P1_DOWN -> Down (ext)
                    g_bindings[3].scanCode = 0x4B; g_bindings[3].flags = 0x0009; // P1_LEFT -> Left (ext)
                    g_bindings[4].scanCode = 0x4D; g_bindings[4].flags = 0x0009; // P1_RIGHT -> Right (ext)
                    g_bindings[5].scanCode = 0x2C; g_bindings[5].flags = 0x0008; // P1_BTN1 -> Z
                    g_bindings[6].scanCode = 0x2D; g_bindings[6].flags = 0x0008; // P1_BTN2 -> X
                    g_bindings[7].scanCode = 0x2E; g_bindings[7].flags = 0x0008; // P1_BTN3 -> C
                    g_bindings[8].scanCode = 0x2F; g_bindings[8].flags = 0x0008; // P1_BTN4 -> V
                    g_bindings[9].scanCode = 0x31; g_bindings[9].flags = 0x0008; // P1_BTN5 -> N
                    g_bindings[13].scanCode = 0x12; g_bindings[13].flags = 0x0008; // P2_START -> E
                    g_bindings[14].scanCode = 0x48; g_bindings[14].flags = 0x0008; // P2_UP -> Numpad8
                    g_bindings[15].scanCode = 0x50; g_bindings[15].flags = 0x0008; // P2_DOWN -> Numpad2
                    g_bindings[16].scanCode = 0x4B; g_bindings[16].flags = 0x0008; // P2_LEFT -> Numpad4
                    g_bindings[17].scanCode = 0x4D; g_bindings[17].flags = 0x0008; // P2_RIGHT -> Numpad6
                    g_bindings[18].scanCode = 0x1E; g_bindings[18].flags = 0x0008; // P2_BTN1 -> A
                    g_bindings[19].scanCode = 0x1F; g_bindings[19].flags = 0x0008; // P2_BTN2 -> S
                    g_bindings[20].scanCode = 0x20; g_bindings[20].flags = 0x0008; // P2_BTN3 -> D
                    g_bindings[21].scanCode = 0x21; g_bindings[21].flags = 0x0008; // P2_BTN4 -> F
                    g_bindings[22].scanCode = 0x14; g_bindings[22].flags = 0x0008; // P2_BTN5 -> T
                    g_bindings[44].scanCode = 0x25; g_bindings[44].flags = 0x0008; // S_COIN1 -> K
                    g_bindings[45].scanCode = 0x24; g_bindings[45].flags = 0x0008; // S_COIN2 -> J
                    g_bindings[48].scanCode = 0x23; g_bindings[48].flags = 0x0008; // S_TEST1 -> H
                    // Disable P3/P4 for eX-BOARD
                    for (int i = 26; i < 44; i++) g_bindings[i].defined = FALSE;
                    SetStatus(L"Loaded eX-BOARD preset");
                }
                g_modified = TRUE;
                UpdateListView();
            } else if (cmd == 1004) {
                LoadFromINI(KEYMAP_INI_FILE);
                UpdateListView();
                SetStatus(L"Loaded from keymap.ini");
            }
            break;
        }
        }
        break;
    }

    case WM_NOTIFY: {
        NMHDR* hdr = (NMHDR*)lParam;
        if (hdr->idFrom == ID_LISTVIEW && hdr->code == NM_DBLCLK) {
            int sel = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
            if (sel >= 0) StartKeyCapture(hWnd, sel);
        }
        break;
    }

    case WM_CLOSE:
        if (g_modified) {
            int ret = MessageBoxW(hWnd,
                L"Save changes before exiting?",
                APP_NAME, MB_YESNOCANCEL | MB_ICONQUESTION);
            if (ret == IDYES) SaveToINI(KEYMAP_INI_FILE);
            if (ret == IDCANCEL) return 0;
        }
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"KeymapEditorClass";
    RegisterClassW(&wc);

    HWND hWnd = CreateWindowExW(0, wc.lpszClassName, APP_NAME,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 550,
        NULL, NULL, hInstance, NULL);

    if (!hWnd) return 1;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
