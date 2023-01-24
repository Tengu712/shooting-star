#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(p) if (res != VK_SUCCESS) (p)
#define EMSG_ENUM_INST_EXT_PROPS 1
#define EMSG_CREATE_INST 2
#define EMSG_ENUM_PHYS_DEVICES 3
#define EMSG_FIND_QUEUE_FAMILY_INDEX 4
#define EMSG_ENUM_DEVICE_EXT_PROPS 5
#define EMSG_CREATE_DEVICE 6

VkInstance g_instance;
VkPhysicalDeviceMemoryProperties g_phys_device_memory_prop;
VkDevice g_device;

int skd_init_vulkan() {
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

    // finish
    return 0;
}

void skd_terminate_vulkan() {
    if (g_device != NULL) vkDeviceWaitIdle(g_device);
    if (g_device != NULL) vkDestroyDevice(g_device, NULL);
    if (g_instance != NULL) vkDestroyInstance(g_instance, NULL);
}
