#include <vulkan/vulkan.h>
#include <stdlib.h>

#define CHECK(p) if (res != VK_SUCCESS) (p)
#define VUL_EMSG_ENUM_INST_EXT_PROPS 1
#define VUL_EMSG_CREATE_INST 2

VkInstance g_instance;

int skd_init_vulkan() {
    VkResult res;

    // instance
    int inst_ext_props_cnt = 0;
    res = vkEnumerateInstanceExtensionProperties(
        NULL,
        &inst_ext_props_cnt,
        NULL
    );
    CHECK(VUL_EMSG_ENUM_INST_EXT_PROPS);
    VkExtensionProperties *inst_ext_props =
        (VkExtensionProperties*)
        malloc(sizeof(VkExtensionProperties) * inst_ext_props_cnt);
    res = vkEnumerateInstanceExtensionProperties(
        NULL,
        &inst_ext_props_cnt,
        inst_ext_props
    );
    CHECK(VUL_EMSG_ENUM_INST_EXT_PROPS);
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
    CHECK(VUL_EMSG_CREATE_INST);
    free((char **)inst_exts);
    free(inst_ext_props);

    // finish
    return 0;
}

void skd_terminate_vulkan() {
    if (g_instance != NULL) vkDestroyInstance(g_instance, NULL);
}
