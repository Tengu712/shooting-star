#pragma once

#include "sstar.h"
#include "common_window_vulkan.h"

#include <stdint.h>

// ================================================================================================================= //
//         General                                                                                                   //
// ================================================================================================================= //

// A function to initialize Vulkan.
warn_t init_vulkan(const WindowParam *window_param, uint32_t max_image_texture_cnt);

// A terminator function.
void terminate_vulkan(void);

// ================================================================================================================= //
//         Rendering                                                                                                 //
// ================================================================================================================= //

// A function to prepare rendering.
warn_t prepare_rendering(void);

// A function to begin to render.
// To save processing image id, passed first parameter.
warn_t begin_render(float r, float g, float b);

// A function to end to render.
warn_t end_render(void);

// A function to draw.
void draw(const ModelData *data);

// A function to update camera.
// It overwrites camera uniform buffer data,
// so you should call it once before rendering.
warn_t update_camera(const CameraData *cameradata);

// A function to use image texture.
warn_t use_image_texture(uint32_t id);

// ================================================================================================================= //
//         Image                                                                                                     //
// ================================================================================================================= //

// A function to load an image from memory.
// The number of channel of the image must be 4 (RGBA).
// It set texture id to `out` parameter.
warn_t load_image_from_memory(const unsigned char *pixels, int32_t width, int32_t height, uint32_t *out_id);

// A function to unload an image.
void unload_image(uint32_t id);
