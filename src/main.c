#include "error.h"
#include "window.h"
#include "vulkan.h"

#include <stdio.h>
#include <math.h>

int32_t main() {
    // start up
    init_logger();
    log_info("Fireball");
    // TODO: load configure
    // TODO: load game logic module
    const uint16_t width = 640;
    const uint16_t height = 480;
    const uint16_t max_image_num = 10;
    // window
    create_window("", width, height);
    SkdWindowParam window_param = {};
    create_window_param(&window_param);
    // vulkan
    init_vulkan(&window_param, max_image_num);
    // mainloop
    int32_t cnt = 0;
    CameraData cameradata = DEFAULT_CAMERA_DATA;
    uint32_t tex_id_1, tex_id_2;
    load_image_from_file("foo.png", &tex_id_1); // TODO: check warning
    load_image_from_file("bar.png", &tex_id_2); // TODO: check warning
    while (1) {
        if (do_window_events() == 1) {
            break;
        }
        uint32_t id;
        prepare_rendering(&id); // TODO: check warning
        cameradata.view_pos.x = sin((float)cnt * 3.1415f / 180.0f);
        update_camera(&cameradata); // TODO: check warning
        begin_render(id, 0.1f, 0.1f, 0.1f); // TODO: check warning
        ModelData modeldata = DEFAULT_MODEL_DATA;
        modeldata.scl.x = 0.1f;
        modeldata.scl.y = 0.1f;
        for (int32_t i = 0; i < 36; ++i) {
            modeldata.trs.x = sin((float)i * 3.1415f / 18.0f);
            modeldata.trs.y = cos((float)i * 3.1415f / 18.0f);
            if (i == 0) {
                use_image_texture(tex_id_1); // TODO: check warning
            } else if (i == 18) {
                use_image_texture(tex_id_2); // TODO: check warning
            }
            draw(&modeldata);
        }
        end_render(id); // TODO: check warning
        if (cnt % 60 == 0) log_debug("%d", cnt / 60);
        cnt += 1;
    }
    // finish
    terminate_vulkan();
    terminate_window();
    return 0;
}
