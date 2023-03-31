#include "private.h"

extern VulkanApp app;

warn_t skd_prepare_rendering(uint32_t *p_id) {
    // get aquire image index
    uint32_t next_image_idx;
    WARN(
        vkAcquireNextImageKHR(
            app.core.device,
            app.rendering.swapchain,
            UINT64_MAX,
            app.framedata.present_semaphore,
            VK_NULL_HANDLE,
            &next_image_idx
        ),
        "failed to get next image index."
    );
    // wait for a fence
    const VkFence fence = app.framedata.fence;
    if (vkWaitForFences(app.core.device, 1, &fence, VK_TRUE, UINT64_MAX) != 0) warning("failed to wait for fence.");
    // reset fences
    if (vkResetFences(app.core.device, 1, &fence) != 0) warning("failed to reset fence.");
    // TODO: reset command buffer?
    // finish
    *p_id = next_image_idx;
    return SUCCESS;
}

warn_t skd_begin_render(uint32_t id, float r, float g, float b) {
    // begin command buffer
    const VkCommandBuffer command = app.framedata.command_buffer;
    const VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        0,
        NULL,
    };
    WARN(vkBeginCommandBuffer(command, &command_buffer_begin_info), "failed to begin record commands to render.");
    // begin render pass
    const VkClearValue clear_value = {{{ r, g, b, 0.0f }}};
    const VkExtent2D extent = { app.rendering.width, app.rendering.height };
    const VkRenderPassBeginInfo render_pass_begin_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        NULL,
        app.pipeline.render_pass,
        app.pipeline.framebuffers[id],
        { {0, 0}, extent },
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
        &app.resource.descriptor_sets[0],
        0,
        NULL
    );
    // bind square
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command, 0, 1, &app.resource.square.vertex_buffer, &offset);
    vkCmdBindIndexBuffer(command, app.resource.square.index_buffer, offset, VK_INDEX_TYPE_UINT32);
    // finish
    return SUCCESS;
}

warn_t skd_end_render(uint32_t id) {
    const VkCommandBuffer command = app.framedata.command_buffer;
    const VkFence fence = app.framedata.fence;
    // end
    vkCmdEndRenderPass(command);
    vkEndCommandBuffer(command);
    // submit
    const VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        NULL,
        1,
        &app.framedata.present_semaphore,
        &wait_stage_mask,
        1,
        &command,
        1,
        &app.framedata.render_semaphore,
    };
    WARN(vkQueueSubmit(app.rendering.queue, 1, &submit_info, fence), "failed to submit queue to render.");
    // present
    VkResult res;
    VkPresentInfoKHR present_info = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        NULL,
        1,
        &app.framedata.render_semaphore,
        1,
        &app.rendering.swapchain,
        &id,
        &res,
    };
    WARN(vkQueuePresentKHR(app.rendering.queue, &present_info), "failed to enqueue present command.");
    // finish
    return SUCCESS;
}

void skd_draw(ModelData *data) {
    const VkCommandBuffer command = app.framedata.command_buffer;
    if (data != NULL) {
        vkCmdPushConstants(command, app.pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelData), data);
    }
    vkCmdDrawIndexed(command, app.resource.square.index_cnt, 1, 0, 0, 0);
}
