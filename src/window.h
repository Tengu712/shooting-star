#pragma once

#include "common_window_vulkan.h"

#include <stdint.h>

// ================================================================================================================= //
//         Error Messages                                                                                            //
// ================================================================================================================= //

typedef enum EMSG_WINDOW {
    EMSG_WINDOW_SUCCESS = 0,
    // linux
    EMSG_CONNECT_X,
    EMSG_GET_SETUP,
    EMSG_GET_SCREEN,
    EMSG_CREATE_WINDOW,
    EMSG_CHANGE_PROPERTY,
    // windows
    EMSG_REGISTER_WNDCLASS,
    EMSG_CONVERT_WTITLE,
} wndres_t;

inline static const char *skd_get_window_error_message(int32_t res) {
    switch (res) {
        // linux
        case EMSG_CONNECT_X:
            return "connecting with X server";
        case EMSG_GET_SETUP:
            return "getting a setup";
        case EMSG_GET_SCREEN:
            return "getting a screen";
        case EMSG_CREATE_WINDOW:
            return "creating a window";
        case EMSG_CHANGE_PROPERTY:
            return "changing window property";
        // windows
        case EMSG_REGISTER_WNDCLASS:
            return "registering window class";
        case EMSG_CONVERT_WTITLE:
            return "converting title string from multi-byte to wide";
        default:
            return "unexpected";
    }
}

// ================================================================================================================= //
//         Functions                                                                                                 //
// ================================================================================================================= //

// A function to create window param based on each implementation.
void skd_create_window_param(SkdWindowParam *out);

// A function to create a window.
wndres_t skd_create_window(const char *title, uint16_t width, uint16_t height);

// A function to do all queued window events.
// It returns 1 if program should be closed.
int32_t skd_do_window_events(void);

// A terminator function.
void skd_terminate_window(void);
