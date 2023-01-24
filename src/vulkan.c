#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>

#include "vulkan.h"

#define CHECK(p) if (res != VK_SUCCESS) return (p);
#define EMSG_ENUM_INST_EXT_PROPS 1
#define EMSG_CREATE_INST 2
#define EMSG_ENUM_PHYS_DEVICES 3
#define EMSG_FIND_QUEUE_FAMILY_INDEX 4
#define EMSG_ENUM_DEVICE_EXT_PROPS 5
#define EMSG_CREATE_DEVICE 6
#define EMSG_CREATE_SURFACE 7
#define EMSG_GET_SURFACE_FORMATS 8
#define EMSG_GET_SURFACE_CAPABILITIES 9
#define EMSG_CREATE_RENDER_PASS 10
#define EMSG_CREATE_SWAPCHAIN 11
#define EMSG_GET_IMAGES 12
#define EMSG_CREATE_IMAGE_VIEW 13

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

int create_xcb_surface(SkdWindowUnion *window_param) {
    const VkXcbSurfaceCreateInfoKHR ci = {
        VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        NULL,
        0,
        window_param->xcb_window.connection,
        window_param->xcb_window.window,
    };
    VkResult res = vkCreateXcbSurfaceKHR(g_instance, &ci, NULL, &g_surface);
    CHECK(0);
    return 1;
}

int skd_init_vulkan(int window_kind, SkdWindowUnion *window_param) {
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
    switch (window_kind) {
        case SKD_WIN_KIND_XCB:
            if (create_xcb_surface(window_param) != 1) {
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

    // finish
    return 0;
}

void skd_terminate_vulkan(void) {
    vkDeviceWaitIdle(g_device);
    for (int i = 0; i < g_images_cnt; ++i) {
        vkDestroyImageView(g_device, g_image_views[i], NULL);
    }
    free(g_image_views);
    vkDestroySwapchainKHR(g_device, g_swapchain, NULL);
    vkDestroyRenderPass(g_device, g_render_pass, NULL);
    vkDestroySurfaceKHR(g_instance, g_surface, NULL);
    vkDestroyDevice(g_device, NULL);
    vkDestroyInstance(g_instance, NULL);
}
