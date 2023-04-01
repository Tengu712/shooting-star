#include "private.h"

extern VulkanApp app;

warn_t update_camera(CameraData *cameradata) {
    if (!map_memory(&app, app.resource.camera.buffer_memory, (void *)cameradata, sizeof(CameraData)))
        return warning("failed to map camera data.");
    else
        return SUCCESS;
}

warn_t use_image_texture(uint32_t id) {
    if (id >= app.resource.max_image_texture_num)
        return warning("tried to use image texture out of range.");
    if (app.resource.image_textures[id].view == NULL || app.resource.descriptor_sets[id] == NULL)
        return warning("tried to use null image texture.");
    vkCmdBindDescriptorSets(
        app.framedata.command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        app.pipeline.pipeline_layout,
        0,
        1,
        &app.resource.descriptor_sets[id],
        0,
        NULL
    );
    return SUCCESS;
}
