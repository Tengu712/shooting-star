#include "private.h"

#ifdef RELEASE_BUILD
#define INST_LAYER_NAMES_CNT 0
#define INST_LAYER_NAMES { }
#else
#define INST_LAYER_NAMES_CNT 1
#define INST_LAYER_NAMES { "VK_LAYER_KHRONOS_validation\0" }
#endif

extern char shader_vert_data[];
extern int32_t shader_vert_size;
extern char shader_frag_data[];
extern int32_t shader_frag_size;

VulkanApp app;

warn_t init_vulkan(SkdWindowParam *window_param, uint32_t max_image_texture_num) {
    log_info("start to initialize Vulkan ...");

    warn_t res = FB_SUCCESS;
    // NOTE: considering empty image
    const uint32_t max_image_texture_num_add_1 = max_image_texture_num + 1;
    // NOTE: as for Fireball the num of descriptor sets
    // NOTE: is the same as that of image texture.
    const uint32_t max_descriptor_set_num = max_image_texture_num_add_1;

// core

    // instance
    uint32_t inst_ext_props_cnt = 0;
    CHECK(vkEnumerateInstanceExtensionProperties(NULL, &inst_ext_props_cnt, NULL), "failed to get the number of instance extension properties.");
    VkExtensionProperties *inst_ext_props = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * inst_ext_props_cnt);
    CHECK(vkEnumerateInstanceExtensionProperties(NULL, &inst_ext_props_cnt, inst_ext_props), "failed to enumerate instance extension properties.");
    const char **inst_exts = (const char **)malloc(sizeof(char *) * inst_ext_props_cnt);
    const int32_t inst_exts_cnt = inst_ext_props_cnt;
    for (int32_t i = 0; i < inst_ext_props_cnt; ++i) {
        inst_exts[i] = inst_ext_props[i].extensionName;
    }
    const int32_t inst_layer_names_cnt = INST_LAYER_NAMES_CNT;
    const char *inst_layer_names[INST_LAYER_NAMES_CNT] = INST_LAYER_NAMES;
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
    CHECK(vkCreateInstance(&create_info, NULL, &app.core.instance), "failed to create instance.");
    free((char **)inst_exts);
    free(inst_ext_props);

    // physical device
    uint32_t phys_devices_cnt = 0;
    CHECK(vkEnumeratePhysicalDevices(app.core.instance, &phys_devices_cnt, NULL), "failed to get the number of physical devices.");
    VkPhysicalDevice *phys_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * phys_devices_cnt);
    CHECK(vkEnumeratePhysicalDevices(app.core.instance, &phys_devices_cnt, phys_devices), "failed to enumerate physical devices.");
    const VkPhysicalDevice phys_device = phys_devices[0];
    vkGetPhysicalDeviceMemoryProperties(phys_device, &app.core.phys_device_memory_prop);
    free(phys_devices);

    // queue family index
    uint32_t queue_family_props_cnt = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_props_cnt, NULL);
    VkQueueFamilyProperties *queue_family_props = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queue_family_props_cnt);
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_props_cnt, queue_family_props);
    int32_t queue_family_index = -1;
    for (int32_t i = 0; i < queue_family_props_cnt; ++i) {
        if ((queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0) {
            queue_family_index = i;
            break;
        }
    }
    if (queue_family_index == -1) error("failed to find queue family index.");
    free(queue_family_props);

    // device
    uint32_t device_ext_props_cnt = 0;
    CHECK(vkEnumerateDeviceExtensionProperties(phys_device, NULL, &device_ext_props_cnt, NULL), "failed to get the number of device extension properties.");
    VkExtensionProperties *device_ext_props = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * device_ext_props_cnt);
    CHECK(vkEnumerateDeviceExtensionProperties(phys_device, NULL, &device_ext_props_cnt, device_ext_props), "failed to enumerate device extension properties.");
    const char **device_exts = (const char**)malloc(sizeof(char*) * device_ext_props_cnt);
    int32_t device_exts_cnt = 0;
    for (int32_t i = 0; i < device_ext_props_cnt; ++i) {
        if (strcmp(device_ext_props[i].extensionName, "VK_EXT_buffer_device_address") == 0) {
            continue;
        }
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
    CHECK(vkCreateDevice(phys_device, &device_create_info, NULL, &app.core.device), "failed to create device.");
    free((char**)device_exts);
    free(device_ext_props);

// renderer

    // surface
#ifdef __linux__
    const VkXcbSurfaceCreateInfoKHR ci = {
        VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        NULL,
        0,
        window_param->data.xcb_window.connection,
        window_param->data.xcb_window.window,
    };
    CHECK(vkCreateXcbSurfaceKHR(app.core.instance, &ci, NULL, &app.rendering.surface), "failed to create xcb surface.");
#elif _WIN32
    VkWin32SurfaceCreateInfoKHR ci = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        NULL,
        0,
        window_param->data.winapi_window.hinst,
        window_param->data.winapi_window.hwnd,
    };
    CHECK(vkCreateWin32SurfaceKHR(app.core.instance, &ci, NULL, &app.rendering.surface), "failed to create win32 surface.");
