#include <stdio.h>
#include <stdlib.h>

#include "window.h"
#include "vulkan.h"

int main() {
    int res;
    // window
    // TODO: depend on config
    res = skd_create_window("", 640, 480);
    if (res != 0) {
        // TODO: show it with dialog
        fprintf(
            stderr,
            "Fireball: failed to create a window. : %s\n",
            skd_get_window_error_message(res)
        );
        return res;
    }
    SkdWindowParam window_param = {};
    skd_create_window_param(&window_param);
    // vulkan
    res = skd_init_vulkan(&window_param);
    if (res != 0) {
        // TODO: show it with dialog
        fprintf(
            stderr,
            "Fireball: failed to initialize Vulkan. : %s\n",
            skd_get_vulkan_error_message(res)
        );
        return res;
    }
    // assets
    Image image = {};
    res = skd_load_image_from_file("foo.png", &image);
    if (res != 0) {
        // TODO: show it with dialog
        fprintf(
            stderr,
            "Fireball: failed to load image. : %s\n",
            skd_get_image_error_message(res)
        );
        return res;
    }
    skd_set_image(&image);
    // mainloop
    while (1) {
        if (skd_do_window_events() == 1) break;
        unsigned int id;
        if (!skd_begin_render(&id, 0.1f, 0.1f, 0.1f)) break;
        skd_scale(0.5f, 0.5f, 1.0f);
        skd_trans(0.5f, 0.0f, 0.0f);
        if (!skd_draw(id)) break;
        if (!skd_end_render(id)) break;
    }
    // finish
    skd_unload_image(&image);
    skd_terminate_vulkan();
    skd_terminate_window();
    return 0;
}
