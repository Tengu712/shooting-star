#include <stdio.h>

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
    SkdWindowUnion window_param = {
        .xcb_window = {
            skd_get_connection(),
            skd_get_window(),
        },
    };
    res = skd_init_vulkan(SKD_WIN_KIND_XCB, &window_param);
    CHECK("failed to init Vulkan.\n");
    while (1) {
        if (skd_do_window_events() == 1) {
            break;
        }
    }
    skd_terminate_vulkan();
    skd_terminate_window();
    return 0;
}
