#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vulkan.h"

#ifdef __linux__
#define VK_USE_PLATFORM_XCB_KHR
#endif
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#define CHECK(p) if (res != VK_SUCCESS) return (p);

// ========================================================================= //
//         Structs                                                           //
// ========================================================================= //

typedef struct Vec4_t {
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct Mat4_t {
    float a11;
    float a21;
    float a31;
    float a41;
    float a12;
    float a22;
    float a32;
    float a42;
    float a13;
    float a23;
    float a33;
    float a43;
    float a14;
    float a24;
    float a34;
    float a44;
} Mat4;

typedef struct UniformBufferObject_t {
    Mat4 mat_scl;
    Mat4 mat_rtx;
    Mat4 mat_rty;
    Mat4 mat_rtz;
    Mat4 mat_trs;
    Mat4 mat_view;
    Mat4 mat_proj;
    Vec4 vec_uv;
    Vec4 vec_param;
} UniformBufferObject;

typedef struct Vertex_t {
    float pos_x;
    float pos_y;
    float pos_z;
    float uv_u;
    float uv_v;
} Vertex;

typedef struct Model_t {
    uint32_t index_cnt;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
} Model;

extern char shader_vert_data[];
extern int shader_vert_size;
extern char shader_frag_data[];
extern int shader_frag_size;

// ========================================================================= //
//         Global Variables                                                  //
// ========================================================================= //

VkInstance g_instance;
VkPhysicalDeviceMemoryProperties g_phys_device_memory_prop;
VkDevice g_device;
VkSurfaceKHR g_surface;
uint32_t g_width;
uint32_t g_height;
VkRenderPass g_render_pass;
VkSwapchainKHR g_swapchain;
uint32_t g_images_cnt;
VkImageView *g_image_views;
VkFramebuffer *g_framebuffers;
VkQueue g_queue;
VkCommandPool g_command_pool;
VkCommandBuffer *g_command_buffers;
VkFence *g_fences;
VkSemaphore g_render_semaphore;
VkSemaphore g_present_semaphore;
VkShaderModule g_vert_shader;
VkShaderModule g_frag_shader;
VkBuffer g_uniform_buffer;
VkDeviceMemory g_uniform_buffer_memory;
VkDescriptorSetLayout g_descriptor_set_layout;
VkDescriptorPool g_descriptor_pool;
VkDescriptorSet g_descriptor_set;
VkPipelineLayout g_pipeline_layout;
VkPipeline g_pipeline;
Model g_square;
int g_is_ubo_updated;
UniformBufferObject g_ubo;
VkSampler g_sampler;
Image g_empty_image;

// ========================================================================= //
//         General Functions                                                 //
// ========================================================================= //

#ifdef __linux__
int create_xcb_surface(SkdWindowParam *window_param) {
    const VkXcbSurfaceCreateInfoKHR ci = {
        VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        NULL,
        0,
        window_param->data.xcb_window.connection,
        window_param->data.xcb_window.window,
    };
    VkResult res = vkCreateXcbSurfaceKHR(g_instance, &ci, NULL, &g_surface);
    CHECK(0);
    return 1;
}
#else
int create_xcb_surface(SkdWindowParam *window_param) {
    return 0;
}
#endif
#ifdef _WIN32
int create_win32_surface(SkdWindowParam *window_param) {
    VkWin32SurfaceCreateInfoKHR ci = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        NULL,
        0,
        window_param->data.winapi_window.hinst,
        window_param->data.winapi_window.hwnd,
    };
    VkResult res = vkCreateWin32SurfaceKHR(g_instance, &ci, NULL, &g_surface);
    CHECK(0);
    return 1;
}
#else
int create_win32_surface(SkdWindowParam *window_param) {
    return 0;
}
#endif

int get_memory_type_index(
    VkMemoryRequirements reqs,
    VkMemoryPropertyFlags flags
) {
    for (int i = 0; i < g_phys_device_memory_prop.memoryTypeCount; ++i) {
        if ((reqs.memoryTypeBits & (1 << i))
            && (g_phys_device_memory_prop.memoryTypes[i].propertyFlags
                & flags))
        {
            return i;
        }
    }
    return -1;
}

