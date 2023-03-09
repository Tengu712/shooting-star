#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vulkan_private.h"

#include <stdio.h>

extern VulkanApp app;

vkres_t load_image_texture(
    const unsigned char *pixels,
    int width,
    int height,
    int id
) {
    VkResult res;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const int size = width * height * 4;
    Image *out = &app.resource.image_textures[id];
    // staging buffer
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    if (!create_buffer(
            &app,
            (VkDeviceSize)size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &staging_buffer,
            &staging_buffer_memory))
    {
        return EMSG_LOAD_IMAGE;
    }
    if (!map_memory(&app, staging_buffer_memory, (void *)pixels, size)) {
        return EMSG_LOAD_IMAGE;
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
    res = vkCreateImage(app.device, &image_create_info, NULL, &out->image);
    CHECK(EMSG_LOAD_IMAGE);
    VkMemoryRequirements reqs;
    vkGetImageMemoryRequirements(app.device, out->image, &reqs);
    VkMemoryAllocateInfo allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        reqs.size,
        0,
    };
    allocate_info.memoryTypeIndex = get_memory_type_index(
        &app,
        reqs,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    res = vkAllocateMemory(app.device, &allocate_info, NULL, &out->memory);
    CHECK(EMSG_LOAD_IMAGE);
    res = vkBindImageMemory(app.device, out->image, out->memory, 0);
    CHECK(EMSG_LOAD_IMAGE);
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
    res = vkAllocateCommandBuffers(
        app.device,
        &command_buffer_allocate_info,
        &command
    );
    CHECK(EMSG_LOAD_IMAGE);
    VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        0,
        NULL,
    };
    res = vkBeginCommandBuffer(command, &command_buffer_begin_info);
    CHECK(EMSG_LOAD_IMAGE);
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
    vkCmdCopyBufferToImage(
        command,
        staging_buffer,
        out->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &copy_region
    );
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
    res = vkQueueSubmit(app.queue, 1, &submit_info, VK_NULL_HANDLE);
    CHECK(EMSG_LOAD_IMAGE);
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
    res = vkCreateImageView(app.device, &image_view_create_info, NULL, &out->view);
    CHECK(EMSG_LOAD_IMAGE);
    // finish
    res = vkDeviceWaitIdle(app.device);
    CHECK(EMSG_LOAD_IMAGE);
    vkFreeCommandBuffers(app.device, app.command_pool, 1, &command);
    vkFreeMemory(app.device, staging_buffer_memory, NULL);
    vkDestroyBuffer(app.device, staging_buffer, NULL);
    return EMSG_VULKAN_SUCCESS;
}

vkres_t skd_load_image_from_memory(
    const unsigned char *pixels,
    int width,
    int height,
    unsigned int *out_id
) {
    if (out_id == NULL) {
        return EMSG_NULL_OUT_IMAGE_TEXTURE_ID;
    }
    int id;
    for (id = 0; id < app.resource.max_image_texture_num; ++id) {
        if (app.resource.image_textures[id].image == NULL) {
            break;
        }
    }
    if (id >= app.resource.max_image_texture_num) {
        return EMSG_IMAGE_OUT_OF_INDEX;
    }
    // load image
    const vkres_t res = load_image_texture(pixels, width, height, id);
    if (res != EMSG_VULKAN_SUCCESS) {
        return res;
    }
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
    vkUpdateDescriptorSets(
        app.device,
        1,
        &write_descriptor_set,
        0,
        NULL
    );
    // finish
    *out_id = id;
    return EMSG_VULKAN_SUCCESS;
}

vkres_t skd_load_image_from_file(const char *path, unsigned int *out_id) {
    int width = 0;
    int height = 0;
    int channel_cnt = 0;
    unsigned char *pixels = stbi_load(path, &width, &height, &channel_cnt, 0);
    if (pixels == NULL) {
        return EMSG_LOAD_IMAGE_FILE;
    }
    if (channel_cnt != 4) {
        return EMSG_INVALID_IMAGE_FORMAT;
    }
    vkres_t res = skd_load_image_from_memory(pixels, width, height, out_id);
    stbi_image_free((void *)pixels);
    return res;
}

void skd_unload_image(unsigned int id) {
    if (id >= app.resource.max_image_texture_num
        || app.resource.image_textures[id].view == NULL)
    {
        return;
    }
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
    vkUpdateDescriptorSets(
        app.device,
        1,
        &write_descriptor_set,
        0,
        NULL
    );
    // unload
    vkDestroyImageView(app.device, app.resource.image_textures[id].view, NULL);
    vkDestroyImage(app.device, app.resource.image_textures[id].image, NULL);
    vkFreeMemory(app.device, app.resource.image_textures[id].memory, NULL);
    app.resource.image_textures[id].view = NULL;
    app.resource.image_textures[id].image = NULL;
    app.resource.image_textures[id].memory = NULL;
}
