#include "../sstar.h"

#include "../window.h"
#include "../vulkan.h"

EXPORT warn_t ss_init(const char *title, uint16_t width, uint16_t height, uint32_t max_image_num) {
    ss_info("Shooting Star 0.1.0");
    ss_info("initializing Shooting Star ...");
    ss_indent_logger();

    warn_t res = SS_SUCCESS;
    WindowParam window_param;

    if (create_window(title, width, height) != SS_SUCCESS) res = SS_WARN;
    create_window_param(&window_param);
    if (init_vulkan(&window_param, max_image_num) != SS_SUCCESS) res = SS_WARN;

    ss_dedent_logger();
    ss_info("Shooting Star initialization completed.");
    return res;
}

EXPORT void ss_terminate() {
    ss_info("terminating Shooting Star ...");
    terminate_vulkan();
    terminate_window();
    ss_info("Shooting Star termination completed.");
}

EXPORT warn_t ss_should_close() {
    return do_window_events();
}

EXPORT warn_t ss_render(float r, float g, float b, const RenderingQuery *query, uint32_t count) {
    warn_t res = SS_SUCCESS;
    if (prepare_rendering() != SS_SUCCESS) res = SS_WARN;
    if (begin_render(r, g, b) != SS_SUCCESS) res = SS_WARN;

    for (uint32_t i = 0; i < count; ++i) {
        switch (query[i].kind) {
            case RENDERING_QUERY_TYPE_MODEL:
                draw(query[i].data.model_data);
                break;
            case RENDERING_QUERY_TYPE_CAMERA:
                update_camera(query[i].data.camera_data);
                break;
            case RENDERING_QUERY_TYPE_IMAGE_TEXTURE:
                use_image_texture(query[i].data.image_texture_id);
                break;
        }
    }

    if (end_render() != SS_SUCCESS) res = SS_WARN;
    return res;
}

// A function to load image texture from memory.
// The number of channel of the image must be 4 (RGBA).
EXPORT warn_t ss_load_image_from_memory(const unsigned char *pixels, int32_t width, int32_t height, uint32_t *out_id) {
    return load_image_from_memory(pixels, width, height, out_id);
}

// A function to load an image from file.
// The number of channel of the image must be 4 (RGBA).
EXPORT warn_t ss_load_image_from_file(const char *path, uint32_t *out_id) {
    return load_image_from_file(path, out_id);
}

// A function to unload an image.
EXPORT void ss_unload_image(uint32_t id) {
    unload_image(id);
}