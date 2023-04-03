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
    ss_init_logger();
    ss_init("rectangles", 640, 480, 0);
    CameraData camera_data = {
        { 0.0f, 0.0f, -500.0f },
        { 0.0f, 0.0f, 0.0f },
        { 640.0f, 480.0f, 1000.0f },
        { 3.1415f / 4.0f, 640.0f / 480.0f, 0.0f, 1000.0f },
    };
    ModelData model_data1 = DEFAULT_MODEL_DATA;
    model_data1.scl.x = 240.0f;
    model_data1.scl.y = 200.0f;
    model_data1.trs.x = -160.0f;
    model_data1.trs.y = -80.0f;
    ModelData model_data2 = DEFAULT_MODEL_DATA;
    model_data2.scl.x = 200.0f;
    model_data2.scl.y = 200.0f;
    model_data2.trs.x = 200.0f;
    model_data2.trs.y = 100.0f;
    model_data2.param.x = 1.0f;
    while (1) {
        if (ss_should_close()) break;
        model_data1.rot.y += 0.02f;
        model_data1.rot.z -= 0.01f;
        model_data2.rot.x += 0.01f;
        RenderingQuery query[3];
        query[0].kind = RENDERING_QUERY_TYPE_CAMERA;
        query[0].data.camera_data = &camera_data;
        query[1].kind = RENDERING_QUERY_TYPE_MODEL;
        query[1].data.model_data = &model_data1;
        query[2].kind = RENDERING_QUERY_TYPE_MODEL;
        query[2].data.model_data = &model_data2;
        ss_render(0.2f, 0.2f, 0.2f, query, 3);
    }
    ss_terminate();
    return 0;
}