int create_buffer(
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
    res = vkCreateBuffer(g_device, &buffer_create_info, NULL, p_buffer);
    CHECK(0);
    VkMemoryRequirements reqs;
    vkGetBufferMemoryRequirements(g_device, *p_buffer, &reqs);
    VkMemoryAllocateInfo allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        reqs.size,
        0,
    };
    allocate_info.memoryTypeIndex = get_memory_type_index(reqs, flags);
    res = vkAllocateMemory(
        g_device,
        &allocate_info,
        NULL,
        p_device_memory
    );
    CHECK(0);
    res = vkBindBufferMemory(g_device, *p_buffer, *p_device_memory, 0);
    CHECK(0);
    return 1;
}

int map_memory(VkDeviceMemory device_memory, void *data, int size) {
    void *p;
    VkResult res = vkMapMemory(
        g_device,
        device_memory,
        0,
        VK_WHOLE_SIZE,
        0,
        &p
    );
    CHECK(0);
    memcpy(p, data, size);
    vkUnmapMemory(g_device, device_memory);
    return 1;
}

// ========================================================================= //
//         Initialization and Termination                                    //
// ========================================================================= //

int skd_init_vulkan(SkdWindowParam *window_param) {
    VkResult res;

    // instance
    int inst_ext_props_cnt = 0;
    res = vkEnumerateInstanceExtensionProperties(
        NULL,
        &inst_ext_props_cnt,
        NULL
    );
    CHECK(EMSG_ENUM_INST_EXT_PROPS);
    VkExtensionProperties *inst_ext_props =
        (VkExtensionProperties*)
        malloc(sizeof(VkExtensionProperties) * inst_ext_props_cnt);
    res = vkEnumerateInstanceExtensionProperties(
        NULL,
        &inst_ext_props_cnt,
        inst_ext_props
    );
    CHECK(EMSG_ENUM_INST_EXT_PROPS);
    const char **inst_exts =
        (const char **)malloc(sizeof(char *) * inst_ext_props_cnt);
    const int inst_exts_cnt = inst_ext_props_cnt;
    for (int i = 0; i < inst_ext_props_cnt; ++i) {
        inst_exts[i] = inst_ext_props[i].extensionName;
    }
    const char *inst_layer_names[1] = {
        "VK_LAYER_KHRONOS_validation\0",
    };
    const int inst_layer_names_cnt = 1;
    const VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        NULL,
        "VulkanApplication\0",
        0,
        "VulkanApplication\0",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_1,
    };
    const VkInstanceCreateInfo create_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        NULL,
        0,
        &app_info,
        inst_layer_names_cnt,
        inst_layer_names,
        inst_exts_cnt,
        inst_exts,
    };
    res = vkCreateInstance(&create_info, NULL, &g_instance);
    CHECK(EMSG_CREATE_INST);
    free((char **)inst_exts);
    free(inst_ext_props);

    // physical device
    int phys_devices_cnt = 0;
    res = vkEnumeratePhysicalDevices(g_instance, &phys_devices_cnt, NULL);
    CHECK(EMSG_ENUM_PHYS_DEVICES);
    VkPhysicalDevice *phys_devices =
        (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * phys_devices_cnt);
    res = vkEnumeratePhysicalDevices(g_instance, &phys_devices_cnt, phys_devices);
    CHECK(EMSG_ENUM_PHYS_DEVICES);
    const VkPhysicalDevice phys_device = phys_devices[0];
    vkGetPhysicalDeviceMemoryProperties(
        phys_device,
        &g_phys_device_memory_prop
    );
    free(phys_devices);

    // queue family index
    int queue_family_props_cnt = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        phys_device,
        &queue_family_props_cnt,
        NULL
    );
    VkQueueFamilyProperties *queue_family_props =
        (VkQueueFamilyProperties *)
        malloc(sizeof(VkQueueFamilyProperties) * queue_family_props_cnt);
    vkGetPhysicalDeviceQueueFamilyProperties(
        phys_device,
        &queue_family_props_cnt,
        queue_family_props
    );
    int queue_family_index = -1;
    for (int i = 0; i < queue_family_props_cnt; ++i) {
        if ((queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0) {
            queue_family_index = i;
            break;
        }
    }
    if (queue_family_index == -1) {
        return EMSG_FIND_QUEUE_FAMILY_INDEX;
    }
    free(queue_family_props);

    // device
    int device_ext_props_cnt = 0;
    res = vkEnumerateDeviceExtensionProperties(
        phys_device,
        NULL,
        &device_ext_props_cnt,
        NULL
    );
    CHECK(EMSG_ENUM_DEVICE_EXT_PROPS);
    VkExtensionProperties *device_ext_props =
        (VkExtensionProperties*)
        malloc(sizeof(VkExtensionProperties) * device_ext_props_cnt);
    res = vkEnumerateDeviceExtensionProperties(
        phys_device,
        NULL,
        &device_ext_props_cnt,
        device_ext_props
    );
    CHECK(EMSG_ENUM_DEVICE_EXT_PROPS);
    const char **device_exts =
        (const char**)malloc(sizeof(char*) * device_ext_props_cnt);
    int device_exts_cnt = 0;
    for (int i = 0; i < device_ext_props_cnt; ++i) {
        const int cmpres = strcmp(
            device_ext_props[i].extensionName,
            "VK_EXT_buffer_device_address"
        );
        if (cmpres == 0) continue;
        device_exts[device_exts_cnt] = device_ext_props[i].extensionName;
        device_exts_cnt += 1;
    }
    const float default_queue_priority = 1.0;
    const VkDeviceQueueCreateInfo queue_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        NULL,
        0,
        queue_family_index,
        1,
        &default_queue_priority,
    };
    const VkDeviceCreateInfo device_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        NULL,
        0,
        1,
        &queue_create_info,
        0,
        NULL,
        device_exts_cnt,
        device_exts,
        NULL,
    };
    res = vkCreateDevice(phys_device, &device_create_info, NULL, &g_device);
    CHECK(EMSG_CREATE_DEVICE);
    free((char**)device_exts);
    free(device_ext_props);

    // surface
    switch (window_param->kind) {
        case SKD_WIN_KIND_XCB:
            if (create_xcb_surface(window_param) != 1) {
                return EMSG_CREATE_SURFACE;
            }
            break;
        case SKD_WIN_KIND_WINAPI:
            if (create_win32_surface(window_param) != 1) {
                return EMSG_CREATE_SURFACE;
            }
            break;
        default:
            return EMSG_CREATE_SURFACE;
    }
    uint32_t surface_formats_cnt = 0;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(
        phys_device,
        g_surface,
        &surface_formats_cnt,
        NULL
    );
    CHECK(EMSG_GET_SURFACE_FORMATS);
    VkSurfaceFormatKHR *surface_formats = 
        (VkSurfaceFormatKHR *)
        malloc(sizeof(VkSurfaceFormatKHR) * surface_formats_cnt);
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(
        phys_device,
        g_surface,
        &surface_formats_cnt,
        surface_formats
    );
    CHECK(EMSG_GET_SURFACE_FORMATS);
    int surface_format_index = -1;
    for (int i = 0; i < surface_formats_cnt; ++i) {
        if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
            surface_format_index = i;
            break;
        }
    }
    if (surface_format_index == -1) {
        return EMSG_GET_SURFACE_FORMATS;
    }
    const VkSurfaceFormatKHR surface_format =
        surface_formats[surface_format_index];
    VkSurfaceCapabilitiesKHR surface_capabilities;
    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        phys_device,
        g_surface,
        &surface_capabilities
    );
    CHECK(EMSG_GET_SURFACE_CAPABILITIES);
    g_width = surface_capabilities.currentExtent.width;
    g_height = surface_capabilities.currentExtent.height;
    free(surface_formats);

    // render pass
    const VkAttachmentDescription attachment_desc = {
        0,
        surface_format.format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    const VkAttachmentReference attachment_ref = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    const VkSubpassDescription subpass_desc = {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        NULL,
        1,
        &attachment_ref,
        NULL,
        NULL,
        0,
        NULL,
    };
    const VkRenderPassCreateInfo render_pass_create_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        NULL,
        0,
        1,
        &attachment_desc,
        1,
        &subpass_desc,
        0,
        NULL,
    };
    res = vkCreateRenderPass(
        g_device,
        &render_pass_create_info,
        NULL,
        &g_render_pass
    );
    CHECK(EMSG_CREATE_RENDER_PASS);

    // swapchain
    const uint32_t min_image_count =
        surface_capabilities.minImageCount > 2 ?
            surface_capabilities.minImageCount : 2;
    const VkSwapchainCreateInfoKHR swapchain_create_info = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        NULL,
        0,
        g_surface,
        min_image_count,
        surface_format.format,
        surface_format.colorSpace,
        surface_capabilities.currentExtent,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL,
        surface_capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_PRESENT_MODE_FIFO_KHR,
        VK_TRUE,
        VK_NULL_HANDLE
    };
    res = vkCreateSwapchainKHR(
        g_device,
        &swapchain_create_info,
        NULL,
        &g_swapchain
    );
    CHECK(EMSG_CREATE_SWAPCHAIN);

    // image views
    res = vkGetSwapchainImagesKHR(g_device, g_swapchain, &g_images_cnt, NULL);
    CHECK(EMSG_GET_IMAGES);
    VkImage *images = (VkImage *)malloc(sizeof(VkImage) * g_images_cnt);
    res = vkGetSwapchainImagesKHR(g_device, g_swapchain, &g_images_cnt, images);
    CHECK(EMSG_GET_IMAGES);
    g_image_views = (VkImageView *)malloc(sizeof(VkImageView) * g_images_cnt);
    for (int i = 0; i < g_images_cnt; ++i) {
        VkImageViewCreateInfo image_view_create_info = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            NULL,
            0,
            images[i],
            VK_IMAGE_VIEW_TYPE_2D,
            surface_format.format,
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A,
            },
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1,
            }
        };
        res = vkCreateImageView(
            g_device,
            &image_view_create_info,
            NULL,
            &g_image_views[i]
        );
        CHECK(EMSG_CREATE_IMAGE_VIEW);
    }
    free(images);

    // framebuffers
    VkFramebufferCreateInfo frame_buffer_create_info = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        NULL,
        0,
        g_render_pass,
        1,
        NULL,
        g_width,
        g_height,
        1,
    };
    g_framebuffers =
        (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * g_images_cnt);
    for (int i = 0; i < g_images_cnt; ++i) {
        frame_buffer_create_info.pAttachments = &g_image_views[i];
        res = vkCreateFramebuffer(
            g_device,
            &frame_buffer_create_info,
            NULL,
            &g_framebuffers[i]
        );
        CHECK(EMSG_CREATE_FRAMEBUFFER);
    }

    // queue
    vkGetDeviceQueue(g_device, queue_family_index, 0, &g_queue);

    // command pool
    const VkCommandPoolCreateInfo command_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        NULL,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        queue_family_index,
    };
    res = vkCreateCommandPool(
        g_device,
        &command_pool_create_info,
        NULL,
        &g_command_pool
    );
    CHECK(EMSG_CREATE_COMMAND_POOL);

    // command buffers
    const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        NULL,
        g_command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        g_images_cnt,
    };
    g_command_buffers =
        (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * g_images_cnt);
    res = vkAllocateCommandBuffers(
        g_device,
        &command_buffer_allocate_info,
        g_command_buffers
    );
    CHECK(EMSG_ALLOCATE_COMMAND_BUFFERS);

    // fences
    const VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        NULL,
        VK_FENCE_CREATE_SIGNALED_BIT,
    };
    g_fences = (VkFence *)malloc(sizeof(VkFence) * g_images_cnt);
    for (int i = 0; i < g_images_cnt; ++i) {
        res = vkCreateFence(g_device, &fence_create_info, NULL, &g_fences[i]);
        CHECK(EMSG_CREATE_FENCE);
    }

    // semaphores
    const VkSemaphoreCreateInfo semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        NULL,
        0,
    };
    res = vkCreateSemaphore(
        g_device,
        &semaphore_create_info,
        NULL,
        &g_render_semaphore
    );
    CHECK(EMSG_CREATE_SEMAPHORE);
    res = vkCreateSemaphore(
        g_device,
        &semaphore_create_info,
        NULL,
        &g_present_semaphore
    );
    CHECK(EMSG_CREATE_SEMAPHORE);

    // shaders
    VkShaderModuleCreateInfo shader_module_create_info = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        NULL,
        0,
        shader_vert_size,
        (const uint32_t *)shader_vert_data,
    };
    res = vkCreateShaderModule(
        g_device,
        &shader_module_create_info,
        NULL,
        &g_vert_shader
    );
    CHECK(EMSG_CREATE_SHADER);
    shader_module_create_info.codeSize = shader_frag_size;
    shader_module_create_info.pCode = (const uint32_t *)shader_frag_data;
    res = vkCreateShaderModule(
        g_device,
        &shader_module_create_info,
        NULL,
        &g_frag_shader
    );
    CHECK(EMSG_CREATE_SHADER);

    // uniform buffer
    if (!create_buffer(
            sizeof(UniformBufferObject),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &g_uniform_buffer,
            &g_uniform_buffer_memory))
    {
        return EMSG_CREATE_BUFFER;
    }

    // sampler
    VkSamplerCreateInfo sampler_create_info = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        NULL,
        0,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_NEAREST,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        0.0,
        0,
        1.0,
        0,
        VK_COMPARE_OP_NEVER,
        0.0,
        0.0,
        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        0,
    };
    res = vkCreateSampler(g_device, &sampler_create_info, NULL, &g_sampler);
    CHECK(EMSG_CREATE_SAMPLER);
    const unsigned char pixels[] = { 0, 0, 0, 0 };
    if (skd_load_image_from_memory(pixels, 1, 1, &g_empty_image) != EMSG_SUCCESS) {
        return EMSG_CREATE_SAMPLER;
    }

    // descriptor
    VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[] = {
        {
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            NULL,
        },
        {
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            NULL,
        },
    };
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        NULL,
        0,
        2,
        descriptor_set_layout_bindings,
    };
    res = vkCreateDescriptorSetLayout(
        g_device,
        &descriptor_set_layout_create_info,
        NULL,
        &g_descriptor_set_layout
    );
    CHECK(EMSG_CREATE_DESCRIPTOR);
    VkDescriptorPoolSize descriptor_pool_size = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
    };
    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        NULL,
        0,
        1,
        1,
        &descriptor_pool_size,
    };
    res = vkCreateDescriptorPool(
        g_device,
        &descriptor_pool_create_info,
        NULL,
        &g_descriptor_pool
    );
    CHECK(EMSG_CREATE_DESCRIPTOR);
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        NULL,
        g_descriptor_pool,
        1,
        &g_descriptor_set_layout,
    };
    res = vkAllocateDescriptorSets(
        g_device,
        &descriptor_set_allocate_info,
        &g_descriptor_set
    );
    CHECK(EMSG_CREATE_DESCRIPTOR);
    VkDescriptorBufferInfo uniform_buffer_descriptor_buffer_info = {
        g_uniform_buffer,
        0,
        VK_WHOLE_SIZE,
    };
    VkDescriptorImageInfo sampler_descriptor_image_info = {
        g_sampler,
        g_empty_image.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet uniform_buffer_write_descriptor_sets[] = {
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            NULL,
            g_descriptor_set,
            0,
            0,
            1,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            NULL,
            &uniform_buffer_descriptor_buffer_info,
            NULL,
        },
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            NULL,
            g_descriptor_set,
            1,
            0,
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            &sampler_descriptor_image_info,
            NULL,
            NULL,
        },
    };
    vkUpdateDescriptorSets(
        g_device,
        2,
        uniform_buffer_write_descriptor_sets,
        0,
        NULL
    );

    // pipeline
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        NULL,
        0,
        1,
        &g_descriptor_set_layout,
        0,
        NULL,
    };
    res = vkCreatePipelineLayout(
        g_device,
        &pipeline_layout_create_info,
        NULL,
        &g_pipeline_layout
    );
    CHECK(EMSG_CREATE_PIPELINE_LAYOUT);
    VkPipelineShaderStageCreateInfo shader_stage_create_info[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            g_vert_shader,
            "main",
            NULL,
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            g_frag_shader,
            "main",
            NULL,
        },
    };
    VkVertexInputBindingDescription vertex_input_binding_desc = {
        0,
        sizeof(Vertex),
        VK_VERTEX_INPUT_RATE_VERTEX,
    };
    VkVertexInputAttributeDescription vertex_input_attribute_desc[] = {
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
        { 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 3 },
    };
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        NULL,
        0,
        1,
        &vertex_input_binding_desc,
        2,
        vertex_input_attribute_desc,
    };
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        NULL,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE,
    };
    VkViewport viewport = {
        0.0f,
        0.0f,
        (float)g_width,
        (float)g_height,
        0.0f,
        1.0f,
    };
    VkRect2D scissor = {
        {0, 0},
        {g_width, g_height},
    };
    VkPipelineViewportStateCreateInfo viewport_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        NULL,
        0,
        1,
        &viewport,
        1,
        &scissor,
    };
    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        NULL,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
    };
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        NULL,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        0.0f,
        NULL,
        VK_FALSE,
        VK_FALSE,
    };
    const VkColorComponentFlags color_component_flag =
        VK_COLOR_COMPONENT_R_BIT 
        | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
        VK_TRUE,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_OP_ADD,
        color_component_flag,
    };
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        NULL,
        0,
        VK_FALSE,
        (VkLogicOp)0,
        1,
        &color_blend_attachment_state,
        {0.0f, 0.0f, 0.0f, 0.0f},
    };
    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        NULL,
        0,
        2,
        shader_stage_create_info,
        &vertex_input_state_create_info,
        &input_assembly_state_create_info,
        NULL,
        &viewport_state_create_info,
        &rasterization_state_create_info,
        &multisample_state_create_info,
        NULL,
        &color_blend_state_create_info,
        NULL,
        g_pipeline_layout,
        g_render_pass,
        0,
        NULL,
        0,
    };
    res = vkCreateGraphicsPipelines(
        g_device,
        VK_NULL_HANDLE,
        1,
        &pipeline_create_info,
        NULL,
        &g_pipeline
    );
    CHECK(EMSG_CREATE_PIPELINE);

    // g_square model
    Vertex vtxs[4] = {
        { -0.5f, -0.5f, 0.0f, 0.0f, 0.0f },
        { -0.5f,  0.5f, 0.0f, 0.0f, 1.0f },
        {  0.5f,  0.5f, 0.0f, 1.0f, 1.0f },
        {  0.5f, -0.5f, 0.0f, 1.0f, 0.0f },
    };
    uint32_t idxs[6] = { 0, 1, 2, 0, 2, 3 };
    g_square.index_cnt = 6;
    if (!create_buffer(
            sizeof(Vertex) * 4,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &g_square.vertex_buffer,
            &g_square.vertex_buffer_memory))
    {
        return EMSG_CREATE_SQUARE;
    }
    if (!create_buffer(
            sizeof(uint32_t) * 6,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &g_square.index_buffer,
            &g_square.index_buffer_memory))
    {
        return EMSG_CREATE_SQUARE;
    }
    if (!map_memory(
            g_square.vertex_buffer_memory,
            (void *)vtxs,
            sizeof(Vertex) * 4))
    {
        return EMSG_CREATE_SQUARE;
    }
    if (!map_memory(
            g_square.index_buffer_memory,
            (void *)idxs,
            sizeof(uint32_t) * 6))
    {
        return EMSG_CREATE_SQUARE;
    }

    // ubo
    Mat4 mat_e = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    Vec4 vec_e = { 0.0f, 0.0f, 0.0f, 0.0f };
    g_ubo.mat_scl = mat_e;
    g_ubo.mat_rtx = mat_e;
    g_ubo.mat_rty = mat_e;
    g_ubo.mat_rtz = mat_e;
    g_ubo.mat_trs = mat_e;
    g_ubo.mat_view = mat_e;
    g_ubo.mat_proj = mat_e;
    g_ubo.vec_uv = vec_e;
    g_ubo.vec_param = vec_e;
    if (!map_memory(
            g_uniform_buffer_memory,
            (void *)&g_ubo,
            sizeof(UniformBufferObject)))
    {
        return EMSG_MAP_UBO;
    }
    g_is_ubo_updated = 0;

    // finish
    return 0;
}

