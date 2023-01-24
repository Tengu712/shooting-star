#pragma once

#define SKD_WIN_KIND_XCB 0
#define SKD_WIN_KIND_WINAPI 1

// A union for a parameter passed to skd_init_vulkan().
typedef union {
    struct {
        void *connection;
        unsigned int window;
    } xcb_window;
    struct {
        void *hinst;
        void *hwnd;
    } winapi_window;
} SkdWindowUnion;

// A function to initialize Vulkan.
// It returns 0 if it succeeded.
int skd_init_vulkan(int window_kind, SkdWindowUnion *window_param);

// A terminator function.
void skd_terminate_vulkan(void);

// A function to begin to render.
// To save processing image id, passed first parameter.
// It returns 0 if it succeeded.
int skd_begin_render(unsigned int *p_id, float r, float g, float b);

// A function to end to render.
// It returns 0 if it succeeded.
int skd_end_render(unsigned int id);
