#pragma once

#include <stdint.h>

// A union as an interface for init_vulkan().
typedef union {
    struct {
        void *display;
        uint32_t window;
    } xlib;
    struct {
        void *hinst;
        void *hwnd;
    } win32;
} WindowParam;
