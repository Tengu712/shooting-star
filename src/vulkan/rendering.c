#include "private.h"

extern VulkanApp app;

vkres_t skd_prepare_rendering(uint32_t *p_id) {
    // get aquire image index
    uint32_t next_image_idx;
    CHECK(
        EMSG_ACQUIRE_NEXT_IMAGE,
        vkAcquireNextImageKHR(
            app.device,
            app.swapchain,
            UINT64_MAX,
            app.framedata.present_semaphore,
            VK_NULL_HANDLE,
            &next_image_idx
        )
    );
    // wait for a fence
    const VkFence fence = app.framedata.fence;
    CHECK(EMSG_WAIT_FOR_FENCE, vkWaitForFences(app.device, 1, &fence, VK_TRUE, UINT64_MAX));
    // reset fences
    CHECK(EMSG_RESET_FENCE, vkResetFences(app.device, 1, &fence));
    // TODO: reset command buffer?
    // finish
    *p_id = next_image_idx;
    return EMSG_VULKAN_SUCCESS;
}

vkres_t skd_begin_render(uint32_t id, float r, float g, float b) {
    // begin command buffer
    const VkCommandBuffer command = app.framedata.command_buffer;
    const VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        0,
        NULL,
    };
    CHECK(EMSG_BEGIN_COMMAND_BUFFER, vkBeginCommandBuffer(command, &command_buffer_begin_info));
    // begin render pass
    const VkClearValue clear_value = {{{ r, g, b, 0.0f }}};
    const VkExtent2D extent = { app.width, app.height };
    const VkRenderPassBeginInfo render_pass_begin_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        NULL,
        app.render_pass,
        app.framebuffers[id],
        { {0, 0}, extent },
        1,
        &clear_value,
    };
    vkCmdBeginRenderPass(command, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    // bind pipeline
    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, app.pipeline);
    // bind descriptor
    vkCmdBindDescriptorSets(
        command,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        app.pipeline_layout,
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
    return EMSG_VULKAN_SUCCESS;
}

vkres_t skd_end_render(uint32_t id) {
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
    CHECK(EMSG_SUBMIT_QUEUE, vkQueueSubmit(app.queue, 1, &submit_info, fence));
    // present
    VkResult res;
    VkPresentInfoKHR present_info = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        NULL,
        1,
        &app.framedata.render_semaphore,
        1,
        &app.swapchain,
        &id,
        &res,
    };
    CHECK(EMSG_PRESENT_QUEUE, vkQueuePresentKHR(app.queue, &present_info));
    // finish
    return EMSG_VULKAN_SUCCESS;
}

void skd_draw(ModelData *data) {
    const VkCommandBuffer command = app.framedata.command_buffer;
    if (data != NULL) {
        vkCmdPushConstants(command, app.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelData), data);
    }
    vkCmdDrawIndexed(command, app.resource.square.index_cnt, 1, 0, 0, 0);
}