void skd_terminate_vulkan(void) {
    vkDeviceWaitIdle(g_device);
    vkFreeMemory(g_device, g_square.vertex_buffer_memory, NULL);
    vkFreeMemory(g_device, g_square.index_buffer_memory, NULL);
    vkDestroyBuffer(g_device, g_square.vertex_buffer, NULL);
    vkDestroyBuffer(g_device, g_square.index_buffer, NULL);
    vkDestroyPipeline(g_device, g_pipeline, NULL);
    vkDestroyPipelineLayout(g_device, g_pipeline_layout, NULL);
    vkDestroyDescriptorPool(g_device, g_descriptor_pool, NULL);
    vkDestroyDescriptorSetLayout(g_device, g_descriptor_set_layout, NULL);
    skd_unload_image(&g_empty_image);
    vkDestroySampler(g_device, g_sampler, NULL);
    vkFreeMemory(g_device, g_uniform_buffer_memory, NULL);
    vkDestroyBuffer(g_device, g_uniform_buffer, NULL);
    vkDestroyShaderModule(g_device, g_vert_shader, NULL);
    vkDestroyShaderModule(g_device, g_frag_shader, NULL);
    vkDestroySemaphore(g_device, g_present_semaphore, NULL);
    vkDestroySemaphore(g_device, g_render_semaphore, NULL);
    for (int i = 0; i < g_images_cnt; ++i) {
        vkDestroyFence(g_device, g_fences[i], NULL);
    }
    free(g_fences);
    vkFreeCommandBuffers(
        g_device,
        g_command_pool,
        g_images_cnt,
        g_command_buffers
    );
    vkDestroyCommandPool(g_device, g_command_pool, NULL);
    for (int i = 0; i < g_images_cnt; ++i) {
        vkDestroyFramebuffer(g_device, g_framebuffers[i], NULL);
    }
    for (int i = 0; i < g_images_cnt; ++i) {
        vkDestroyImageView(g_device, g_image_views[i], NULL);
    }
    free(g_framebuffers);
    free(g_image_views);
    vkDestroySwapchainKHR(g_device, g_swapchain, NULL);
    vkDestroyRenderPass(g_device, g_render_pass, NULL);
    vkDestroySurfaceKHR(g_instance, g_surface, NULL);
    vkDestroyDevice(g_device, NULL);
    vkDestroyInstance(g_instance, NULL);
}

