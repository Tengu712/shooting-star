#pragma once

#include "common_window_vulkan.h"

enum EMSG_VULKAN {
    EMSG_SUCCESS = 0,
    EMSG_ENUM_INST_EXT_PROPS,
    EMSG_CREATE_INST,
    EMSG_ENUM_PHYS_DEVICES,
    EMSG_FIND_QUEUE_FAMILY_INDEX,
    EMSG_ENUM_DEVICE_EXT_PROPS,
    EMSG_CREATE_DEVICE,
    EMSG_CREATE_SURFACE,
    EMSG_GET_SURFACE_FORMATS,
    EMSG_GET_SURFACE_CAPABILITIES,
    EMSG_CREATE_RENDER_PASS,
    EMSG_CREATE_SWAPCHAIN,
    EMSG_GET_IMAGES,
    EMSG_CREATE_IMAGE_VIEW,
    EMSG_CREATE_FRAMEBUFFER,
    EMSG_CREATE_COMMAND_POOL,
    EMSG_ALLOCATE_COMMAND_BUFFERS,
    EMSG_CREATE_FENCE,
    EMSG_CREATE_SEMAPHORE,
    EMSG_CREATE_PIPELINE_LAYOUT,
    EMSG_CREATE_SHADER,
    EMSG_CREATE_BUFFER,
    EMSG_CREATE_SAMPLER,
    EMSG_CREATE_DESCRIPTOR,
    EMSG_CREATE_PIPELINE,
    EMSG_CREATE_SQUARE,
    EMSG_MAP_UBO,
    // image
    EMSG_NULL_OUT_IMAGE,
    EMSG_INVALID_IMAGE_FORMAT,
    EMSG_LOAD_IMAGE,
    EMSG_LOAD_IMAGE_FILE,
};

typedef struct Image_t {
    void *image;
    void *view;
    void *memory;
} Image;


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

// A function to draw square.
// It returns 0 if it succeeded.
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
