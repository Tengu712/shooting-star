#pragma once

#include "common_window_vulkan.h"

// A function to initialize Vulkan.
// It returns 0 if it succeeded.
int skd_init_vulkan(SkdWindowParam *window_param);

// A terminator function.
void skd_terminate_vulkan(void);

// A function to begin to render.
// To save processing image id, passed first parameter.
// It returns 0 if it succeeded.
int skd_begin_render(unsigned int *p_id, float r, float g, float b);

// A function to end to render.
// It returns 0 if it succeeded.
int skd_end_render(unsigned int id);
