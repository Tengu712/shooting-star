#include <stdio.h>

#include "src/window.h"
#include "src/vulkan.h"

int main() {
    int res = skd_create_window("test program", 640, 480);
    if (res != 0) {
        fprintf(stderr, "failed to create a window.\n");
        return res;
    }
    res = skd_init_vulkan();
    if (res != 0) {
        fprintf(stderr, "failed to init Vulkan.\n");
        return res;
    }
    while (1) {
        if (skd_do_window_events() == 1) {
            skd_terminate_window();
            return 0;
        }
    }
    return 0;
}
