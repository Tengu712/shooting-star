#include "window.h"
#include "vulkan.h"

#include <stdio.h>
#include <math.h>

#define CHECK_WINDOW(f) { \
        wndres = (f); \
        if (wndres != EMSG_WINDOW_SUCCESS) { \
            /* TODO: show it with dialog */ \
            fprintf( \
                stderr, \
                "Fireball: failed to create a window. : %d : %s\n", \
                wndres, \
                skd_get_window_error_message(wndres) \
            ); \
            return vkres; \
        } \
    }
#define CHECK_VULKAN(f) { \
        vkres = (f); \
        if (vkres != EMSG_VULKAN_SUCCESS) { \
            /* TODO: show it with dialog */ \
            fprintf( \
                stderr, \
                "Fireball: failed to initialize Vulkan. : %d : %s\n", \
                vkres, \
                skd_get_vulkan_error_message(vkres) \
            ); \
            return vkres; \
        } \
    }

int32_t main() {
    wndres_t wndres = EMSG_WINDOW_SUCCESS;
    vkres_t vkres = EMSG_VULKAN_SUCCESS;
    // window
    CHECK_WINDOW(skd_create_window("", 640, 480));
    SkdWindowParam window_param = {};
    skd_create_window_param(&window_param);
    // vulkan
    CHECK_VULKAN(skd_init_vulkan(&window_param, 10));
    // mainloop
    int32_t cnt = 0;
    CameraData cameradata = DEFAULT_CAMERA_DATA;
    uint32_t tex_id_1, tex_id_2;
    CHECK_VULKAN(skd_load_image_from_file("foo.png", &tex_id_1));
    CHECK_VULKAN(skd_load_image_from_file("bar.png", &tex_id_2));
    while (1) {
        if (skd_do_window_events() == 1) break;
        uint32_t id;
        CHECK_VULKAN(skd_prepare_rendering(&id));
        cameradata.view_pos.x = sin((float)cnt * 3.1415f / 180.0f);
        CHECK_VULKAN(skd_update_camera(&cameradata));
        CHECK_VULKAN(skd_begin_render(id, 0.1f, 0.1f, 0.1f));
        ModelData modeldata = DEFAULT_MODEL_DATA;
        modeldata.scl.x = 0.1f;
        modeldata.scl.y = 0.1f;
        for (int32_t i = 0; i < 36; ++i) {
            modeldata.trs.x = sin((float)i * 3.1415f / 18.0f);
            modeldata.trs.y = cos((float)i * 3.1415f / 18.0f);
            if (i == 0) {
                CHECK_VULKAN(skd_use_image_texture(tex_id_1));
            } else if (i == 18) {
                CHECK_VULKAN(skd_use_image_texture(tex_id_2));
            }
            skd_draw(&modeldata);
        }
        CHECK_VULKAN(skd_end_render(id));
        if (cnt % 60 == 0) printf("%d\n", cnt / 60);
        cnt += 1;
    }
    // finish
    skd_terminate_vulkan();
    skd_terminate_window();
    return 0;
}
