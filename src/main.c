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
    const vkres_t vkres = skd_init_vulkan(&window_param, 10);
    if (vkres != EMSG_VULKAN_SUCCESS) {
        // TODO: show it with dialog
        fprintf(
            stderr,
            "Fireball: failed to initialize Vulkan. : %d : %s\n",
            vkres,
            skd_get_vulkan_error_message(vkres)
        );
        return vkres;
    }
    // mainloop
    int cnt = 0;
    CameraData cameradata = DEFAULT_CAMERA_DATA;
    unsigned int tex_id_1, tex_id_2;
    if (skd_load_image_from_file("foo.png", &tex_id_1) != EMSG_VULKAN_SUCCESS) return -1;
    if (skd_load_image_from_file("bar.png", &tex_id_2) != EMSG_VULKAN_SUCCESS) return -1;
    while (1) {
        if (skd_do_window_events() == 1) break;
        unsigned int id;
        if (!skd_prepare_rendering(&id)) break;
        cameradata.view_pos.x = sin((float)cnt * 3.1415f / 180.0f);
        if (skd_update_camera(&cameradata) != EMSG_VULKAN_SUCCESS) break;
        if (!skd_begin_render(id, 0.1f, 0.1f, 0.1f)) break;
        ModelData modeldata = DEFAULT_MODEL_DATA;
        modeldata.scl.x = 0.1f;
        modeldata.scl.y = 0.1f;
        for (int i = 0; i < 36; ++i) {
            modeldata.trs.x = sin((float)i * 3.1415f / 18.0f);
            modeldata.trs.y = cos((float)i * 3.1415f / 18.0f);
            if (i == 0) {
                if (skd_use_image_texture(tex_id_1) != EMSG_VULKAN_SUCCESS) return -1;
            } else if (i == 18) {
                if (skd_use_image_texture(tex_id_2) != EMSG_VULKAN_SUCCESS) return -1;
            }
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
