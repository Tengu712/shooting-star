#include "window.h"
#include "vulkan.h"

#include <stdio.h>
#include <math.h>

int main() {
    int res;
    // window
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
    if (skd_init_vulkan(&window_param) != EMSG_VULKAN_SUCCESS) {
        // TODO: show it with dialog
        fprintf(
            stderr,
            "Fireball: failed to initialize Vulkan. : %s\n",
            skd_get_vulkan_error_message(res)
        );
        return res;
    }
    // mainloop
    int cnt = 0;
    while (1) {
        if (skd_do_window_events() == 1) break;
        unsigned int id;
        if (!skd_prepare_rendering(&id)) break;
        if (!skd_begin_render(id, 0.1f, 0.1f, 0.1f)) break;
        ModelData modeldata = DEFAULT_MODEL_DATA;
        modeldata.scl.x = 0.01f;
        modeldata.scl.y = 0.01f;
        for (int i = 0; i < 3600; ++i) {
            modeldata.trs.x = sin((float)i * 3.1415f / 1800.0f);
            modeldata.trs.y = cos((float)i * 3.1415f / 1800.0f);
            skd_draw(&modeldata);
        }
        if (!skd_end_render(id)) break;
        if (cnt % 60 == 0) printf("%d\n", cnt / 60);
        cnt += 1;
    }
    // finish
    skd_terminate_vulkan();
    skd_terminate_window();
    return 0;
}