// ========================================================================= //
//         Rendering API                                                     //
// ========================================================================= //

int skd_begin_render(unsigned int *p_id, float r, float g, float b) {
    VkResult res;
    // get next image index
    uint32_t next_image_idx;
    res = vkAcquireNextImageKHR(
        g_device,
        g_swapchain,
        UINT64_MAX,
        g_present_semaphore,
        VK_NULL_HANDLE,
        &next_image_idx
    );
    CHECK(0);
    // wait for a fence
    res = vkWaitForFences(
        g_device,
        1,
        &g_fences[next_image_idx],
        VK_TRUE,
        UINT64_MAX
    );
    CHECK(0);
    // begin command buffer
    const VkCommandBuffer command = g_command_buffers[next_image_idx];
    const VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        0,
        NULL,
    };
    res = vkBeginCommandBuffer(command, &command_buffer_begin_info);
    CHECK(0);
    // begin render pass
    const VkClearValue clear_value = { r, g, b, 0.0f };
    const VkExtent2D extent = { g_width, g_height };
    const VkRenderPassBeginInfo render_pass_begin_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        NULL,
        g_render_pass,
        g_framebuffers[next_image_idx],
        { {0, 0}, extent },
        1,
        &clear_value,
    };
    vkCmdBeginRenderPass(
        command,
        &render_pass_begin_info,
        VK_SUBPASS_CONTENTS_INLINE
    );
    // bind pipeline
    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline);
    // bind descriptor set
    vkCmdBindDescriptorSets(
        command,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        g_pipeline_layout,
        0,
        1,
        &g_descriptor_set,
        0,
        NULL
    );
    // bind square
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command, 0, 1, &g_square.vertex_buffer, &offset);
    vkCmdBindIndexBuffer(
        command,
        g_square.index_buffer,
        offset,
        VK_INDEX_TYPE_UINT32
    );
    // finish
    *p_id = next_image_idx;
    return 1;
}