#endif
    uint32_t surface_formats_cnt = 0;
    CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, app.rendering.surface, &surface_formats_cnt, NULL), "failed to get the number of surface formats.");
    VkSurfaceFormatKHR *surface_formats = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * surface_formats_cnt);
    CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, app.rendering.surface, &surface_formats_cnt, surface_formats), "failed to get surface formats.");
    int32_t surface_format_index = -1;
    for (int32_t i = 0; i < surface_formats_cnt; ++i) {
        if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
            surface_format_index = i;
            break;
        }
    }
    if (surface_format_index == -1) error("failed to get surface format.");
    const VkSurfaceFormatKHR surface_format = surface_formats[surface_format_index];
    VkSurfaceCapabilitiesKHR surface_capabilities;
    CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, app.rendering.surface, &surface_capabilities), "failed to get surface capabilities.");
    app.rendering.width = surface_capabilities.currentExtent.width;
    app.rendering.height = surface_capabilities.currentExtent.height;
    free(surface_formats);

    // swapchain
    const uint32_t min_image_count = surface_capabilities.minImageCount > 2 ? surface_capabilities.minImageCount : 2;
    const VkSwapchainCreateInfoKHR swapchain_create_info = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        NULL,
        0,
        app.rendering.surface,
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
    CHECK(vkCreateSwapchainKHR(app.core.device, &swapchain_create_info, NULL, &app.rendering.swapchain), "failed to create swapchain.");

    // image views
    CHECK(vkGetSwapchainImagesKHR(app.core.device, app.rendering.swapchain, &app.rendering.images_cnt, NULL), "failed to get the number of swapchain images.");
    VkImage *images = (VkImage *)malloc(sizeof(VkImage) * app.rendering.images_cnt);
    CHECK(vkGetSwapchainImagesKHR(app.core.device, app.rendering.swapchain, &app.rendering.images_cnt, images), "failed to get swapchain images.");
    app.rendering.image_views = (VkImageView *)malloc(sizeof(VkImageView) * app.rendering.images_cnt);
    for (int32_t i = 0; i < app.rendering.images_cnt; ++i) {
        const VkImageViewCreateInfo image_view_create_info = {
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
        CHECK(vkCreateImageView(app.core.device, &image_view_create_info, NULL, &app.rendering.image_views[i]), "failed to create image view.");
    }
    free(images);

    // queue
    vkGetDeviceQueue(app.core.device, queue_family_index, 0, &app.rendering.queue);

    // command pool
    const VkCommandPoolCreateInfo command_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        NULL,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        queue_family_index,
    };
    CHECK(vkCreateCommandPool(app.core.device, &command_pool_create_info, NULL, &app.rendering.command_pool), "failed to create command pool.");

