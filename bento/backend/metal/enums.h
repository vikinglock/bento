#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include <regex>
#include <simd/simd.h>


#ifdef IMGUI
#include "../../lib/imgui/imgui.h"
#include "../../lib/imgui/backends/imgui_impl_metal.h"
#include "../../lib/imgui/backends/imgui_impl_osx.h"
#endif

#include <Metal/Metal.h>
#include "metalcommon.h"

#ifdef IOS
enum KeyCode {
    KEY_A = 0x04,KEY_B = 0x05,KEY_C = 0x06,KEY_D = 0x07,KEY_E = 0x08,
    KEY_F = 0x09,KEY_G = 0x0A,KEY_H = 0x0B,KEY_I = 0x0C,KEY_J = 0x0D,
    KEY_K = 0x0E,KEY_L = 0x0F,KEY_M = 0x10,KEY_N = 0x11,KEY_O = 0x12,
    KEY_P = 0x13,KEY_Q = 0x14,KEY_R = 0x15,KEY_S = 0x16,KEY_T = 0x17,
    KEY_U = 0x18,KEY_V = 0x19,KEY_W = 0x1A,KEY_X = 0x1B,KEY_Y = 0x1C,
    KEY_Z = 0x1D,

    KEY_1 = 0x1E,KEY_2 = 0x1F,KEY_3 = 0x20,
    KEY_4 = 0x21,KEY_5 = 0x22,KEY_6 = 0x23,
    KEY_7 = 0x24,KEY_8 = 0x25,KEY_9 = 0x26,
    KEY_0 = 0x27,

    KEY_RETURN = 0x28,
    KEY_ESCAPE = 0x29,
    KEY_BACKSPACE = 0x2A,
    KEY_TAB = 0x2B,
    KEY_SPACE = 0x2C,

    KEY_MINUS = 0x2D,
    KEY_EQUALS = 0x2E,
    KEY_LEFT_BRACKET = 0x2F,
    KEY_RIGHT_BRACKET = 0x30,
    KEY_BACKSLASH = 0x31,
    KEY_SEMICOLON = 0x33,
    KEY_QUOTE = 0x34,
    KEY_GRAVE = 0x35,
    KEY_COMMA = 0x36,
    KEY_PERIOD = 0x37,
    KEY_SLASH = 0x38,

    KEY_CAPSLOCK = 0x39,

    KEY_F1 = 0x3A,KEY_F2 = 0x3B,KEY_F3 = 0x3C,
    KEY_F4 = 0x3D,KEY_F5 = 0x3E,KEY_F6 = 0x3F,
    KEY_F7 = 0x40,KEY_F8 = 0x41,KEY_F9 = 0x42,
    KEY_F10 = 0x43,KEY_F11 = 0x44,KEY_F12 = 0x45,

    KEY_RIGHT = 0x4F,
    KEY_LEFT  = 0x50,
    KEY_DOWN  = 0x51,
    KEY_UP    = 0x52,

    KEY_INSERT = 0x49,
    KEY_HOME   = 0x4A,
    KEY_PAGEUP = 0x4B,
    KEY_DELETE = 0x4C,
    KEY_END    = 0x4D,
    KEY_PAGEDOWN = 0x4E,

    KEY_NUM_LOCK = 0x53,
    KEY_NUMPAD_DIVIDE = 0x54,
    KEY_NUMPAD_MULTIPLY = 0x55,
    KEY_NUMPAD_SUBTRACT = 0x56,
    KEY_NUMPAD_ADD = 0x57,
    KEY_NUMPAD_ENTER = 0x58,
    KEY_NUMPAD_1 = 0x59,KEY_NUMPAD_2 = 0x5A,KEY_NUMPAD_3 = 0x5B,
    KEY_NUMPAD_4 = 0x5C,KEY_NUMPAD_5 = 0x5D,KEY_NUMPAD_6 = 0x5E,
    KEY_NUMPAD_7 = 0x5F,KEY_NUMPAD_8 = 0x60,KEY_NUMPAD_9 = 0x61,
    KEY_NUMPAD_0 = 0x62,
    KEY_NUMPAD_DECIMAL = 0x63,

    KEY_NUMPAD_EQUALS = 0x67,
    KEY_KP_COMMA = 0x85,
    KEY_KP_LEFTPAREN = 0xB6,
    KEY_KP_RIGHTPAREN = 0xB7,

