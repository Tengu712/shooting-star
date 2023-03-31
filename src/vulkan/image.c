#include "private.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern VulkanApp app;

warn_t load_image_texture(const unsigned char *pixels, int32_t width, int32_t height, int32_t id) {
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
            &staging_buffer_memory) != SUCCESS)
    {
        return warning("failed to create buffer for image texture.");
    }
    if (!map_memory(&app, staging_buffer_memory, (void *)pixels, size)) {
        return warning("failed to map image texture.");
    }
    // image
    VkImageCreateInfo image_create_info = {
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
    WARN(vkCreateImage(app.device, &image_create_info, NULL, &out->image), "failed to create image for image texture.");
    VkMemoryRequirements reqs;
    vkGetImageMemoryRequirements(app.device, out->image, &reqs);
    VkMemoryAllocateInfo allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        reqs.size,
        0,
    };
    allocate_info.memoryTypeIndex = get_memory_type_index(&app, reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    WARN(vkAllocateMemory(app.device, &allocate_info, NULL, &out->memory), "failed to allocate memory for image texture.");
    WARN(vkBindImageMemory(app.device, out->image, out->memory, 0), "failed to bind image memory for image texture.");
    // begin copy command
    VkBufferImageCopy copy_region = {
        0,
        0,
        0,
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        { 0, 0, 0 },
        { width, height, 1 },
    };
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        NULL,
        app.command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1,
    };
    VkCommandBuffer command;
    WARN(vkAllocateCommandBuffers(app.device, &command_buffer_allocate_info, &command), "failed to allocate command buffers to create image texture.");
    VkCommandBufferBeginInfo command_buffer_begin_info = {
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
    VkSubmitInfo submit_info = {
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
    WARN(vkQueueSubmit(app.queue, 1, &submit_info, VK_NULL_HANDLE), "failed to submit queue to create image texture.");
    VkImageViewCreateInfo image_view_create_info = {
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
    WARN(vkCreateImageView(app.device, &image_view_create_info, NULL, &out->view), "failed to create image view for image texture.");
    // finish
    vkDeviceWaitIdle(app.device);
    vkFreeCommandBuffers(app.device, app.command_pool, 1, &command);
    vkFreeMemory(app.device, staging_buffer_memory, NULL);
    vkDestroyBuffer(app.device, staging_buffer, NULL);
    return SUCCESS;
}

warn_t skd_load_image_from_memory(const unsigned char *pixels, int32_t width, int32_t height, uint32_t *out_id) {
    if (out_id == NULL) warning("tried to output image texture id to null.");
    int32_t id;
    for (id = 0; id < app.resource.max_image_texture_num; ++id) {
        if (app.resource.image_textures[id].image == NULL) break;
    }
    if (id >= app.resource.max_image_texture_num) warning("tried to too many image textures.");
    // load image
    if (load_image_texture(pixels, width, height, id) != SUCCESS) return WARNING;
    // register image texture to descriptor set
    // HACK: should i move this part into descriptor_sets.c?
    VkDescriptorImageInfo sampler_descriptor_image_info = {
        app.sampler,
        app.resource.image_textures[id].view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet write_descriptor_set = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        NULL,
        app.resource.descriptor_sets[id],
        1,
        0,
        1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        &sampler_descriptor_image_info,
        NULL,
        NULL,
    };
    vkUpdateDescriptorSets(app.device, 1, &write_descriptor_set, 0, NULL);
    // finish
    *out_id = id;
    return SUCCESS;
}

warn_t skd_load_image_from_file(const char *path, uint32_t *out_id) {
    int32_t width = 0;
    int32_t height = 0;
    int32_t channel_cnt = 0;
    unsigned char *pixels = stbi_load(path, &width, &height, &channel_cnt, 0);
    if (pixels == NULL) warning("failed to load image file."); // TODO: log file name
    if (channel_cnt != 4) warning("tried to create image texture from invalid color format image file."); // TODO: log file name
    const warn_t res = skd_load_image_from_memory(pixels, width, height, out_id);
    stbi_image_free((void *)pixels);
    return res;
}

void skd_unload_image(uint32_t id) {
    if (id >= app.resource.max_image_texture_num || app.resource.image_textures[id].view == NULL) return;
    vkDeviceWaitIdle(app.device);
    // detach from descriptor set
    VkDescriptorImageInfo sampler_descriptor_image_info = {
        app.sampler,
        app.resource.image_textures[0].view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet write_descriptor_set = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        NULL,
        app.resource.descriptor_sets[id],
        1,
        0,
        1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        &sampler_descriptor_image_info,
        NULL,
        NULL,
    };
    vkUpdateDescriptorSets(app.device, 1, &write_descriptor_set, 0, NULL);
    // unload
    vkDestroyImageView(app.device, app.resource.image_textures[id].view, NULL);
    vkDestroyImage(app.device, app.resource.image_textures[id].image, NULL);
    vkFreeMemory(app.device, app.resource.image_textures[id].memory, NULL);
    app.resource.image_textures[id].view = NULL;
    app.resource.image_textures[id].image = NULL;
    app.resource.image_textures[id].memory = NULL;
}