// pipeline

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
    CHECK(vkCreateRenderPass(app.core.device, &render_pass_create_info, NULL, &app.pipeline.render_pass), "failed to create render pass.");

    // framebuffers
    VkFramebufferCreateInfo frame_buffer_create_info = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        NULL,
        0,
        app.pipeline.render_pass,
        1,
        NULL,
        app.rendering.width,
        app.rendering.height,
        1,
    };
    app.pipeline.framebuffers = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * app.rendering.images_cnt);
    for (int32_t i = 0; i < app.rendering.images_cnt; ++i) {
        frame_buffer_create_info.pAttachments = &app.rendering.image_views[i];
        CHECK(vkCreateFramebuffer(app.core.device, &frame_buffer_create_info, NULL, &app.pipeline.framebuffers[i]), "failed to create framebuffer.");
    }

    // shaders
    VkShaderModuleCreateInfo shader_module_create_info = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        NULL,
        0,
        shader_vert_size,
        (const uint32_t *)shader_vert_data,
    };
    CHECK(vkCreateShaderModule(app.core.device, &shader_module_create_info, NULL, &app.pipeline.vert_shader), "failed to create vertex shader module.");
    shader_module_create_info.codeSize = shader_frag_size;
    shader_module_create_info.pCode = (const uint32_t *)shader_frag_data;
    CHECK(vkCreateShaderModule(app.core.device, &shader_module_create_info, NULL, &app.pipeline.frag_shader), "failed to create fragment shader module.");

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
    CHECK(vkCreateSampler(app.core.device, &sampler_create_info, NULL, &app.pipeline.sampler), "failed to create sampler.");

    // descriptor
    VkDescriptorPoolSize descriptor_pool_sizes[] = {
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
        },
        {
            VK_DESCRIPTOR_TYPE_SAMPLER,
            max_image_texture_num_add_1,
        },
    };
    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        NULL,
        0,
        max_descriptor_set_num,
        2,
        descriptor_pool_sizes,
    };
    CHECK(vkCreateDescriptorPool(app.core.device, &descriptor_pool_create_info, NULL, &app.pipeline.descriptor_pool), "failed to create descriptor pool.");
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
    CHECK(vkCreateDescriptorSetLayout(app.core.device, &descriptor_set_layout_create_info, NULL, &app.pipeline.descriptor_set_layout), "failed to create descriptor set layout.");

    // pipeline layout
    VkPushConstantRange push_constant_range = {
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(ModelData),
    };
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        NULL,
        0,
        1,
        &app.pipeline.descriptor_set_layout,
        1,
        &push_constant_range,
    };
    CHECK(vkCreatePipelineLayout(app.core.device, &pipeline_layout_create_info, NULL, &app.pipeline.pipeline_layout), "failed to create pipeline layout.");

    // pipeline
    VkPipelineShaderStageCreateInfo shader_stage_create_info[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            app.pipeline.vert_shader,
            "main",
            NULL,
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            app.pipeline.frag_shader,
            "main",
            NULL,
        },
    };
    VkVertexInputBindingDescription vertex_input_binding_desc = {
        0,
        sizeof(float) * 5,
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
        (float)app.rendering.width,
        (float)app.rendering.height,
        0.0f,
        1.0f,
    };
    VkRect2D scissor = { {0, 0}, {app.rendering.width, app.rendering.height} };
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
        app.pipeline.pipeline_layout,
        app.pipeline.render_pass,
        0,
        NULL,
        0,
    };
    CHECK(vkCreateGraphicsPipelines(app.core.device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &app.pipeline.pipeline), "failed to create pipeline.");

// frame data

    // command buffers
    const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        NULL,
        app.rendering.command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1,
    };
    CHECK(vkAllocateCommandBuffers(app.core.device, &command_buffer_allocate_info, &app.framedata.command_buffer), "failed to allocate command buffers.");

    // fence
    const VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        NULL,
        VK_FENCE_CREATE_SIGNALED_BIT,
    };
    CHECK(vkCreateFence(app.core.device, &fence_create_info, NULL, &app.framedata.fence), "failed to create fence.");

    // semaphores
    const VkSemaphoreCreateInfo semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        NULL,
        0,
    };
    CHECK(vkCreateSemaphore(app.core.device, &semaphore_create_info, NULL, &app.framedata.render_semaphore), "failed to create semaphore to wait for render completed.");
    CHECK(vkCreateSemaphore(app.core.device, &semaphore_create_info, NULL, &app.framedata.present_semaphore), "failed to create semaphore to wait for present completed.");