    KEY_LEFT_CONTROL = 0xE0,
    KEY_LEFT_SHIFT = 0xE1,
    KEY_LEFT_ALT = 0xE2,
    KEY_LEFT_OPTION = 0xE6,
    KEY_LEFT_COMMAND = 0xE3,
    KEY_RIGHT_CONTROL = 0xE4,
    KEY_RIGHT_SHIFT = 0xE5,
    KEY_RIGHT_ALT = 0xE6,
    KEY_RIGHT_OPTION = 0xE6,
    KEY_RIGHT_COMMAND = 0xE7
};
#else
#include <Carbon/Carbon.h>
enum KeyCode {
    //  #####     KEYS     #####
    KEY_F1 = kVK_F1, KEY_F2  = kVK_F2 , KEY_F3  = kVK_F3 , KEY_F4  = kVK_F4 ,
    KEY_F5 = kVK_F5, KEY_F6  = kVK_F6 , KEY_F7  = kVK_F7 , KEY_F8  = kVK_F8 ,
    KEY_F9 = kVK_F9, KEY_F10 = kVK_F10, KEY_F11 = kVK_F11, KEY_F12 = kVK_F12,

    KEY_F13 = kVK_F13, KEY_F14 = kVK_F14, KEY_F15 = kVK_F15, KEY_F16 = kVK_F16,
    KEY_F17 = kVK_F17, KEY_F18 = kVK_F18, KEY_F19 = kVK_F19, KEY_F20 = kVK_F20,

    KEY_NUMPAD_0 = kVK_ANSI_Keypad0,
    KEY_NUMPAD_1 = kVK_ANSI_Keypad1, KEY_NUMPAD_2 = kVK_ANSI_Keypad2, KEY_NUMPAD_3 = kVK_ANSI_Keypad3,
    KEY_NUMPAD_4 = kVK_ANSI_Keypad4, KEY_NUMPAD_5 = kVK_ANSI_Keypad5, KEY_NUMPAD_6 = kVK_ANSI_Keypad6,
    KEY_NUMPAD_7 = kVK_ANSI_Keypad7, KEY_NUMPAD_8 = kVK_ANSI_Keypad8, KEY_NUMPAD_9 = kVK_ANSI_Keypad9,

    KEY_NUM_LOCK = kVK_ANSI_KeypadClear,
    KEY_NUMPAD_ADD = kVK_ANSI_KeypadPlus,
    KEY_NUMPAD_SUBTRACT = kVK_ANSI_KeypadMinus,
    KEY_NUMPAD_MULTIPLY = kVK_ANSI_KeypadMultiply,
    KEY_NUMPAD_DIVIDE = kVK_ANSI_KeypadDivide,
    KEY_NUMPAD_DECIMAL = kVK_ANSI_KeypadDecimal,
    KEY_NUMPAD_EQUALS = kVK_ANSI_KeypadEquals,
    KEY_NUMPAD_ENTER = kVK_ANSI_KeypadEnter,
    KEY_NUMPAD_CLEAR = kVK_ANSI_KeypadClear,

    KEY_A = kVK_ANSI_A, KEY_B = kVK_ANSI_B, KEY_C = kVK_ANSI_C, KEY_D = kVK_ANSI_D, KEY_E = kVK_ANSI_E,
    KEY_F = kVK_ANSI_F, KEY_G = kVK_ANSI_G, KEY_H = kVK_ANSI_H, KEY_I = kVK_ANSI_I, KEY_J = kVK_ANSI_J,
    KEY_K = kVK_ANSI_K, KEY_L = kVK_ANSI_L, KEY_M = kVK_ANSI_M, KEY_N = kVK_ANSI_N, KEY_O = kVK_ANSI_O,
    KEY_P = kVK_ANSI_P, KEY_Q = kVK_ANSI_Q, KEY_R = kVK_ANSI_R, KEY_S = kVK_ANSI_S, KEY_T = kVK_ANSI_T,
    KEY_U = kVK_ANSI_U, KEY_V = kVK_ANSI_V, KEY_W = kVK_ANSI_W, KEY_X = kVK_ANSI_X, KEY_Y = kVK_ANSI_Y,
    KEY_Z = kVK_ANSI_Z,

    KEY_0 = kVK_ANSI_0, KEY_1 = kVK_ANSI_1, KEY_2 = kVK_ANSI_2, KEY_3 = kVK_ANSI_3, KEY_4 = kVK_ANSI_4,
    KEY_5 = kVK_ANSI_5, KEY_6 = kVK_ANSI_6, KEY_7 = kVK_ANSI_7, KEY_8 = kVK_ANSI_8, KEY_9 = kVK_ANSI_9, // beautiful

    KEY_SPACE = kVK_Space, KEY_TAB = kVK_Tab, KEY_RETURN = kVK_Return,
    KEY_DELETE = kVK_ForwardDelete, KEY_ESCAPE = kVK_Escape, KEY_BACKSPACE = kVK_Delete,

