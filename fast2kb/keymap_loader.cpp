#include "stdafx.h"
#include "keymap_config.h"

KeymapConfig g_keymap;
static char g_keymapName[64] = "Custom";

static const char* INI_GET(const char* section, const char* key, const char* def) {
    static char buf[64];
    GetPrivateProfileStringA(section, key, def, buf, sizeof(buf), KEYMAP_INI_FILENAME);
    return buf;
}

static WORD ReadScanCode(const char* section) {
    const char* s = INI_GET(section, "ScanCode", "");
    if (*s == '\0') return 0;
    return (WORD)strtoul(s, NULL, 0);
}

static DWORD ReadFlags(const char* section) {
    const char* s = INI_GET(section, "Flags", "");
    if (*s == '\0') return KEYEVENTF_SCANCODE;
    return (DWORD)strtoul(s, NULL, 0);
}

static BOOL ReadDefined(const char* section) {
    const char* s = INI_GET(section, "Defined", "");
    if (*s == '\0') return FALSE;
    return s[0] == '1' || s[0] == 't' || s[0] == 'T';
}

static void LoadOne(WORD& scan, DWORD& flags, BOOL& defined,
    const char* section, WORD defScan, DWORD defFlags, BOOL defDefined)
{
    const char* s = INI_GET(section, "ScanCode", "");
    if (*s == '\0') {
        scan = defScan;
    } else {
        scan = (WORD)strtoul(s, NULL, 0);
    }

    s = INI_GET(section, "Flags", "");
    if (*s == '\0') {
        flags = defFlags;
    } else {
        flags = (DWORD)strtoul(s, NULL, 0);
    }

    s = INI_GET(section, "Defined", "");
    if (*s == '\0') {
        defined = defDefined;
    } else {
        defined = (s[0] == '1' || s[0] == 't' || s[0] == 'T');
    }
}

static void WriteOne(const char* section, WORD scan, DWORD flags, BOOL defined) {
    char buf[32];
    sprintf_s(buf, sizeof(buf), "0x%04X", scan);
    WritePrivateProfileStringA(section, "ScanCode", buf, KEYMAP_INI_FILENAME);
    sprintf_s(buf, sizeof(buf), "0x%08X", flags);
    WritePrivateProfileStringA(section, "Flags", buf, KEYMAP_INI_FILENAME);
    WritePrivateProfileStringA(section, "Defined", defined ? "1" : "0", KEYMAP_INI_FILENAME);
}

