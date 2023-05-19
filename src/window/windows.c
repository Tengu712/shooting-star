#include "../window.h"

#include <windows.h>
#include <xinput.h>
#include <stdlib.h>

static HINSTANCE g_hinst;
static HWND g_hwnd;
static int32_t g_input_states[NUM_OF_KEYCODES];

static SsKeycode convert_keycode_win_to_ss(UINT key) {
    switch (key) {
        case 0x41: return KEYCODE_A;
        case 0x42: return KEYCODE_B;
        case 0x43: return KEYCODE_C;
        case 0x44: return KEYCODE_D;
        case 0x45: return KEYCODE_E;
        case 0x46: return KEYCODE_F;
        case 0x47: return KEYCODE_G;
        case 0x48: return KEYCODE_H;
        case 0x49: return KEYCODE_I;
        case 0x4A: return KEYCODE_J;
        case 0x4B: return KEYCODE_K;
        case 0x4C: return KEYCODE_L;
        case 0x4D: return KEYCODE_M;
        case 0x4E: return KEYCODE_N;
        case 0x4F: return KEYCODE_O;
        case 0x50: return KEYCODE_P;
        case 0x51: return KEYCODE_Q;
        case 0x52: return KEYCODE_R;
        case 0x53: return KEYCODE_S;
        case 0x54: return KEYCODE_T;
        case 0x55: return KEYCODE_U;
        case 0x56: return KEYCODE_V;
        case 0x57: return KEYCODE_W;
        case 0x58: return KEYCODE_X;
        case 0x59: return KEYCODE_Y;
        case 0x5A: return KEYCODE_Z;
        case 0x30: return KEYCODE_0;
        case 0x31: return KEYCODE_1;
        case 0x32: return KEYCODE_2;
        case 0x33: return KEYCODE_3;
        case 0x34: return KEYCODE_4;
        case 0x35: return KEYCODE_5;
        case 0x36: return KEYCODE_6;
        case 0x37: return KEYCODE_7;
        case 0x38: return KEYCODE_8;
        case 0x39: return KEYCODE_9;
        case VK_UP: return KEYCODE_UP;
        case VK_LEFT: return KEYCODE_LEFT;
        case VK_DOWN: return KEYCODE_DOWN;
        case VK_RIGHT: return KEYCODE_RIGHT;
        case VK_RETURN: return KEYCODE_ENTER;
        case VK_SPACE: return KEYCODE_SPACE;
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT: return KEYCODE_SHIFT;
        case VK_TAB: return KEYCODE_TAB;
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL: return KEYCODE_CONTROL;
        case VK_ESCAPE: return KEYCODE_ESCAPE;
        default: return NO_KEYCODE;
    }
}

static LRESULT WINAPI WindowProcedure(HWND hwnd, uint32_t msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

void create_window_param(WindowParam *out) {
    out->win32.hinst = (void *)g_hinst;
    out->win32.hwnd = (void *)g_hwnd;
}

warn_t create_window(const char *title, uint32_t width, uint32_t height) {
    ss_info("initializing win32 window ...");
    ss_indent_logger();
    // instance handle
    g_hinst = GetModuleHandle(NULL);
    // window class
    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    const WNDCLASSEXW wc = {
        sizeof(WNDCLASSEXW),
        CS_CLASSDC,
        WindowProcedure,
        0L,
        0L,
        g_hinst,
        NULL,
        NULL,
        NULL,
        NULL,
        L"WIN32APIWINDOW",
        NULL,
    };
    if (!RegisterClassExW(&wc)) ss_error("failed to register window class.");
    // adjust window size
    RECT rect = { 0, 0, (long)width, (long)height };
    AdjustWindowRect(&rect, style, FALSE);
    // window
    wchar_t wtitle[256] = {};
    if (!MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, title, -1, wtitle, 256)) ss_error("failed to convert title multibyte to wide.");
    g_hwnd = CreateWindowExW(
        0,
        L"WIN32APIWINDOW",
        wtitle,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (int)(rect.right - rect.left),
        (int)(rect.bottom - rect.top),
        NULL,
        NULL,
        g_hinst,
        NULL
    );
    if (!g_hwnd) ss_error("failed to create window.");
    // init input states
    memset(g_input_states, 0, sizeof(int32_t) * NUM_OF_KEYCODES);
    // finish
    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);
    ss_dedent_logger();
    ss_info("win32 window initialization succeeded.");
    return 0;
}

int32_t do_window_events(void) {
    MSG msg;
    while (1) {
        if (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return 1;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }
        break;
    }

    // update input states

    const size_t changes_cnt = NUM_OF_KB_KEYS + NUM_OF_JS_BUTTONS;
    char input_state_changes[changes_cnt];
    memset(input_state_changes, 0, sizeof(char) * changes_cnt);

    BYTE states[256];
    GetKeyboardState(states);
    for (int i = 0; i < 256; ++i) {
        const SsKeycode kc = convert_keycode_win_to_ss(i);
        if (kc == NO_KEYCODE)
            continue;
        if (states[i] & 0x80) input_state_changes[kc] = 1;
    }

    XINPUT_STATE xstate;
    memset(&xstate, 0, sizeof(XINPUT_STATE));
    const DWORD is_connected = XInputGetState(0, &xstate);
    if (is_connected == ERROR_SUCCESS) {
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) input_state_changes[KEYCODE_JS_BUTTON_0] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) input_state_changes[KEYCODE_JS_BUTTON_1] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) input_state_changes[KEYCODE_JS_BUTTON_2] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) input_state_changes[KEYCODE_JS_BUTTON_3] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_START) input_state_changes[KEYCODE_JS_BUTTON_4] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) input_state_changes[KEYCODE_JS_BUTTON_5] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) input_state_changes[KEYCODE_JS_BUTTON_6] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) input_state_changes[KEYCODE_JS_BUTTON_7] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) input_state_changes[KEYCODE_JS_BUTTON_8] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) input_state_changes[KEYCODE_JS_BUTTON_9] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_A) input_state_changes[KEYCODE_JS_BUTTON_10] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B) input_state_changes[KEYCODE_JS_BUTTON_11] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_X) input_state_changes[KEYCODE_JS_BUTTON_12] = 1;
        if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_Y) input_state_changes[KEYCODE_JS_BUTTON_13] = 1;
    }

    for (int i = 1; i < changes_cnt; ++i) {
        if (input_state_changes[i] == 1) {
            if (g_input_states[i] < 1) g_input_states[i] = 1;
            else g_input_states[i] += 1;
        } else {
            if (g_input_states[i] > 0) g_input_states[i] = -1;
            else g_input_states[i] = 0;
        }
    }

    // finish
    return 0;
}

void terminate_window(void) {
    UnregisterClassW(L"WIN32APIWINDOW", g_hinst);
    DestroyWindow(g_hwnd);
}

int32_t get_input_state(SsKeycode keycode) {
    return g_input_states[keycode];
}