// rendering default objects

    // descriptor sets #1
    app.resource.max_descriptor_set_num = max_descriptor_set_num;
    app.resource.descriptor_sets = (VkDescriptorSet *)malloc(sizeof(VkDescriptorSet) * app.resource.max_descriptor_set_num);

    // camera
    const CameraData default_camera_data = DEFAULT_CAMERA_DATA;
    if (create_buffer(
            &app,
            sizeof(CameraData),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &app.resource.camera.buffer,
            &app.resource.camera.buffer_memory) != FB_SUCCESS)
    {
        error("failed to create buffer for camera.");
    }
    if (!map_memory(&app, app.resource.camera.buffer_memory, (void *)&default_camera_data, sizeof(CameraData))) {
        res = warning("failed to map camera data.");
    }

    // image textures
    app.resource.max_image_texture_num = max_image_texture_num_add_1;
    app.resource.image_textures = (Image *)calloc(app.resource.max_image_texture_num, sizeof(Image));
    // empty image
    const unsigned char pixels[] = { 0xff, 0xff, 0xff, 0xff };
    if (load_image_texture(pixels, 1, 1, 0) != FB_SUCCESS) res = FB_WARN;

    // descriptor sets #2
    for (int32_t i = 0; i < app.resource.max_descriptor_set_num; ++i) {
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            NULL,
            app.pipeline.descriptor_pool,
            1,
            &app.pipeline.descriptor_set_layout,
        };
        CHECK(vkAllocateDescriptorSets(app.core.device, &descriptor_set_allocate_info, &app.resource.descriptor_sets[i]), "failed to allocate descriptor sets.");
        VkDescriptorBufferInfo camera_descriptor_buffer_info = {
            app.resource.camera.buffer,
            0,
            VK_WHOLE_SIZE,
        };
        VkDescriptorImageInfo sampler_descriptor_image_info = {
            app.pipeline.sampler,
            app.resource.image_textures[0].view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        VkWriteDescriptorSet write_descriptor_sets[] = {
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                NULL,
                app.resource.descriptor_sets[i],
                0,
                0,
                1,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                NULL,
                &camera_descriptor_buffer_info,
                NULL,
            },
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                NULL,
                app.resource.descriptor_sets[i],
                1,
                0,
                1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                &sampler_descriptor_image_info,
                NULL,
                NULL,
            },
        };
        vkUpdateDescriptorSets(app.core.device, 2, write_descriptor_sets, 0, NULL);
    }

    // square
    float vtxs[4][5] = {
        { -0.5f, -0.5f, 0.0f, 0.0f, 0.0f },
        { -0.5f,  0.5f, 0.0f, 0.0f, 1.0f },
        {  0.5f,  0.5f, 0.0f, 1.0f, 1.0f },
        {  0.5f, -0.5f, 0.0f, 1.0f, 0.0f },
    };
    uint32_t idxs[6] = { 0, 1, 2, 0, 2, 3 };
    app.resource.square.index_cnt = 6;
    if (create_buffer(
            &app,
            sizeof(float) * 5 * 4,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &app.resource.square.vertex_buffer,
            &app.resource.square.vertex_buffer_memory) != FB_SUCCESS)
    {
        error("failed to create vertex buffer.");
    }
    if (create_buffer(
            &app,
            sizeof(uint32_t) * 6,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &app.resource.square.index_buffer,
            &app.resource.square.index_buffer_memory) != FB_SUCCESS)
    {
        error("failed to create index buffer.");
    }
    if (!map_memory(&app, app.resource.square.vertex_buffer_memory, (void *)vtxs, sizeof(float) * 5 * 4)) {
        res = warning("failed to map vertex buffer.");
    }
    if (!map_memory(&app, app.resource.square.index_buffer_memory, (void *)idxs, sizeof(uint32_t) * 6)) {
        res = warning("failed to map index buffer.");
    }

    // finish
    log_info("succeeded to initialize vulkan.");
    return res;
}

void terminate_vulkan(void) {
    vkDeviceWaitIdle(app.core.device);
    // frame data
    vkDestroySemaphore(app.core.device, app.framedata.present_semaphore, NULL);
    vkDestroySemaphore(app.core.device, app.framedata.render_semaphore, NULL);
    vkDestroyFence(app.core.device, app.framedata.fence, NULL);
    vkFreeCommandBuffers(app.core.device, app.rendering.command_pool, 1, &app.framedata.command_buffer);
    // rendering default objects
    for (int32_t i = 1; i < app.resource.max_image_texture_num; ++i) {
        unload_image(i);
    }
    unload_image(0);
    vkFreeMemory(app.core.device, app.resource.camera.buffer_memory, NULL);
    vkDestroyBuffer(app.core.device, app.resource.camera.buffer, NULL);
    vkFreeMemory(app.core.device, app.resource.square.vertex_buffer_memory, NULL);
    vkFreeMemory(app.core.device, app.resource.square.index_buffer_memory, NULL);
    vkDestroyBuffer(app.core.device, app.resource.square.vertex_buffer, NULL);
    vkDestroyBuffer(app.core.device, app.resource.square.index_buffer, NULL);
    // others
    vkDestroyPipeline(app.core.device, app.pipeline.pipeline, NULL);
    vkDestroyPipelineLayout(app.core.device, app.pipeline.pipeline_layout, NULL);
    vkDestroyDescriptorPool(app.core.device, app.pipeline.descriptor_pool, NULL);
    vkDestroyDescriptorSetLayout(app.core.device, app.pipeline.descriptor_set_layout, NULL);
    vkDestroySampler(app.core.device, app.pipeline.sampler, NULL);
    vkDestroyShaderModule(app.core.device, app.pipeline.vert_shader, NULL);
    vkDestroyShaderModule(app.core.device, app.pipeline.frag_shader, NULL);
    vkDestroyCommandPool(app.core.device, app.rendering.command_pool, NULL);
    for (int32_t i = 0; i < app.rendering.images_cnt; ++i) {
        vkDestroyFramebuffer(app.core.device, app.pipeline.framebuffers[i], NULL);
    }
    for (int32_t i = 0; i < app.rendering.images_cnt; ++i) {
        vkDestroyImageView(app.core.device, app.rendering.image_views[i], NULL);
    }
    free(app.pipeline.framebuffers);
    free(app.rendering.image_views);
    vkDestroySwapchainKHR(app.core.device, app.rendering.swapchain, NULL);
    vkDestroyRenderPass(app.core.device, app.pipeline.render_pass, NULL);
    vkDestroySurfaceKHR(app.core.instance, app.rendering.surface, NULL);
    vkDestroyDevice(app.core.device, NULL);
    vkDestroyInstance(app.core.instance, NULL);
}