int skd_end_render(unsigned int id) {
    VkResult res;
    const VkCommandBuffer command = g_command_buffers[id];
    const VkFence fence = g_fences[id];
    // end
    vkCmdEndRenderPass(command);
    vkEndCommandBuffer(command);
    // reset fences
    res = vkResetFences(g_device, 1, &fence);
    CHECK(0);
    // submit
    const VkPipelineStageFlags wait_stage_mask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        NULL,
        1,
        &g_present_semaphore,
        &wait_stage_mask,
        1,
        &command,
        1,
        &g_render_semaphore,
    };
    res = vkQueueSubmit(g_queue, 1, &submit_info, fence);
    CHECK(0);
    // present
    VkPresentInfoKHR present_info = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        NULL,
        1,
        &g_render_semaphore,
        1,
        &g_swapchain,
        &id,
        &res,
    };
    res = vkQueuePresentKHR(g_queue, &present_info);
    CHECK(0);
    // finish
    return 1;
}

int skd_draw(int id) {
    if (g_is_ubo_updated) {
        if (!map_memory(
                g_uniform_buffer_memory,
                (void *)&g_ubo,
                sizeof(UniformBufferObject)))
        {
            return 0;
        }
        g_is_ubo_updated = 0;
    }
    const VkCommandBuffer command = g_command_buffers[id];
    vkCmdDrawIndexed(command, g_square.index_cnt, 1, 0, 0, 0);
    return 1;
}

