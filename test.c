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
    SkdWindowParam *window_param = skd_create_window_param();
    res = skd_init_vulkan(window_param);
    CHECK("failed to init Vulkan.\n");
    free(window_param);
    while (1) {
        if (skd_do_window_events() == 1) break;
        unsigned int id;
        if (!skd_begin_render(&id, 1.0f, 0.0f, 0.0f)) break;
        if (!skd_draw(id)) break;
        if (!skd_end_render(id)) break;
    }
    skd_terminate_vulkan();
    skd_terminate_window();
    return 0;
}
