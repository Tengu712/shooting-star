#pragma once

#include <stdint.h>

typedef enum WindowParamType_t {
    WINDOW_PARAM_TYPE_XCB,
    WINDOW_PARAM_TYPE_WIN32,
} WindowParamType;

// A union as an interface for init_vulkan().
typedef struct {
    WindowParamType kind;
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
