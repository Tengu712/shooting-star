#pragma once

#ifdef _WIN32
#define EXPORT __declspec(dllexport) __stdcall
#else
// TODO: for Linux
#define EXPORT
#endif

#include <stdint.h>

// ================================================================================================================= //
//     Logger                                                                                                        //
// ================================================================================================================= //

typedef int warn_t;
#define FB_WARN 0
#define FB_SUCCESS 1

EXPORT warn_t fb_init_logger(void);
EXPORT void fb_indent_logger(void);
EXPORT void fb_dedent_logger(void);
EXPORT void fb_error(const char *msg);
EXPORT warn_t fb_warning(const char *msg);
EXPORT void fb_info(const char *msg);
EXPORT void fb_debug(const char *format, ...);

// ================================================================================================================= //
//     Fireball                                                                                                      //
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

typedef enum RenderingQueryType_t {
    RENDERING_QUERY_TYPE_MODEL,
    RENDERING_QUERY_TYPE_CAMERA,
    RENDERING_QUERY_TYPE_IMAGE_TEXTURE,
} RenderingQueryType;

typedef struct RenderingQuery_t {
    RenderingQueryType kind;
    union {
        ModelData model_data;
        CameraData camera_data;
        uint32_t image_texture_id;
    } data;
} RenderingQuery;

// A function to initialize Fireball.
// This parameters are:
//   1. title: window title
//   2. width: window width
//   3. height: window height
//   4. max_image_num: the max number of image textures
EXPORT warn_t fb_init(const char *title, uint16_t width, uint16_t height, uint32_t max_image_num);

// A function to terminate Fireball.
EXPORT void fb_terminate(void);

// A function to process window event and check if the app should be closed.
EXPORT warn_t fb_should_close(void);

// A function to render entities.
// This parameters are:
//   1. r: red for clearing the screen
//   2. g: green for clearing the screen
//   3. b: blue for clearing the screen
//   4. query: query array for rendering operations
//   5. count: the number of query array
EXPORT warn_t fb_render(float r, float g, float b, const RenderingQuery *query, uint32_t count);