// ========================================================================= //
//         Image API                                                         //
// ========================================================================= //

int skd_load_image_from_memory(
    const unsigned char *pixels,
    int width,
    int height,
    Image *out
) {
    if (out == NULL) {
        return EMSG_NULL_OUT_IMAGE;
    }
    VkResult res;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const int size = width * height * 4;
    // staging buffer
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    if (!create_buffer(
            (VkDeviceSize)size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &staging_buffer,
            &staging_buffer_memory))
    {
        return EMSG_LOAD_IMAGE;
    }
    if (!map_memory(staging_buffer_memory, (void *)pixels, size)) {
        return EMSG_LOAD_IMAGE;
    }
    stbi_image_free((void *)pixels);
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
    VkImage image;
    res = vkCreateImage(g_device, &image_create_info, NULL, &image);
    CHECK(EMSG_LOAD_IMAGE);
    VkMemoryRequirements reqs;
    vkGetImageMemoryRequirements(g_device, image, &reqs);
    VkMemoryAllocateInfo allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        reqs.size,
        0,
    };
    allocate_info.memoryTypeIndex = get_memory_type_index(
        reqs,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    VkDeviceMemory memory;
    res = vkAllocateMemory(g_device, &allocate_info, NULL, &memory);
    CHECK(EMSG_LOAD_IMAGE);
    res = vkBindImageMemory(g_device, image, memory, 0);
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
        g_command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1,
    };
    VkCommandBuffer command;
    res = vkAllocateCommandBuffers(
        g_device,
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
        image,
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
        image,
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
    res = vkQueueSubmit(g_queue, 1, &submit_info, VK_NULL_HANDLE);
    CHECK(EMSG_LOAD_IMAGE);
    VkImageViewCreateInfo image_view_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        NULL,
        0,
        image,
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
    VkImageView view;
    res = vkCreateImageView(g_device, &image_view_create_info, NULL, &view);
    CHECK(EMSG_LOAD_IMAGE);
    // finish
    res = vkDeviceWaitIdle(g_device);
    CHECK(EMSG_LOAD_IMAGE);
    vkFreeCommandBuffers(g_device, g_command_pool, 1, &command);
    vkFreeMemory(g_device, staging_buffer_memory, NULL);
    vkDestroyBuffer(g_device, staging_buffer, NULL);
    out->image = image;
    out->view = view;
    out->memory = memory;
    return EMSG_SUCCESS;
}

