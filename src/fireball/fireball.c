#include "../fireball.h"

#include "../window.h"
#include "../vulkan.h"

EXPORT warn_t fb_init(const char *title, uint16_t width, uint16_t height, uint32_t max_image_num) {
    fb_info("Fireball 0.1.0");
    fb_info("initializing Fireball ...");
    fb_indent_logger();

    warn_t res = FB_SUCCESS;
    SkdWindowParam window_param;

    if (create_window(title, width, height) != FB_SUCCESS) res = FB_WARN;
    create_window_param(&window_param);
    if (init_vulkan(&window_param, max_image_num) != FB_SUCCESS) res = FB_WARN;

    fb_dedent_logger();
    fb_info("Fireball initialization completed.");
    return res;
}

EXPORT void fb_terminate() {
    fb_info("terminating Fireball ...");
    terminate_vulkan();
    terminate_window();
    fb_info("Fireball termination completed.");
}

EXPORT warn_t fb_should_close() {
    return do_window_events();
}

EXPORT warn_t fb_render(float r, float g, float b) {
    warn_t res = FB_SUCCESS;
    uint32_t id;
    if (prepare_rendering(&id) != FB_SUCCESS) res = FB_WARN;
    if (begin_render(id, r, g, b) != FB_SUCCESS) res = FB_WARN;
    if (end_render(id) != FB_SUCCESS) res = FB_WARN;
    return res;
}
