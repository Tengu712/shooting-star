#pragma once

#include "sstar.h"
#include "common_window_vulkan.h"

// A function to create window param based on each implementation.
void create_window_param(WindowParam *out);

// A function to create a window.
warn_t create_window(const char *title, uint32_t width, uint32_t height);

// A function to do all queued window events.
// It returns 1 if program should be closed.
int32_t do_window_events(void);

// A terminator function.
void terminate_window(void);

int32_t get_input_state(SsKeyCode key_code);
