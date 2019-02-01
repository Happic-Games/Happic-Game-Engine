#pragma once

/*
* Copied values from https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
*/

#include "../DataStructures/Types.h"

typedef uint16 Key;
typedef uint16 MouseButton;

#define INPUT_KEY_ERROR 0x00
#define INPUT_KEY_ESC 0x01
#define INPUT_KEY_1 0x02
#define INPUT_KEY_2 0x03
#define INPUT_KEY_3 0x04
#define INPUT_KEY_4 0x05
#define INPUT_KEY_5 0x06
#define INPUT_KEY_6 0x07
#define INPUT_KEY_7 0x08
#define INPUT_KEY_8 0x09
#define INPUT_KEY_9 0x0a
#define INPUT_KEY_0 0x0b
#define INPUT_KEY_DASH 0x0c
#define INPUT_KEY_EQUALS 0x0d
#define INPUT_KEY_BACKSPACE 0x0e
#define INPUT_KEY_TAB 0x0f
#define INPUT_KEY_Q 0x10
#define INPUT_KEY_W 0x11
#define INPUT_KEY_E 0x12
#define INPUT_KEY_R 0x13
#define INPUT_KEY_T 0x14
#define INPUT_KEY_Y 0x15
#define INPUT_KEY_U 0x16
#define INPUT_KEY_I 0x17
#define INPUT_KEY_O 0x18
#define INPUT_KEY_P 0x19
#define INPUT_KEY_LEFT_BRACKET 0x1a
#define INPUT_KEY_RIGHT_BRACKET 0x1b
#define INPUT_KEY_ENTER 0x1c
#define INPUT_KEY_CTL 0x1d
#define INPUT_KEY_A 0x1e
#define INPUT_KEY_S 0x1f
#define INPUT_KEY_D 0x20
#define INPUT_KEY_F 0x21
#define INPUT_KEY_G 0x22
#define INPUT_KEY_H 0x23
#define INPUT_KEY_J 0x24
#define INPUT_KEY_K 0x25
#define INPUT_KEY_L 0x26
#define INPUT_KEY_SEMI_COLON 0x27
#define INPUT_KEY_QUOTE 0x28
#define INPUT_KEY_TILDE 0x29
#define INPUT_KEY_LEFT_SHIFT 0x2a
#define INPUT_KEY_BACK_SLASH 0x2b
#define INPUT_KEY_Z 0x2c
#define INPUT_KEY_X 0x2d
#define INPUT_KEY_C 0x2e
#define INPUT_KEY_V 0x2f
#define INPUT_KEY_B 0x30
#define INPUT_KEY_N 0x31
#define INPUT_KEY_M 0x32
#define INPUT_KEY_COMMA 0x33
#define INPUT_KEY_PERIOD 0x34
#define INPUT_KEY_SLASH 0x35
#define INPUT_KEY_RIGHT_SHIFT 0x36
#define INPUT_KEY_KP_ASTERISK 0x37
#define INPUT_KEY_LEFT_ALT 0x38
#define INPUT_KEY_SPACE 0x39 
#define INPUT_KEY_CAPS_LOCK 0x3a
#define INPUT_KEY_F1 0x3b
#define INPUT_KEY_F2 0x3c
#define INPUT_KEY_F3 0x3d
#define INPUT_KEY_F4 0x3e
#define INPUT_KEY_F5 0x3f
#define INPUT_KEY_F6 0x40
#define INPUT_KEY_F7 0x41
#define INPUT_KEY_F8 0x42
#define INPUT_KEY_F9 0x43
#define INPUT_KEY_F10 0x44
#define INPUT_KEY_NUM_LOCK 0x45
#define INPUT_KEY_SCROLL_LOCK 0x46
#define INPUT_KEY_KP_7 0x47
#define INPUT_KEY_KP_8 0x48
#define INPUT_KEY_KP_9 0x49
#define INPUT_KEY_KP_MINUS 0x4a
#define INPUT_KEY_KP_4 0x4b
#define INPUT_KEY_KP_5 0x4c
#define INPUT_KEY_KP_6 0x4d
#define INPUT_KEY_KP_PLUS 0x4e
#define INPUT_KEY_KP_1 0x4f
#define INPUT_KEY_KP_2 0x50
#define INPUT_KEY_KP_3 0x51
#define INPUT_KEY_KP_0 0x52
#define INPUT_KEY_KP_DEL 0x53
#define INPUT_KEY_SYS_RQ 0x54
#define INPUT_KEY_BLANK 0x56
#define INPUT_KEY_F11 0x57
#define INPUT_KEY_F12 0x58
#define INPUT_KEY_UP INPUT_KEY_KP_8
#define INPUT_KEY_DOWN INPUT_KEY_KP_2
#define INPUT_KEY_LEFT INPUT_KEY_KP_4
#define INPUT_KEY_RIGHT INPUT_KEY_KP_6

#define INPUT_BUTTON_LEFT 0
#define INPUT_BUTTON_RIGHT 1
#define INPUT_BUTTON_EXTRA_1 2
#define INPUT_BUTTON_EXTRA_2 3
#define INPUT_BUTTON_EXTRA_3 4
#define INPUT_MOUSE_WHEEL 5

