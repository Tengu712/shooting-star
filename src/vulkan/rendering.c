#include "private.h"

static uint32_t pre_image_idx = 0;
static uint32_t cur_image_idx = 0;

extern VulkanApp app;

warn_t prepare_rendering(void) {
    // get aquire image index
    WARN(
        vkAcquireNextImageKHR(
            app.core.device,
            app.rendering.swapchain,
            UINT64_MAX,
            app.frame_datas[pre_image_idx].semaphore,
            VK_NULL_HANDLE,
            &cur_image_idx
        ),
        "failed to get next image index."
    );
    // wait for a fence and reset it
    if (vkWaitForFences(app.core.device, 1, &app.frame_datas[cur_image_idx].fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) ss_warning("failed to wait for fence.");
    if (vkResetFences(app.core.device, 1, &app.frame_datas[cur_image_idx].fence) != VK_SUCCESS) ss_warning("failed to reset fence.");
    // reset a command buffer
    if (vkResetCommandBuffer(
            app.frame_datas[cur_image_idx].command_buffer,
            VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
        ) != VK_SUCCESS)
    {
        ss_warning("failed to reset command buffer.");
    }
    // finish
    return SS_SUCCESS;
}

warn_t begin_render(float r, float g, float b) {
    // begin command buffer
    const VkCommandBuffer command = app.frame_datas[cur_image_idx].command_buffer;
    const VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        0,
        NULL,
    };
    WARN(vkBeginCommandBuffer(command, &command_buffer_begin_info), "failed to begin record commands to render.");
    // begin render pass
    const VkClearValue clear_value = {{{ r, g, b, 0.0f }}};
    const VkRenderPassBeginInfo render_pass_begin_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        NULL,
        app.pipeline.render_pass,
        app.pipeline.framebuffers[cur_image_idx],
        { {0, 0}, app.rendering.surface_size },
        1,
        &clear_value,
    };
    vkCmdBeginRenderPass(command, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    // bind pipeline
    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, app.pipeline.pipeline);
    // bind descriptor
    vkCmdBindDescriptorSets(
        command,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        app.pipeline.pipeline_layout,
        0,
        1,
        &app.pipeline.descriptor_sets[0],
        0,
        NULL
    );
    // bind square
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command, 0, 1, &app.resource.square.vertex_buffer, &offset);
    vkCmdBindIndexBuffer(command, app.resource.square.index_buffer, offset, VK_INDEX_TYPE_UINT32);
    // finish
    return SS_SUCCESS;
}

warn_t end_render(void) {
    const VkCommandBuffer command = app.frame_datas[cur_image_idx].command_buffer;
    // end
    vkCmdEndRenderPass(command);
    vkEndCommandBuffer(command);
    // submit
    const VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        NULL,
        1,
        &app.frame_datas[pre_image_idx].semaphore,
        &wait_stage_mask,
        1,
        &command,
        1,
        &app.frame_datas[cur_image_idx].semaphore,
    };
    WARN(vkQueueSubmit(app.rendering.queue, 1, &submit_info, app.frame_datas[cur_image_idx].fence), "failed to submit queue to render.");
    // present
    VkResult res;
    VkPresentInfoKHR present_info = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        NULL,
        1,
        &app.frame_datas[cur_image_idx].semaphore,
        1,
        &app.rendering.swapchain,
        &cur_image_idx,
        &res,
    };
    WARN(vkQueuePresentKHR(app.rendering.queue, &present_info), "failed to enqueue present command.");
    // finish
    pre_image_idx = cur_image_idx;
    return SS_SUCCESS;
}

void draw(const ModelData *data) {
    const VkCommandBuffer command = app.frame_datas[cur_image_idx].command_buffer;
    if (data != NULL)
        vkCmdPushConstants(command, app.pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelData), data);
    vkCmdDrawIndexed(command, app.resource.square.index_cnt, 1, 0, 0, 0);
}

warn_t update_camera(const CameraData *camera_data) {
    if (!map_memory(&app, app.resource.camera.buffer_memory, (void *)camera_data, sizeof(CameraData)))
        return ss_warning("failed to map camera data.");
    else
        return SS_SUCCESS;
}

warn_t use_image_texture(uint32_t id) {
    if (id >= app.resource.max_image_texture_cnt)
        return ss_warning("tried to use image texture out of range.");
    if (app.resource.image_textures[id].view == NULL || app.pipeline.descriptor_sets[id] == NULL)
        return ss_warning("tried to use null image texture.");
    vkCmdBindDescriptorSets(
        app.frame_datas[cur_image_idx].command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        app.pipeline.pipeline_layout,
        0,
        1,
        &app.pipeline.descriptor_sets[id],
        0,
        NULL
    );
    return SS_SUCCESS;
}
