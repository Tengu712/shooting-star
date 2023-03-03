#pragma once

#include "common_window_vulkan.h"

// A function to create window param based on each implementation.
void skd_create_window_param(SkdWindowParam *out);

// A function to get error message.
const char *skd_get_window_error_message(int res);

// A function to create a window.
// It returns 0 if it succeeded.
int skd_create_window(const char *title, unsigned short width, unsigned short height);

// A function to do all queued window events.
// It returns 1 if program should be closed.
int skd_do_window_events(void);

// A terminator function.
void skd_terminate_window(void);
