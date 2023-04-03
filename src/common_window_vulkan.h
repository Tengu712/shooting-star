#pragma once

#include <stdint.h>

// A union as an interface for init_vulkan().
typedef union {
    struct {
        void *connection;
        uint32_t window;
    } xcb_window;
    struct {
        void *hinst;
        void *hwnd;
    } winapi_window;
} WindowParam;
