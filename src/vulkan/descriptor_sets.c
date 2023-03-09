#include "private.h"

extern VulkanApp app;

vkres_t skd_update_camera(CameraData *cameradata) {
    if (!map_memory(
            &app,
            app.resource.camera.buffer_memory,
            (void *)cameradata,
            sizeof(CameraData)))
    {
        return EMSG_UPDATE_CAMERA;
    }
    return EMSG_VULKAN_SUCCESS;
}

vkres_t skd_use_image_texture(uint32_t id) {
    if (id >= app.resource.max_image_texture_num) {
        return EMSG_USE_IMAGE_TEXTURE_OUT_OF_INDEX;
    }
    if (app.resource.image_textures[id].view == NULL
        || app.resource.descriptor_sets[id] == NULL)
    {
        return EMSG_USE_NULL_IMAGE_TEXTURE;
    }
    vkCmdBindDescriptorSets(
        app.framedata.command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        app.pipeline_layout,
        0,
        1,
        &app.resource.descriptor_sets[id],
        0,
        NULL
    );
    return EMSG_VULKAN_SUCCESS;
}
