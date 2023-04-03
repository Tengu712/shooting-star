#include "sstar.h"

#define DEFAULT_MODEL_DATA { \
        { 0.0f, 0.0f, 0.0f, 0.0f }, \
        { 0.0f, 0.0f, 0.0f, 0.0f }, \
        { 1.0f, 1.0f, 1.0f, 1.0f }, \
        { 1.0f, 1.0f, 1.0f, 1.0f }, \
        { 0.0f, 0.0f, 1.0f, 1.0f }, \
        { 0.0f, 0.0f, 0.0f, 0.0f }, \
    }; \

int main() {
    ss_init_logger();
    ss_init("rectangles", 640, 480, 640.0f, 480.0f, 0);
    while (1) {
        if (ss_should_close()) break;
        ModelData model_data1 = DEFAULT_MODEL_DATA;
        model_data1.trs.x = -0.5f;
        model_data1.trs.y = -0.5f;
        model_data1.scl.x = 0.5f;
        model_data1.scl.y = 0.25f;
        ModelData model_data2 = DEFAULT_MODEL_DATA;
        model_data2.trs.x = 0.5f;
        model_data2.trs.y = 0.5f;
        model_data2.scl.x = 0.25f;
        model_data2.scl.y = 0.5f;
        RenderingQuery query[] = {
            { RENDERING_QUERY_TYPE_MODEL, &model_data1 },
            { RENDERING_QUERY_TYPE_MODEL, &model_data2 },
        };
        ss_render(0.2f, 0.2f, 0.2f, query, 2);
    }
    ss_terminate();
    return 0;
}