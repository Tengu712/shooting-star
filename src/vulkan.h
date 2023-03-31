#pragma once

#include "common_window_vulkan.h"

#include <stdint.h>

// ================================================================================================================= //
//         Structs                                                                                                   //
// ================================================================================================================= //

typedef struct Vec3_t {
    float x;
    float y;
    float z;
} Vec3;

typedef struct Vec4_t {
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct Mat4_t {
    float a11;
    float a21;
    float a31;
    float a41;
    float a12;
    float a22;
    float a32;
    float a42;
    float a13;
    float a23;
    float a33;
    float a43;
    float a14;
    float a24;
    float a34;
    float a44;
} Mat4;

typedef struct ModelData_t {
    Vec4 scl;
    Vec4 rot;
    Vec4 trs;
    Vec4 col;
    Vec4 uv;
    Vec4 param;
} ModelData;

typedef struct CameraData_t {
    Vec3 view_pos;
    Vec3 view_rot;
    struct Ortho {
        float width;
        float height;
        float depth;
    } ortho;
    struct Perse {
        float pov;
        float aspect;
        float near;
        float far;
    } perse;
} CameraData;

#define DEFAULT_MODEL_DATA { \
        { 1.0f, 1.0f, 1.0f, 0.0f }, \
        { 0.0f, 0.0f, 0.0f, 0.0f }, \
        { 0.0f, 0.0f, 0.0f, 0.0f }, \
        { 1.0f, 1.0f, 1.0f, 1.0f }, \
        { 0.0f, 0.0f, 1.0f, 1.0f }, \
        { 0.0f, 0.0f, 0.0f, 0.0f }, \
    }

#define DEFAULT_CAMERA_DATA { \
        { 0.0f, 0.0f, 0.0f }, \
        { 0.0f, 0.0f, 0.0f }, \
        { 1.0f, 1.0f, 1.0f }, \
        { 45.0f, 1.0f, 0.0f, 1.0f }, \
    }

// ================================================================================================================= //
//         General                                                                                                   //
// ================================================================================================================= //

// A function to initialize Vulkan.
warn_t skd_init_vulkan(SkdWindowParam *window_param, uint32_t max_image_num);

// A terminator function.
void skd_terminate_vulkan(void);

// ================================================================================================================= //
//         Rendering                                                                                                 //
// ================================================================================================================= //

// A function to aquire next image id and wait for a fence.
warn_t skd_prepare_rendering(uint32_t *p_id);

// A function to begin to render.
// To save processing image id, passed first parameter.
warn_t skd_begin_render(uint32_t id, float r, float g, float b);

// A function to end to render.
warn_t skd_end_render(uint32_t id);

// A function to draw.
void skd_draw(ModelData *data);

// ================================================================================================================= //
//         Image                                                                                                     //
// ================================================================================================================= //

// A function to load an image from memory.
// The number of channel of the image must be 4 (RGBA).
// It set texture id to `out` parameter.
warn_t skd_load_image_from_memory(const unsigned char *pixels, int32_t width, int32_t height, uint32_t *out_id);

// A function to load an image from file.
// The number of channel of the image must be 4 (RGBA).
// It set texture id to `out` parameter.
warn_t skd_load_image_from_file(const char *path, uint32_t *out_id);

// A function to unload an image.
void skd_unload_image(uint32_t id);

// ================================================================================================================= //
//         Descriptor Sets                                                                                           //
// ================================================================================================================= //

// A function to update camera.
// It overwrites camera uniform buffer data,
// so you should call it once before rendering.
warn_t skd_update_camera(CameraData *cameradata);

// A function to use image texture.
warn_t skd_use_image_texture(uint32_t id);
