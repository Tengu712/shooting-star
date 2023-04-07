#include "sstar.h"

#define DEFAULT_MODEL_DATA { \
        { 1.0f, 1.0f, 1.0f, 1.0f }, \
        { 0.0f, 0.0f, 0.0f, 1.0f }, \
        { 0.0f, 0.0f, 0.0f, 1.0f }, \
        { 1.0f, 1.0f, 1.0f, 1.0f }, \
        { 0.0f, 0.0f, 1.0f, 1.0f }, \
        { 0.0f, 0.0f, 0.0f, 0.0f }, \
    } \

int main() {
    ss_init("images", 640, 480, 2);
    uint32_t img_tokyo_station, img_shapes;
    ss_load_image_from_file("tokyo-station.png", &img_tokyo_station);
    ss_load_image_from_file("shapes.png", &img_shapes);
    CameraData camera_data = {
        { 0.0f, 0.0f, -500.0f },
        { 0.0f, 0.0f, 0.0f },
        { 640.0f, 480.0f, 1000.0f },
        { 3.1415f / 4.0f, 640.0f / 480.0f, 0.0f, 1000.0f },
    };
    ModelData tokyo_station = DEFAULT_MODEL_DATA;
    tokyo_station.scl.x = 640.0f;
    tokyo_station.scl.y = 480.0f;
    ModelData circle = DEFAULT_MODEL_DATA;
    circle.scl.x = 240.0f;
    circle.scl.y = 240.0f;
    circle.rot.z = -3.1415f / 4.0;
    circle.trs.x = -150.0f;
    circle.trs.y = -50.0f;
    circle.uv.z = 0.5f;
    ModelData rectangle = DEFAULT_MODEL_DATA;
    rectangle.scl.x = 320.0f;
    rectangle.scl.y = 320.0f;
    rectangle.trs.x = 150.0f;
    rectangle.trs.y = 50.0f;
    rectangle.uv.x = 0.5f;
    while (1) {
        if (ss_should_close()) break;
        RenderingQuery query[] = {
            {.kind=RENDERING_QUERY_TYPE_CAMERA, .data.camera_data=&camera_data },
            {.kind=RENDERING_QUERY_TYPE_IMAGE_TEXTURE, .data.image_texture_id=img_tokyo_station},
            {.kind=RENDERING_QUERY_TYPE_MODEL, .data.model_data=&tokyo_station},
            {.kind=RENDERING_QUERY_TYPE_IMAGE_TEXTURE, .data.image_texture_id=img_shapes},
            {.kind=RENDERING_QUERY_TYPE_MODEL, .data.model_data=&circle},
            {.kind=RENDERING_QUERY_TYPE_MODEL, .data.model_data=&rectangle},
        };
        ss_render(0.2f, 0.2f, 0.2f, query, 6);
    }
    ss_terminate();
    return 0;
}
