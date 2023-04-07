#include "private.h"

extern VulkanApp app;

static void update_descriptor_sets(uint32_t desc_set_id, int32_t img_tex_id) {
    const VkDescriptorImageInfo sampler_descriptor_image_info = {
        app.pipeline.sampler,
        app.resource.image_textures[img_tex_id].view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    const VkWriteDescriptorSet write_descriptor_set = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        NULL,
        app.pipeline.descriptor_sets[desc_set_id],
        1,
        0,
        1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        &sampler_descriptor_image_info,
        NULL,
        NULL,
    };
    vkUpdateDescriptorSets(app.core.device, 1, &write_descriptor_set, 0, NULL);
}

warn_t load_image_texture(const unsigned char *pixels, int32_t width, int32_t height, uint32_t id) {
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const int32_t size = width * height * 4;
    Image *out = &app.resource.image_textures[id];
    // staging buffer
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    if (create_buffer(
            &app,
            (VkDeviceSize)size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &staging_buffer,
            &staging_buffer_memory) != SS_SUCCESS)
    {
        return ss_warning("failed to create buffer for image texture.");
    }
    if (!map_memory(&app, staging_buffer_memory, (void *)pixels, size)) {
        return ss_warning("failed to map image texture.");
    }
    // image
    const VkImageCreateInfo image_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        NULL,
        0,
        VK_IMAGE_TYPE_2D,
        format,
        { width, height, 1 },
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL,
        VK_IMAGE_LAYOUT_UNDEFINED,
    };
    WARN(vkCreateImage(app.core.device, &image_create_info, NULL, &out->image), "failed to create image for image texture.");
    VkMemoryRequirements reqs;
    vkGetImageMemoryRequirements(app.core.device, out->image, &reqs);
    VkMemoryAllocateInfo allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        reqs.size,
        0,
    };
    allocate_info.memoryTypeIndex = get_memory_type_index(&app, reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    WARN(vkAllocateMemory(app.core.device, &allocate_info, NULL, &out->memory), "failed to allocate memory for image texture.");
    WARN(vkBindImageMemory(app.core.device, out->image, out->memory, 0), "failed to bind image memory for image texture.");
    // begin copy command
    const VkBufferImageCopy copy_region = {
        0,
        0,
        0,
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        { 0, 0, 0 },
        { width, height, 1 },
    };
    const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        NULL,
        app.rendering.command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1,
    };
    VkCommandBuffer command;
    WARN(vkAllocateCommandBuffers(app.core.device, &command_buffer_allocate_info, &command), "failed to allocate command buffers to create image texture.");
    const VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        0,
        NULL,
    };
    WARN(vkBeginCommandBuffer(command, &command_buffer_begin_info), "failed to begin to record commands to create image texture.");
    // copy buffer to image
    VkImageMemoryBarrier image_memory_barrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        NULL,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        out->image,
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
    };
    vkCmdPipelineBarrier(
        command,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &image_memory_barrier
    );
    vkCmdCopyBufferToImage(command, staging_buffer, out->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkCmdPipelineBarrier(
        command,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &image_memory_barrier
    );
    // end command
    vkEndCommandBuffer(command);
    // image view
    const VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        NULL,
        0,
        NULL,
        NULL,
        1,
        &command,
        0,
        NULL,
    };
    WARN(vkQueueSubmit(app.rendering.queue, 1, &submit_info, VK_NULL_HANDLE), "failed to submit queue to create image texture.");
    const VkImageViewCreateInfo image_view_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        NULL,
        0,
        out->image,
        VK_IMAGE_VIEW_TYPE_2D,
        format,
        {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A,
        },
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
    };
    WARN(vkCreateImageView(app.core.device, &image_view_create_info, NULL, &out->view), "failed to create image view for image texture.");
    // finish
    vkDeviceWaitIdle(app.core.device);
    vkFreeCommandBuffers(app.core.device, app.rendering.command_pool, 1, &command);
    vkFreeMemory(app.core.device, staging_buffer_memory, NULL);
    vkDestroyBuffer(app.core.device, staging_buffer, NULL);
    return SS_SUCCESS;
}

warn_t load_image_from_memory(const unsigned char *pixels, int32_t width, int32_t height, uint32_t *out_id) {
    if (out_id == NULL) return ss_warning("tried to output image texture id to null.");
    uint32_t id;
    for (id = 0; id < app.resource.max_image_texture_cnt; ++id) {
        if (app.resource.image_textures[id].image == NULL) break;
    }
    if (id >= app.resource.max_image_texture_cnt) return ss_warning("tried to too many image textures.");
    // load image
    if (load_image_texture(pixels, width, height, id) != SS_SUCCESS) return SS_WARN;
    // register image texture to descriptor set
    update_descriptor_sets(id, id);
    // finish
    *out_id = id;
    return SS_SUCCESS;
}

void unload_image(uint32_t id) {
    if (id >= app.resource.max_image_texture_cnt || app.resource.image_textures[id].view == NULL) return;
    vkDeviceWaitIdle(app.core.device);
    // detach from descriptor set
    update_descriptor_sets(id, 0);
    // unload
    vkDestroyImageView(app.core.device, app.resource.image_textures[id].view, NULL);
    vkDestroyImage(app.core.device, app.resource.image_textures[id].image, NULL);
    vkFreeMemory(app.core.device, app.resource.image_textures[id].memory, NULL);
    app.resource.image_textures[id].view = NULL;
    app.resource.image_textures[id].image = NULL;
    app.resource.image_textures[id].memory = NULL;
}
