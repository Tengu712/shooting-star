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
#define SS_WARN 0
#define SS_SUCCESS 1

EXPORT void ss_indent_logger(void);
EXPORT void ss_dedent_logger(void);
EXPORT void ss_error(const char *msg);
EXPORT warn_t ss_warning(const char *msg);
EXPORT warn_t ss_warning_fmt(const char *format, ...);
EXPORT void ss_info(const char *msg);
EXPORT void ss_info_fmt(const char *format, ...);
EXPORT void ss_debug(const char *format, ...);

// ================================================================================================================= //
//     Shooting Star                                                                                                 //
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
    Vec4 view_pos;
    Vec4 view_rot;
    struct Ortho {
        float width;
        float height;
        float depth;
        float _dummy;
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
        ModelData *model_data;
        CameraData *camera_data;
        uint32_t image_texture_id;
    } data;
} RenderingQuery;

// A function to initialize Shooting Star.
// This parameters are:
//   1. title: window title
//   2. width: screen width
//   3. height: screen height
//   4. max_image_num: the max number of image textures
EXPORT warn_t ss_init(const char *title, uint16_t width, uint16_t height, uint32_t max_image_num);

// A function to terminate Shooting Star.
EXPORT void ss_terminate(void);

// A function to process window event and check if the app should be closed.
EXPORT warn_t ss_should_close(void);

// A function to render entities.
// This parameters are:
//   1. r: red for clearing the screen
//   2. g: green for clearing the screen
//   3. b: blue for clearing the screen
//   4. query: query array for rendering operations
//   5. count: the number of query array
EXPORT warn_t ss_render(float r, float g, float b, const RenderingQuery *query, uint32_t count);

// A function to load image texture from memory.
// The number of channel of the image must be 4 (RGBA).
EXPORT warn_t ss_load_image_from_memory(const unsigned char *pixels, int32_t width, int32_t height, uint32_t *out_id);

// A function to load an image texture from file.
// The number of channel of the image must be 4 (RGBA).
EXPORT warn_t ss_load_image_from_file(const char *path, uint32_t *out_id);

// A function to unload an image texture.
EXPORT void ss_unload_image(uint32_t id);