BOOL LoadKeymapFromINI(const char* filename) {
    // Check if INI exists; if not, use current g_keymap values as-is
    if (GetFileAttributesA(filename) == INVALID_FILE_ATTRIBUTES) {
        return FALSE;
    }

#define LOAD_BINDING(member, section, defScan, defFlags, defDefined) \
    LoadOne(g_keymap.member.scanCode, g_keymap.member.flags, \
        g_keymap.member.defined, section, defScan, defFlags, defDefined)

    LOAD_BINDING(p1_start, "P1_START", DIK_1, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p1_up, "P1_UP", DIK_UP, KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, TRUE);
    LOAD_BINDING(p1_down, "P1_DOWN", DIK_DOWN, KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, TRUE);
    LOAD_BINDING(p1_left, "P1_LEFT", DIK_LEFT, KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, TRUE);
    LOAD_BINDING(p1_right, "P1_RIGHT", DIK_RIGHT, KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, TRUE);
    LOAD_BINDING(p1_btn1, "P1_BTN1", DIK_LCONTROL, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p1_btn2, "P1_BTN2", DIK_LALT, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p1_btn3, "P1_BTN3", DIK_SPACE, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p1_btn4, "P1_BTN4", DIK_LSHIFT, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p1_btn5, "P1_BTN5", DIK_Z, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p1_btn6, "P1_BTN6", DIK_X, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p1_btn7, "P1_BTN7", DIK_C, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p1_btn8, "P1_BTN8", DIK_V, KEYEVENTF_SCANCODE, TRUE);

    LOAD_BINDING(p2_start, "P2_START", DIK_2, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_up, "P2_UP", DIK_R, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_down, "P2_DOWN", DIK_F, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_left, "P2_LEFT", DIK_D, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_right, "P2_RIGHT", DIK_G, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_btn1, "P2_BTN1", DIK_A, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_btn2, "P2_BTN2", DIK_S, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_btn3, "P2_BTN3", DIK_Q, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_btn4, "P2_BTN4", DIK_W, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_btn5, "P2_BTN5", DIK_E, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_btn6, "P2_BTN6", DIK_Y, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_btn7, "P2_BTN7", DIK_H, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p2_btn8, "P2_BTN8", DIK_U, KEYEVENTF_SCANCODE, TRUE);

    LOAD_BINDING(p3_start, "P3_START", DIK_3, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p3_up, "P3_UP", DIK_I, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p3_down, "P3_DOWN", DIK_K, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p3_left, "P3_LEFT", DIK_J, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p3_right, "P3_RIGHT", DIK_L, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p3_btn1, "P3_BTN1", DIK_RCONTROL, KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, TRUE);
    LOAD_BINDING(p3_btn2, "P3_BTN2", DIK_RSHIFT, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p3_btn3, "P3_BTN3", DIK_RETURN, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p3_btn4, "P3_BTN4", DIK_RALT, KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, TRUE);

    LOAD_BINDING(p4_start, "P4_START", DIK_4, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p4_up, "P4_UP", DIK_NUMPAD8, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p4_down, "P4_DOWN", DIK_NUMPAD2, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p4_left, "P4_LEFT", DIK_NUMPAD4, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p4_right, "P4_RIGHT", DIK_NUMPAD6, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p4_btn1, "P4_BTN1", DIK_NUMPAD0, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p4_btn2, "P4_BTN2", DIK_NUMPADPERIOD, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(p4_btn3, "P4_BTN3", DIK_NUMPADENTER, KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, TRUE);
    LOAD_BINDING(p4_btn4, "P4_BTN4", DIK_NUMPADPLUS, KEYEVENTF_SCANCODE, TRUE);

    LOAD_BINDING(s_coin1, "S_COIN1", DIK_5, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_coin2, "S_COIN2", DIK_6, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_coin3, "S_COIN3", DIK_7, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_coin4, "S_COIN4", DIK_8, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_test1, "S_TEST1", DIK_F2, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_test2, "S_TEST2", DIK_F2, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_tilt1, "S_TILT1", DIK_T, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_tilt2, "S_TILT2", DIK_T, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_service1, "S_SERVICE1", DIK_9, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_service2, "S_SERVICE2", DIK_0, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_service3, "S_SERVICE3", DIK_MINUS, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_service4, "S_SERVICE4", DIK_EQUALS, KEYEVENTF_SCANCODE, TRUE);
    LOAD_BINDING(s_esc1, "S_ESC1", DIK_ESCAPE, KEYEVENTF_SCANCODE, TRUE);

#undef LOAD_BINDING

    const char* name = INI_GET("Keymap", "Name", "");
    if (*name) {
        strcpy_s(g_keymapName, sizeof(g_keymapName), name);
    }

    return TRUE;
}

BOOL SaveKeymapToINI(const char* filename) {
    WritePrivateProfileStringA("Keymap", "Name", g_keymapName, filename);

#define SAVE_BINDING(member, section) \
    WriteOne(section, g_keymap.member.scanCode, g_keymap.member.flags, \
        g_keymap.member.defined)

    SAVE_BINDING(p1_start, "P1_START");
    SAVE_BINDING(p1_up, "P1_UP");
    SAVE_BINDING(p1_down, "P1_DOWN");
    SAVE_BINDING(p1_left, "P1_LEFT");
    SAVE_BINDING(p1_right, "P1_RIGHT");
    SAVE_BINDING(p1_btn1, "P1_BTN1");
    SAVE_BINDING(p1_btn2, "P1_BTN2");
    SAVE_BINDING(p1_btn3, "P1_BTN3");
    SAVE_BINDING(p1_btn4, "P1_BTN4");
    SAVE_BINDING(p1_btn5, "P1_BTN5");
    SAVE_BINDING(p1_btn6, "P1_BTN6");
    SAVE_BINDING(p1_btn7, "P1_BTN7");
    SAVE_BINDING(p1_btn8, "P1_BTN8");

    SAVE_BINDING(p2_start, "P2_START");
    SAVE_BINDING(p2_up, "P2_UP");
    SAVE_BINDING(p2_down, "P2_DOWN");
    SAVE_BINDING(p2_left, "P2_LEFT");
    SAVE_BINDING(p2_right, "P2_RIGHT");
    SAVE_BINDING(p2_btn1, "P2_BTN1");
    SAVE_BINDING(p2_btn2, "P2_BTN2");
    SAVE_BINDING(p2_btn3, "P2_BTN3");
    SAVE_BINDING(p2_btn4, "P2_BTN4");
    SAVE_BINDING(p2_btn5, "P2_BTN5");
    SAVE_BINDING(p2_btn6, "P2_BTN6");
    SAVE_BINDING(p2_btn7, "P2_BTN7");
    SAVE_BINDING(p2_btn8, "P2_BTN8");

    SAVE_BINDING(p3_start, "P3_START");
    SAVE_BINDING(p3_up, "P3_UP");
    SAVE_BINDING(p3_down, "P3_DOWN");
    SAVE_BINDING(p3_left, "P3_LEFT");
    SAVE_BINDING(p3_right, "P3_RIGHT");
    SAVE_BINDING(p3_btn1, "P3_BTN1");
    SAVE_BINDING(p3_btn2, "P3_BTN2");
    SAVE_BINDING(p3_btn3, "P3_BTN3");
    SAVE_BINDING(p3_btn4, "P3_BTN4");

    SAVE_BINDING(p4_start, "P4_START");
    SAVE_BINDING(p4_up, "P4_UP");
    SAVE_BINDING(p4_down, "P4_DOWN");
    SAVE_BINDING(p4_left, "P4_LEFT");
    SAVE_BINDING(p4_right, "P4_RIGHT");
    SAVE_BINDING(p4_btn1, "P4_BTN1");
    SAVE_BINDING(p4_btn2, "P4_BTN2");
    SAVE_BINDING(p4_btn3, "P4_BTN3");
    SAVE_BINDING(p4_btn4, "P4_BTN4");

    SAVE_BINDING(s_coin1, "S_COIN1");
    SAVE_BINDING(s_coin2, "S_COIN2");
    SAVE_BINDING(s_coin3, "S_COIN3");
    SAVE_BINDING(s_coin4, "S_COIN4");
    SAVE_BINDING(s_test1, "S_TEST1");
    SAVE_BINDING(s_test2, "S_TEST2");
    SAVE_BINDING(s_tilt1, "S_TILT1");
    SAVE_BINDING(s_tilt2, "S_TILT2");
    SAVE_BINDING(s_service1, "S_SERVICE1");
    SAVE_BINDING(s_service2, "S_SERVICE2");
    SAVE_BINDING(s_service3, "S_SERVICE3");
    SAVE_BINDING(s_service4, "S_SERVICE4");
    SAVE_BINDING(s_esc1, "S_ESC1");

#undef SAVE_BINDING

    return TRUE;
}

void SetKeymapName(const char* name) {
    strcpy_s(g_keymapName, sizeof(g_keymapName), name);
}

const char* GetKeymapName(void) {
    return g_keymapName;
}
