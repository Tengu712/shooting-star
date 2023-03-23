#include "error.h"
#include "window.h"
#include "vulkan.h"

#include <stdio.h>
#include <math.h>

int32_t main() {
    // start up
    init_logger();
    log_info("Starting up Fireball");
    // TODO: load configure
    // TODO: load game logic module
    const uint16_t width = 640;
    const uint16_t height = 480;
    const uint16_t max_image_num = 10;
    // window
    skd_create_window("", width, height);
    SkdWindowParam window_param = {};
    skd_create_window_param(&window_param);
    // vulkan
    skd_init_vulkan(&window_param, max_image_num);
    // mainloop
    int32_t cnt = 0;
    CameraData cameradata = DEFAULT_CAMERA_DATA;
    uint32_t tex_id_1, tex_id_2;
    skd_load_image_from_file("foo.png", &tex_id_1); // TODO: check warning
    skd_load_image_from_file("bar.png", &tex_id_2); // TODO: check warning
    while (1) {
        if (skd_do_window_events() == 1) {
            break;
        }
        uint32_t id;
        skd_prepare_rendering(&id); // TODO: check warning
        cameradata.view_pos.x = sin((float)cnt * 3.1415f / 180.0f);
        skd_update_camera(&cameradata); // TODO: check warning
        skd_begin_render(id, 0.1f, 0.1f, 0.1f); // TODO: check warning
        ModelData modeldata = DEFAULT_MODEL_DATA;
        modeldata.scl.x = 0.1f;
        modeldata.scl.y = 0.1f;
        for (int32_t i = 0; i < 36; ++i) {
            modeldata.trs.x = sin((float)i * 3.1415f / 18.0f);
            modeldata.trs.y = cos((float)i * 3.1415f / 18.0f);
            if (i == 0) {
                skd_use_image_texture(tex_id_1); // TODO: check warning
            } else if (i == 18) {
                skd_use_image_texture(tex_id_2); // TODO: check warning
            }
            skd_draw(&modeldata);
        }
        skd_end_render(id); // TODO: check warning
        if (cnt % 60 == 0) log_debug("%d", cnt / 60);
        cnt += 1;
    }
    // finish
    skd_terminate_vulkan();
    skd_terminate_window();
    return 0;
}
