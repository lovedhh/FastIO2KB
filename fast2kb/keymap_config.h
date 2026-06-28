#pragma once

#include <windows.h>

#define KEYMAP_INI_FILENAME "keymap.ini"

struct KeyBinding {
    WORD scanCode;
    DWORD flags;
    BOOL defined;
};

struct KeymapConfig {
    KeyBinding p1_start, p1_up, p1_down, p1_left, p1_right;
    KeyBinding p1_btn1, p1_btn2, p1_btn3, p1_btn4, p1_btn5, p1_btn6, p1_btn7, p1_btn8;

    KeyBinding p2_start, p2_up, p2_down, p2_left, p2_right;
    KeyBinding p2_btn1, p2_btn2, p2_btn3, p2_btn4, p2_btn5, p2_btn6, p2_btn7, p2_btn8;

    KeyBinding p3_start, p3_up, p3_down, p3_left, p3_right;
    KeyBinding p3_btn1, p3_btn2, p3_btn3, p3_btn4, p3_btn5, p3_btn6, p3_btn7, p3_btn8;

    KeyBinding p4_start, p4_up, p4_down, p4_left, p4_right;
    KeyBinding p4_btn1, p4_btn2, p4_btn3, p4_btn4, p4_btn5, p4_btn6, p4_btn7, p4_btn8;

    KeyBinding s_coin1, s_coin2, s_coin3, s_coin4;
    KeyBinding s_test1, s_test2;
    KeyBinding s_tilt1, s_tilt2;
    KeyBinding s_service1, s_service2, s_service3, s_service4;
    KeyBinding s_esc1, s_esc2, s_esc3, s_esc4;
};

#define KEY_BINDING_COUNT 68
#define KEY_SECTION_NAME_LEN 24

extern KeymapConfig g_keymap;

BOOL LoadKeymapFromINI(const char* filename);
BOOL SaveKeymapToINI(const char* filename);
void SetKeymapName(const char* name);
const char* GetKeymapName(void);
