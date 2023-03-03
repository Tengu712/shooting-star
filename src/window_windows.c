#include "window.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#define EMSG_REGISTER_WNDCLASS 1
#define EMSG_CONVERT_WTITLE 2
#define EMSG_CREATE_WINDOW 3

HINSTANCE g_hinst;
HWND g_hwnd;

LRESULT WINAPI WindowProcedure(HWND hwnd, unsigned int msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

SkdWindowParam *skd_create_window_param(void) {
    SkdWindowParam *window_param = 
        (SkdWindowParam *)malloc(sizeof(SkdWindowParam));
    window_param->kind = SKD_WIN_KIND_WINAPI;
    window_param->data.winapi_window.hinst = (void *)g_hinst;
    window_param->data.winapi_window.hwnd = (void *)g_hwnd;
    return window_param;
}

const char *skd_get_window_error_message(int res) {
    switch (res) {
        case EMSG_REGISTER_WNDCLASS:
            return "failed to register window class";
        case EMSG_CREATE_WINDOW:
            return "failed to create a window";
        default:
            return "unexpected";
    }
}

int skd_create_window(const char *title, unsigned short width, unsigned short height) {
    // instance handle
    g_hinst = GetModuleHandle(NULL);
    // window class
    const DWORD style = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX;
    WNDCLASSEXW wc = {
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
    if (!RegisterClassExW(&wc)) {
        return EMSG_REGISTER_WNDCLASS;
    }
    // adjust window size
    RECT rect = { 0, 0, (long)width, (long)height };
    AdjustWindowRectEx(&rect, style, 0, 0);
    // window
    const size_t titlelen = _mbstrlen(title);
    wchar_t wtitle[256] = {};
    if (!MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, title, -1, wtitle, 256)) {
        return EMSG_CONVERT_WTITLE;
    }
    g_hwnd = CreateWindowExW(
        0,
        L"WIN32APIWINDOW",
        wtitle,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        g_hinst,
        NULL
    );
    if (!g_hwnd) {
        return EMSG_CREATE_WINDOW;
    }
    // finish
    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);
    return 0;
}

int skd_do_window_events(void) {
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
        return 0;
    }
}

void skd_terminate_window(void) {
    UnregisterClassW(L"WIN32APIWINDOW", g_hinst);
    DestroyWindow(g_hwnd);
}