    KEY_VOLUME_UP = kVK_VolumeUp, KEY_VOLUME_DOWN = kVK_VolumeDown, KEY_MUTE = kVK_Mute,
    KEY_HELP = kVK_Help, KEY_INSERT = kVK_Help, KEY_HOME = kVK_Home,
    KEY_END = kVK_End, KEY_PAGE_DOWN = kVK_PageDown, KEY_PAGE_UP = kVK_PageUp,

    KEY_MINUS = kVK_ANSI_Minus,
    KEY_EQUALS = kVK_ANSI_Equal,
    KEY_LEFT_BRACKET = kVK_ANSI_LeftBracket,
    KEY_RIGHT_BRACKET = kVK_ANSI_RightBracket,
    KEY_BACKSLASH = kVK_ANSI_Backslash,
    KEY_SEMICOLON = kVK_ANSI_Semicolon,
    KEY_QUOTE = kVK_ANSI_Quote,
    KEY_COMMA = kVK_ANSI_Comma,
    KEY_GRAVE = kVK_ANSI_Grave,
    KEY_PERIOD = kVK_ANSI_Period,
    KEY_SLASH = kVK_ANSI_Slash,
    KEY_UP = kVK_UpArrow, KEY_DOWN = kVK_DownArrow, KEY_LEFT = kVK_LeftArrow, KEY_RIGHT = kVK_RightArrow,
    KEY_FUNCTION = kVK_Function, KEY_CAPS_LOCK = kVK_CapsLock,
    KEY_LEFT_SHIFT = kVK_Shift, KEY_LEFT_CONTROL = kVK_Control, KEY_LEFT_OPTION = kVK_Option, KEY_LEFT_ALT = kVK_Option, KEY_LEFT_COMMAND = kVK_Command,
    KEY_RIGHT_SHIFT = 0x39, KEY_RIGHT_CONTROL = 0x3C, KEY_RIGHT_OPTION = 0x3D, KEY_RIGHT_ALT = 0x3D, KEY_RIGHT_COMMAND = 0x36,
    //  #####     MOUSE BUTTONS     #####
    MOUSE_LEFT = 0x0,
    MOUSE_RIGHT = 0x1,
    MOUSE_MIDDLE = 0x2,
    MOUSE_1 = 0x0,
    MOUSE_2 = 0x1,
    MOUSE_3 = 0x2,
    MOUSE_4 = 0x3,
    MOUSE_5 = 0x4,

    //  #####     OTHER     #####
};
#endif

