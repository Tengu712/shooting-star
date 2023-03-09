#pragma once

#include "../vulkan.h"

#ifdef __linux__
#define VK_USE_PLATFORM_XCB_KHR
#elif _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(p) if (res != 0) return (p);

// A struct for model buffer.
typedef struct Model_t {
    uint32_t index_cnt;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
} Model;

// A struct for camera uniform buffer.
typedef struct Camera_t {
    VkBuffer buffer;
    VkDeviceMemory buffer_memory;
} Camera;

// A struct for image texture.
typedef struct Image_t {
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
} Image;

// TODO: struct
typedef struct VulkanApp_t {
    VkInstance instance;
    VkPhysicalDeviceMemoryProperties phys_device_memory_prop;
    VkDevice device;
    VkSurfaceKHR surface;
    uint32_t width;
    uint32_t height;
    VkRenderPass render_pass;
    VkSwapchainKHR swapchain;
    uint32_t images_cnt;
    VkImageView *image_views;
    VkFramebuffer *framebuffers;
    VkQueue queue;
    VkCommandPool command_pool;
    VkShaderModule vert_shader;
    VkShaderModule frag_shader;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    uint32_t dynamic_alignment;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkSampler sampler;
    struct FrameData_t {
        VkCommandBuffer command_buffer;
        VkFence fence;
        VkSemaphore render_semaphore;
        VkSemaphore present_semaphore;
    } framedata;
    struct Resource_t {
        // it's the same as max_image_texture_num
        unsigned int max_descriptor_set_num;
        VkDescriptorSet *descriptor_sets;
        Camera camera;
        // it's guaranteed to be greater than 1
        unsigned int max_image_texture_num;
        Image *image_textures;
        Model square;
    } resource;
} VulkanApp;

// A function to get a memory type index that's available and bitted at flags.
// It returns -1 if it fails.
inline static int get_memory_type_index(
    VulkanApp *app,
    VkMemoryRequirements reqs,
    VkMemoryPropertyFlags flags
) {
    for (int i = 0; i < app->phys_device_memory_prop.memoryTypeCount; ++i) {
        if ((reqs.memoryTypeBits & (1 << i))
            && (app->phys_device_memory_prop.memoryTypes[i].propertyFlags
                & flags))
        {
            return i;
        }
    }
    return -1;
}

// A function to create a buffer.
// It returns 0 if it fails.
inline static int create_buffer(
    VulkanApp *app,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags flags,
    VkBuffer *p_buffer,
    VkDeviceMemory *p_device_memory
) {
    VkResult res;
    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        0,
        size,
        usage,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
    };
    res = vkCreateBuffer(app->device, &buffer_create_info, NULL, p_buffer);
    CHECK(0);
    VkMemoryRequirements reqs;
    vkGetBufferMemoryRequirements(app->device, *p_buffer, &reqs);
    VkMemoryAllocateInfo allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        reqs.size,
        0,
    };
    allocate_info.memoryTypeIndex = get_memory_type_index(app, reqs, flags);
    res = vkAllocateMemory(
        app->device,
        &allocate_info,
        NULL,
        p_device_memory
    );
    CHECK(0);
    res = vkBindBufferMemory(app->device, *p_buffer, *p_device_memory, 0);
    CHECK(0);
    return 1;
}

// A function to map data into memory.
// It returns 0 if it fails.
inline static int map_memory(
    VulkanApp *app,
    VkDeviceMemory device_memory,
    void *data,
    int size
) {
    void *p;
    VkResult res = vkMapMemory(
        app->device,
        device_memory,
        0,
        VK_WHOLE_SIZE,
        0,
        &p
    );
    CHECK(0);
    memcpy(p, data, size);
    vkUnmapMemory(app->device, device_memory);
    return 1;
}

// A function to load image texture into app.resource.image_textures[id].
// WARN: it overwrites data at `id` and doesn't update descriptor set.
vkres_t load_image_texture(
    const unsigned char *pixels,
    int width,
    int height,
    int id
);