int skd_load_image_from_file(const char *path, Image *out) {
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
    return skd_load_image_from_memory(pixels, width, height, out);
}

void skd_set_image(Image *image) {
    if (image == NULL) {
        g_ubo.vec_param.x = 0.0;
    } else {
        VkDescriptorImageInfo sampler_descriptor_image_info = {
            g_sampler,
            image->view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        VkWriteDescriptorSet uniform_buffer_write_descriptor_set = {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            NULL,
            g_descriptor_set,
            1,
            0,
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            &sampler_descriptor_image_info,
            NULL,
            NULL,
        };
        vkUpdateDescriptorSets(
            g_device,
            1,
            &uniform_buffer_write_descriptor_set,
            0,
            NULL
        );
        g_ubo.vec_param.x = 1.0;
    }
    g_is_ubo_updated = 1;
}

void skd_unload_image(Image *image) {
    if (image == NULL) {
        return;
    }
    skd_set_image(NULL);
    vkDeviceWaitIdle(g_device);
    vkDestroyImageView(g_device, image->view, NULL);
    vkDestroyImage(g_device, image->image, NULL);
    vkFreeMemory(g_device, image->memory, NULL);
}
    
// ========================================================================= //
//         Uniform Buffer Object API                                         //
// ========================================================================= //

void skd_scale(float x, float y, float z) {
    g_ubo.mat_scl.a11 = x;
    g_ubo.mat_scl.a22 = y;
    g_ubo.mat_scl.a33 = z;
    g_is_ubo_updated = 1;
}

void skd_trans(float x, float y, float z) {
    g_ubo.mat_trs.a14 = x;
    g_ubo.mat_trs.a24 = y;
    g_ubo.mat_trs.a34 = z;
    g_is_ubo_updated = 1;
}

void skd_uv(float u, float v, float u_end, float v_end) {
    g_ubo.vec_uv.x = u;
    g_ubo.vec_uv.y = v;
    g_ubo.vec_uv.z = u_end;
    g_ubo.vec_uv.w = v_end;
    g_is_ubo_updated = 1;
}