#ifdef IMGUI
//because dearimgui doesn't give a keycallback for metal?????????
static std::unordered_map<int, ImGuiKey> KeytoDearImguiKey = {
    {KEY_F1, ImGuiKey_F1}, {KEY_F2, ImGuiKey_F2}, {KEY_F3, ImGuiKey_F3}, {KEY_F4, ImGuiKey_F4},
    {KEY_F5, ImGuiKey_F5}, {KEY_F6, ImGuiKey_F6}, {KEY_F7, ImGuiKey_F7}, {KEY_F8, ImGuiKey_F8},
    {KEY_F9, ImGuiKey_F9}, {KEY_F10, ImGuiKey_F10}, {KEY_F11, ImGuiKey_F11}, {KEY_F12, ImGuiKey_F12},


    {KEY_NUMPAD_0, ImGuiKey_Keypad0},
    {KEY_NUMPAD_1, ImGuiKey_Keypad1},{KEY_NUMPAD_2, ImGuiKey_Keypad2},{KEY_NUMPAD_3, ImGuiKey_Keypad3},
    {KEY_NUMPAD_4, ImGuiKey_Keypad4},{KEY_NUMPAD_5, ImGuiKey_Keypad5},{KEY_NUMPAD_6, ImGuiKey_Keypad6},
    {KEY_NUMPAD_7, ImGuiKey_Keypad7},{KEY_NUMPAD_8, ImGuiKey_Keypad8},{KEY_NUMPAD_9, ImGuiKey_Keypad9},

    {KEY_NUM_LOCK, ImGuiKey_NumLock},
    {KEY_NUMPAD_ADD, ImGuiKey_KeypadAdd},
    {KEY_NUMPAD_SUBTRACT, ImGuiKey_KeypadSubtract},
    {KEY_NUMPAD_MULTIPLY, ImGuiKey_KeypadMultiply},
    {KEY_NUMPAD_DIVIDE, ImGuiKey_KeypadDivide},
    {KEY_NUMPAD_DECIMAL, ImGuiKey_KeypadDecimal},
    {KEY_NUMPAD_ENTER, ImGuiKey_KeypadEnter},

    {KEY_A, ImGuiKey_A}, {KEY_B, ImGuiKey_B}, {KEY_C, ImGuiKey_C}, {KEY_D, ImGuiKey_D},
    {KEY_E, ImGuiKey_E}, {KEY_F, ImGuiKey_F}, {KEY_G, ImGuiKey_G}, {KEY_H, ImGuiKey_H},
    {KEY_I, ImGuiKey_I}, {KEY_J, ImGuiKey_J}, {KEY_K, ImGuiKey_K}, {KEY_L, ImGuiKey_L},
    {KEY_M, ImGuiKey_M}, {KEY_N, ImGuiKey_N}, {KEY_O, ImGuiKey_O}, {KEY_P, ImGuiKey_P},
    {KEY_Q, ImGuiKey_Q}, {KEY_R, ImGuiKey_R}, {KEY_S, ImGuiKey_S}, {KEY_T, ImGuiKey_T},
    {KEY_U, ImGuiKey_U}, {KEY_V, ImGuiKey_V}, {KEY_W, ImGuiKey_W}, {KEY_X, ImGuiKey_X},
    {KEY_Y, ImGuiKey_Y}, {KEY_Z, ImGuiKey_Z},

    {KEY_0, ImGuiKey_0}, {KEY_1, ImGuiKey_1}, {KEY_2, ImGuiKey_2}, {KEY_3, ImGuiKey_3},
    {KEY_4, ImGuiKey_4}, {KEY_5, ImGuiKey_5}, {KEY_6, ImGuiKey_6}, {KEY_7, ImGuiKey_7},
    {KEY_8, ImGuiKey_8}, {KEY_9, ImGuiKey_9},

    {KEY_SPACE, ImGuiKey_Space}, {KEY_TAB, ImGuiKey_Tab}, {KEY_RETURN, ImGuiKey_Enter},
    {KEY_DELETE, ImGuiKey_Delete}, {KEY_BACKSPACE, ImGuiKey_Backspace}, {KEY_ESCAPE, ImGuiKey_Escape},

    {KEY_UP, ImGuiKey_UpArrow}, {KEY_DOWN, ImGuiKey_DownArrow}, {KEY_LEFT, ImGuiKey_LeftArrow}, {KEY_RIGHT, ImGuiKey_RightArrow},

    {KEY_LEFT_SHIFT, ImGuiKey_LeftShift}, {KEY_RIGHT_SHIFT, ImGuiKey_RightShift},
    {KEY_LEFT_CONTROL, ImGuiKey_LeftCtrl}, {KEY_RIGHT_CONTROL, ImGuiKey_RightCtrl},
    {KEY_LEFT_OPTION, ImGuiKey_LeftAlt}, {KEY_RIGHT_OPTION, ImGuiKey_RightAlt},
    {KEY_LEFT_ALT, ImGuiKey_LeftAlt}, {KEY_RIGHT_ALT, ImGuiKey_RightAlt},
    {KEY_LEFT_COMMAND, ImGuiKey_LeftSuper}, {KEY_RIGHT_COMMAND, ImGuiKey_RightSuper},

    {KEY_MINUS, ImGuiKey_Minus}, {KEY_EQUALS, ImGuiKey_Equal},
    {KEY_LEFT_BRACKET, ImGuiKey_LeftBracket}, {KEY_RIGHT_BRACKET, ImGuiKey_RightBracket},
    {KEY_BACKSLASH, ImGuiKey_Backslash}, {KEY_SEMICOLON, ImGuiKey_Semicolon}, {KEY_QUOTE, ImGuiKey_Apostrophe},
    {KEY_COMMA, ImGuiKey_Comma},
    {KEY_GRAVE, ImGuiKey_GraveAccent},
    {KEY_PERIOD, ImGuiKey_Period},
    {KEY_SLASH, ImGuiKey_Slash},
};
#endif

enum JoystickType{
    GAMEPAD_JOYSTICK_LEFT,
    GAMEPAD_JOYSTICK_RIGHT,
};
enum ButtonType{
    GAMEPAD_KEY_A,
    GAMEPAD_KEY_B,
    GAMEPAD_KEY_X,
    GAMEPAD_KEY_Y,

    GAMEPAD_KEY_R,
    GAMEPAD_KEY_R2,
    GAMEPAD_KEY_R3,

    GAMEPAD_KEY_L,
    GAMEPAD_KEY_L2,
    GAMEPAD_KEY_L3,

    GAMEPAD_KEY_START,
    GAMEPAD_KEY_SELECT,
    GAMEPAD_KEY_HOME,
    GAMEPAD_KEY_SCREENSHOT,

    GAMEPAD_KEY_UP,
    GAMEPAD_KEY_DOWN,
    GAMEPAD_KEY_LEFT,
    GAMEPAD_KEY_RIGHT,
};
enum class Primitive{
    Points,
    Lines,
    LineStrip,
    Triangles,
    TriangleStrip,
};