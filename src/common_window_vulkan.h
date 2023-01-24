#pragma once

#define SKD_WIN_KIND_XCB 0
#define SKD_WIN_KIND_WINAPI 1

// A union as an interface for skd_init_vulkan().
typedef struct {
    int kind;
    union {
        struct {
            void *connection;
            unsigned int window;
        } xcb_window;
        struct {
            void *hinst;
            void *hwnd;
        } winapi_window;
    } data;
} SkdWindowParam;
