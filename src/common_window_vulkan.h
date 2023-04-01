#pragma once

#include <stdint.h>

#define SKD_WIN_KIND_XCB 0
#define SKD_WIN_KIND_WINAPI 1

// A union as an interface for init_vulkan().
typedef struct {
    int32_t kind;
    union {
        struct {
            void *connection;
            uint32_t window;
        } xcb_window;
        struct {
            void *hinst;
            void *hwnd;
        } winapi_window;
    } data;
} WindowParam;
