#pragma once

#include "common_window_vulkan.h"

typedef struct Image_t {
    void *image;
    void *view;
    void *memory;
} Image;

// A function to get error message.
const char *skd_get_vulkan_error_message(int res);
const char *skd_get_image_error_message(int res);

// A function to initialize Vulkan.
// It returns 0 if it succeeded.
int skd_init_vulkan(SkdWindowParam *window_param);

// A terminator function.
void skd_terminate_vulkan(void);

// A function to begin to render.
// To save processing image id, passed first parameter.
// It returns 1 if it succeeded.
int skd_begin_render(unsigned int *p_id, float r, float g, float b);

// A function to end to render.
// It returns 1 if it succeeded.
int skd_end_render(unsigned int id);

// A function to draw square.
// It returns 1 if it succeeded.
int skd_draw(int id);

// A function to load an image from memory.
// The number of channel of the image must be 4 (RGBA).
// It returns 0 if it succeeded.
int skd_load_image_from_memory(
    const unsigned char *pixels,
    int width,
    int height,
    Image *out
);

// A function to load an image from file.
// The number of channel of the image must be 4 (RGBA).
// It returns 0 if it succeeded.
int skd_load_image_from_file(const char *path, Image *out);

// A function to set an image.
// If image is NULL, it mades shader not use image sampler.
void skd_set_image(Image *image);

// A function to unload an image.
void skd_unload_image(Image *image);

// Functions to update ubo.
void skd_scale(float x, float y, float z);
void skd_trans(float x, float y, float z);
void skd_uv(float u, float v, float u_end, float v_end);
