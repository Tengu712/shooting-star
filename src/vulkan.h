#pragma once

#include "common_window_vulkan.h"

// ========================================================================= //
//         Error Messages                                                    //
// ========================================================================= //

typedef enum EMSG_VULKAN {
    EMSG_VULKAN_SUCCESS = 0,
    // initializing
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
    EMSG_CREATE_SAMPLER,
    EMSG_CREATE_DESCRIPTOR,
    EMSG_CREATE_PIPELINE,
    EMSG_CREATE_SQUARE,
    EMSG_CREATE_CAMERA,
    EMSG_CREATE_EMPTY_IMAGE,
    // rendering
    EMSG_UPDATE_CAMERA,
    // image
    EMSG_NULL_OUT_IMAGE,
    EMSG_INVALID_IMAGE_FORMAT,
    EMSG_LOAD_IMAGE_FILE,
    EMSG_LOAD_IMAGE,
} vkres_t;

inline static const char *skd_get_vulkan_error_message(vkres_t res) {
    switch (res) {
        case EMSG_ENUM_INST_EXT_PROPS:
            return "enumerating Vulkan instance extention properties";
        case EMSG_CREATE_INST:
            return "creating Vulkan instance";
        case EMSG_ENUM_PHYS_DEVICES:
            return "enumerating physical devices";
        case EMSG_FIND_QUEUE_FAMILY_INDEX:
            return "finding queue family index";
        case EMSG_ENUM_DEVICE_EXT_PROPS:
            return "enumerating device extention properties";
        case EMSG_CREATE_DEVICE:
            return "creating device";
        case EMSG_CREATE_SURFACE:
            return "creating surface";
        case EMSG_GET_SURFACE_FORMATS:
            return "getting surface formats";
        case EMSG_GET_SURFACE_CAPABILITIES:
            return "getting surface capabiities";
        case EMSG_CREATE_RENDER_PASS:
            return "creating render pass";
        case EMSG_CREATE_SWAPCHAIN:
            return "creating swapchain";
        case EMSG_GET_IMAGES:
            return "getting images";
        case EMSG_CREATE_IMAGE_VIEW:
            return "creating image view";
        case EMSG_CREATE_FRAMEBUFFER:
            return "creating framebuffer";
        case EMSG_CREATE_COMMAND_POOL:
            return "creating command pool";
        case EMSG_ALLOCATE_COMMAND_BUFFERS:
            return "allocating command buffers";
        case EMSG_CREATE_FENCE:
            return "creating fence";
        case EMSG_CREATE_SEMAPHORE:
            return "creating semaphore";
        case EMSG_CREATE_PIPELINE_LAYOUT:
            return "creating pipeline layout";
        case EMSG_CREATE_SHADER:
            return "creating shader";
        case EMSG_CREATE_SAMPLER:
            return "creating sampler";
        case EMSG_CREATE_DESCRIPTOR:
            return "creating descriptor";
        case EMSG_CREATE_PIPELINE:
            return "creating pipeline";
        case EMSG_CREATE_SQUARE:
            return "creating square";
        case EMSG_CREATE_CAMERA:
            return "creating camera";
        case EMSG_CREATE_EMPTY_IMAGE:
            return "creating empty image";
        // rendering
        case EMSG_UPDATE_CAMERA:
            return "updating camera";
        // image
        case EMSG_NULL_OUT_IMAGE:
            return "trying to store image to null Image";
        case EMSG_INVALID_IMAGE_FORMAT:
            return "image format must be RGBA";
        case EMSG_LOAD_IMAGE_FILE:
            return "loading image file";
        case EMSG_LOAD_IMAGE:
            return "loading image";
        default:
            return "unexpected";
    }
}

// ========================================================================= //
//         Structs                                                           //
// ========================================================================= //

typedef struct Image_t *Image;

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
    int is_perse;
    union {
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
    } proj;
} CameraData;

#define DEFAULT_MODEL_DATA { \
        { 1.0f, 1.0f, 1.0f, 0.0f }, \
        { 0.0f, 0.0f, 0.0f, 0.0f }, \
        { 0.0f, 0.0f, 0.0f, 0.0f }, \
        { 1.0f, 1.0f, 1.0f, 1.0f }, \
        { 0.0f, 0.0f, 1.0f, 1.0f }, \
        { 0.0f, 0.0f, 0.0f, 0.0f }, \
    }

// TODO:
#define DEFAULT_CAMERA_DATA { \
        { 0.0f, 0.0f, 0.0f }, \
        { 0.0f, 0.0f, 0.0f }, \
        0, \
        { 1.0f, 1.0f, 1.0f }, \
    }

// ========================================================================= //
//         General                                                           //
// ========================================================================= //

// A function to initialize Vulkan.
vkres_t skd_init_vulkan(SkdWindowParam *window_param);

// A terminator function.
void skd_terminate_vulkan(void);

// ========================================================================= //
//         Rendering                                                         //
// ========================================================================= //

// A function to aquire next image id and wait for a fence.
// It returns 1 if it succeeded.
int skd_prepare_rendering(unsigned int *p_id);

// A function to begin to render.
// To save processing image id, passed first parameter.
// It returns 1 if it succeeded.
int skd_begin_render(unsigned int id, float r, float g, float b);

// A function to end to render.
// It returns 1 if it succeeded.
int skd_end_render(unsigned int id);

// A function to draw.
void skd_draw(ModelData *data);

// ========================================================================= //
//         Image                                                             //
// ========================================================================= //

// A function to load an image from memory.
// The number of channel of the image must be 4 (RGBA).
vkres_t skd_load_image_from_memory(
    const unsigned char *pixels,
    int width,
    int height,
    Image *out
);

// A function to load an image from file.
// The number of channel of the image must be 4 (RGBA).
vkres_t skd_load_image_from_file(const char *path, Image *out);

// A function to unload an image.
void skd_unload_image(Image image);
