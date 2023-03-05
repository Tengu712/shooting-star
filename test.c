#include <stdio.h>
#include <stdlib.h>

#include "src/window.h"
#include "src/vulkan.h"

#define CHECK(p)              \
    if (res != 0) {           \
        fprintf(stderr, (p)); \
        return res;           \
    }

int main() {
    int res;
    res = skd_create_window("test program", 640, 480);
    CHECK("failed to create a window.\n");
    SkdWindowParam window_param = {};
    skd_create_window_param(&window_param);
    res = skd_init_vulkan(&window_param);
    CHECK("failed to init Vulkan.\n");
    Image image = {};
    res = skd_load_image_from_file("foo.png", &image);
    CHECK("failed to load image.\n");
    skd_set_image(&image);
    while (1) {
        if (skd_do_window_events() == 1) break;
        unsigned int id;
        if (!skd_begin_render(&id, 1.0f, 0.0f, 0.0f)) break;
        skd_scale(0.5f, 0.5f, 1.0f);
        skd_trans(0.5f, 0.0f, 0.0f);
        if (!skd_draw(id)) break;
        if (!skd_end_render(id)) break;
    }
    skd_unload_image(&image);
    skd_terminate_vulkan();
    skd_terminate_window();
    return 0;
}
