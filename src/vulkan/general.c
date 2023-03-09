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

vkres_t skd_init_vulkan(SkdWindowParam *window_param, uint32_t max_image_texture_num) {
    VkResult res;
    // NOTE: considering empty image
    const uint32_t max_image_texture_num_add_1 = max_image_texture_num + 1;
    // NOTE: as for Fireball the num of descriptor sets
    // NOTE: is the same as that of image texture.
    const uint32_t max_descriptor_set_num = max_image_texture_num_add_1;

    // instance
    uint32_t inst_ext_props_cnt = 0;
    res = vkEnumerateInstanceExtensionProperties(NULL, &inst_ext_props_cnt, NULL);
    CHECK(EMSG_ENUM_INST_EXT_PROPS);
    VkExtensionProperties *inst_ext_props =
        (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * inst_ext_props_cnt);
    res = vkEnumerateInstanceExtensionProperties(NULL, &inst_ext_props_cnt, inst_ext_props);
    CHECK(EMSG_ENUM_INST_EXT_PROPS);
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
    res = vkCreateInstance(&create_info, NULL, &app.instance);
    CHECK(EMSG_CREATE_INST);
    free((char **)inst_exts);
    free(inst_ext_props);

    // physical device
    uint32_t phys_devices_cnt = 0;
    res = vkEnumeratePhysicalDevices(app.instance, &phys_devices_cnt, NULL);
    CHECK(EMSG_ENUM_PHYS_DEVICES);
    VkPhysicalDevice *phys_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * phys_devices_cnt);
    res = vkEnumeratePhysicalDevices(app.instance, &phys_devices_cnt, phys_devices);
    CHECK(EMSG_ENUM_PHYS_DEVICES);
    const VkPhysicalDevice phys_device = phys_devices[0];
    vkGetPhysicalDeviceMemoryProperties(phys_device, &app.phys_device_memory_prop);
    free(phys_devices);

    // queue family index
    uint32_t queue_family_props_cnt = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_props_cnt, NULL);
    VkQueueFamilyProperties *queue_family_props =
        (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queue_family_props_cnt);
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_props_cnt, queue_family_props);
    int32_t queue_family_index = -1;
    for (int32_t i = 0; i < queue_family_props_cnt; ++i) {
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
    uint32_t device_ext_props_cnt = 0;
    res = vkEnumerateDeviceExtensionProperties(phys_device, NULL, &device_ext_props_cnt, NULL);
    CHECK(EMSG_ENUM_DEVICE_EXT_PROPS);
    VkExtensionProperties *device_ext_props =
        (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * device_ext_props_cnt);
    res = vkEnumerateDeviceExtensionProperties(phys_device, NULL, &device_ext_props_cnt, device_ext_props);
    CHECK(EMSG_ENUM_DEVICE_EXT_PROPS);
    const char **device_exts = (const char**)malloc(sizeof(char*) * device_ext_props_cnt);
    int32_t device_exts_cnt = 0;
    for (int32_t i = 0; i < device_ext_props_cnt; ++i) {
        const int32_t cmpres = strcmp(
            device_ext_props[i].extensionName,
            "VK_EXT_buffer_device_address"
        );
        if (cmpres == 0) {
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
    res = vkCreateDevice(phys_device, &device_create_info, NULL, &app.device);
    CHECK(EMSG_CREATE_DEVICE);
    free((char**)device_exts);
    free(device_ext_props);

    // surface
#ifdef __linux__
    const VkXcbSurfaceCreateInfoKHR ci = {
        VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        NULL,
        0,
        window_param->data.xcb_window.connection,
        window_param->data.xcb_window.window,
    };
    res = vkCreateXcbSurfaceKHR(app.instance, &ci, NULL, &app.surface);
#elif _WIN32
    VkWin32SurfaceCreateInfoKHR ci = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        NULL,
        0,
        window_param->data.winapi_window.hinst,
        window_param->data.winapi_window.hwnd,
    };
    res = vkCreateWin32SurfaceKHR(app.instance, &ci, NULL, &app.surface);
#endif
    CHECK(EMSG_CREATE_SURFACE);
    uint32_t surface_formats_cnt = 0;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, app.surface, &surface_formats_cnt, NULL);
    CHECK(EMSG_GET_SURFACE_FORMATS);
    VkSurfaceFormatKHR *surface_formats =
        (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * surface_formats_cnt);
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, app.surface, &surface_formats_cnt, surface_formats);
    CHECK(EMSG_GET_SURFACE_FORMATS);
    int32_t surface_format_index = -1;
    for (int32_t i = 0; i < surface_formats_cnt; ++i) {
        if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
            surface_format_index = i;
            break;
        }
    }
    if (surface_format_index == -1) {
        return EMSG_GET_SURFACE_FORMATS;
    }
    const VkSurfaceFormatKHR surface_format = surface_formats[surface_format_index];
    VkSurfaceCapabilitiesKHR surface_capabilities;
    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, app.surface, &surface_capabilities);
    CHECK(EMSG_GET_SURFACE_CAPABILITIES);
    app.width = surface_capabilities.currentExtent.width;
    app.height = surface_capabilities.currentExtent.height;
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
    res = vkCreateRenderPass(app.device, &render_pass_create_info, NULL, &app.render_pass);
    CHECK(EMSG_CREATE_RENDER_PASS);

    // swapchain
    const uint32_t min_image_count = surface_capabilities.minImageCount > 2 ? surface_capabilities.minImageCount : 2;
    const VkSwapchainCreateInfoKHR swapchain_create_info = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        NULL,
        0,
        app.surface,
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
    res = vkCreateSwapchainKHR(app.device, &swapchain_create_info, NULL, &app.swapchain);
    CHECK(EMSG_CREATE_SWAPCHAIN);

    // image views
    res = vkGetSwapchainImagesKHR(app.device, app.swapchain, &app.images_cnt, NULL);
    CHECK(EMSG_GET_IMAGES);
    VkImage *images = (VkImage *)malloc(sizeof(VkImage) * app.images_cnt);
    res = vkGetSwapchainImagesKHR(app.device, app.swapchain, &app.images_cnt, images);
    CHECK(EMSG_GET_IMAGES);
    app.image_views = (VkImageView *)malloc(sizeof(VkImageView) * app.images_cnt);
    for (int32_t i = 0; i < app.images_cnt; ++i) {
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
            app.device,
            &image_view_create_info,
            NULL,
            &app.image_views[i]
        );
        CHECK(EMSG_CREATE_IMAGE_VIEW);
    }
    free(images);

    // framebuffers
    VkFramebufferCreateInfo frame_buffer_create_info = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        NULL,
        0,
        app.render_pass,
        1,
        NULL,
        app.width,
        app.height,
        1,
    };
    app.framebuffers = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * app.images_cnt);
    for (int32_t i = 0; i < app.images_cnt; ++i) {
        frame_buffer_create_info.pAttachments = &app.image_views[i];
        res = vkCreateFramebuffer(app.device, &frame_buffer_create_info, NULL, &app.framebuffers[i]);
        CHECK(EMSG_CREATE_FRAMEBUFFER);
    }

    // queue
    vkGetDeviceQueue(app.device, queue_family_index, 0, &app.queue);

    // command pool
    const VkCommandPoolCreateInfo command_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        NULL,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        queue_family_index,
    };
    res = vkCreateCommandPool(app.device, &command_pool_create_info, NULL, &app.command_pool);
    CHECK(EMSG_CREATE_COMMAND_POOL);

    // shaders
    VkShaderModuleCreateInfo shader_module_create_info = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        NULL,
        0,
        shader_vert_size,
        (const uint32_t *)shader_vert_data,
    };
    res = vkCreateShaderModule(app.device, &shader_module_create_info, NULL, &app.vert_shader);
    CHECK(EMSG_CREATE_SHADER);
    shader_module_create_info.codeSize = shader_frag_size;
    shader_module_create_info.pCode = (const uint32_t *)shader_frag_data;
    res = vkCreateShaderModule(app.device, &shader_module_create_info, NULL, &app.frag_shader);
    CHECK(EMSG_CREATE_SHADER);

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
    res = vkCreateSampler(app.device, &sampler_create_info, NULL, &app.sampler);
    CHECK(EMSG_CREATE_SAMPLER);

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
    res = vkCreateDescriptorPool(
        app.device,
        &descriptor_pool_create_info,
        NULL,
        &app.descriptor_pool
    );
    CHECK(EMSG_CREATE_DESCRIPTOR_POOL);
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
    res = vkCreateDescriptorSetLayout(app.device, &descriptor_set_layout_create_info, NULL, &app.descriptor_set_layout);
    CHECK(EMSG_CREATE_DESCRIPTOR_SET_LAYOUT);

    // push constant range
    VkPushConstantRange push_constant_range = {
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(ModelData),
    };

    // pipeline
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        NULL,
        0,
        1,
        &app.descriptor_set_layout,
        1,
        &push_constant_range,
    };
    res = vkCreatePipelineLayout(app.device, &pipeline_layout_create_info, NULL, &app.pipeline_layout);
    CHECK(EMSG_CREATE_PIPELINE_LAYOUT);
    VkPipelineShaderStageCreateInfo shader_stage_create_info[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            app.vert_shader,
            "main",
            NULL,
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            app.frag_shader,
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
        (float)app.width,
        (float)app.height,
        0.0f,
        1.0f,
    };
    VkRect2D scissor = { {0, 0}, {app.width, app.height} };
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
        app.pipeline_layout,
        app.render_pass,
        0,
        NULL,
        0,
    };
    res = vkCreateGraphicsPipelines(app.device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &app.pipeline);
    CHECK(EMSG_CREATE_PIPELINE);

// frame data

    // command buffers
    const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        NULL,
        app.command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1,
    };
    res = vkAllocateCommandBuffers(app.device, &command_buffer_allocate_info, &app.framedata.command_buffer);
    CHECK(EMSG_ALLOCATE_COMMAND_BUFFERS);

    // fences
    const VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        NULL,
        VK_FENCE_CREATE_SIGNALED_BIT,
    };
    res = vkCreateFence(app.device, &fence_create_info, NULL, &app.framedata.fence);
    CHECK(EMSG_CREATE_FENCE);

    // semaphores
    const VkSemaphoreCreateInfo semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        NULL,
        0,
    };
    res = vkCreateSemaphore(app.device, &semaphore_create_info, NULL, &app.framedata.render_semaphore);
    CHECK(EMSG_CREATE_SEMAPHORE);
    res = vkCreateSemaphore(app.device, &semaphore_create_info, NULL, &app.framedata.present_semaphore);
    CHECK(EMSG_CREATE_SEMAPHORE);

// rendering default objects

    // descriptor sets #1
    app.resource.max_descriptor_set_num = max_descriptor_set_num;
    app.resource.descriptor_sets =
        (VkDescriptorSet *)malloc(sizeof(VkDescriptorSet) * app.resource.max_descriptor_set_num);

    // camera
    const CameraData default_camera_data = DEFAULT_CAMERA_DATA;
    if (!create_buffer(
            &app,
            sizeof(CameraData),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &app.resource.camera.buffer,
            &app.resource.camera.buffer_memory))
    {
        return EMSG_CREATE_CAMERA;
    }
    if (!map_memory(&app, app.resource.camera.buffer_memory, (void *)&default_camera_data, sizeof(CameraData))) {
        return EMSG_CREATE_SQUARE;
    }

    // image textures
    app.resource.max_image_texture_num = max_image_texture_num_add_1;
    app.resource.image_textures = (Image *)calloc(app.resource.max_image_texture_num, sizeof(Image));
    // empty image
    const unsigned char pixels[] = { 0xff, 0xff, 0xff, 0xff };
    if (load_image_texture(pixels, 1, 1, 0) != EMSG_VULKAN_SUCCESS) {
        return EMSG_CREATE_EMPTY_IMAGE;
    }

    // descriptor sets #2
    for (int32_t i = 0; i < app.resource.max_descriptor_set_num; ++i) {
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            NULL,
            app.descriptor_pool,
            1,
            &app.descriptor_set_layout,
        };
        res = vkAllocateDescriptorSets(app.device, &descriptor_set_allocate_info, &app.resource.descriptor_sets[i]);
        CHECK(EMSG_CREATE_DESCRIPTOR_SET);
        VkDescriptorBufferInfo camera_descriptor_buffer_info = {
            app.resource.camera.buffer,
            0,
            VK_WHOLE_SIZE,
        };
        VkDescriptorImageInfo sampler_descriptor_image_info = {
            app.sampler,
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
        vkUpdateDescriptorSets(app.device, 2, write_descriptor_sets, 0, NULL);
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
    if (!create_buffer(
            &app,
            sizeof(float) * 5 * 4,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &app.resource.square.vertex_buffer,
            &app.resource.square.vertex_buffer_memory))
    {
        return EMSG_CREATE_SQUARE;
    }
    if (!create_buffer(
            &app,
            sizeof(uint32_t) * 6,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &app.resource.square.index_buffer,
            &app.resource.square.index_buffer_memory))
    {
        return EMSG_CREATE_SQUARE;
    }
    if (!map_memory(&app, app.resource.square.vertex_buffer_memory, (void *)vtxs, sizeof(float) * 5 * 4)) {
        return EMSG_CREATE_SQUARE;
    }
    if (!map_memory(&app, app.resource.square.index_buffer_memory, (void *)idxs, sizeof(uint32_t) * 6)) {
        return EMSG_CREATE_SQUARE;
    }

    // finish
    return EMSG_VULKAN_SUCCESS;
}

void skd_terminate_vulkan(void) {
    vkDeviceWaitIdle(app.device);
    // frame data
    vkDestroySemaphore(app.device, app.framedata.present_semaphore, NULL);
    vkDestroySemaphore(app.device, app.framedata.render_semaphore, NULL);
    vkDestroyFence(app.device, app.framedata.fence, NULL);
    vkFreeCommandBuffers(app.device, app.command_pool, 1, &app.framedata.command_buffer);
    // rendering default objects
    for (int32_t i = 1; i < app.resource.max_image_texture_num; ++i) {
        skd_unload_image(i);
    }
    skd_unload_image(0);
    vkFreeMemory(app.device, app.resource.camera.buffer_memory, NULL);
    vkDestroyBuffer(app.device, app.resource.camera.buffer, NULL);
    vkFreeMemory(app.device, app.resource.square.vertex_buffer_memory, NULL);
    vkFreeMemory(app.device, app.resource.square.index_buffer_memory, NULL);
    vkDestroyBuffer(app.device, app.resource.square.vertex_buffer, NULL);
    vkDestroyBuffer(app.device, app.resource.square.index_buffer, NULL);
    // others
    vkDestroyPipeline(app.device, app.pipeline, NULL);
    vkDestroyPipelineLayout(app.device, app.pipeline_layout, NULL);
    vkDestroyDescriptorPool(app.device, app.descriptor_pool, NULL);
    vkDestroyDescriptorSetLayout(app.device, app.descriptor_set_layout, NULL);
    vkDestroySampler(app.device, app.sampler, NULL);
    vkDestroyShaderModule(app.device, app.vert_shader, NULL);
    vkDestroyShaderModule(app.device, app.frag_shader, NULL);
    vkDestroyCommandPool(app.device, app.command_pool, NULL);
    for (int32_t i = 0; i < app.images_cnt; ++i) {
        vkDestroyFramebuffer(app.device, app.framebuffers[i], NULL);
    }
    for (int32_t i = 0; i < app.images_cnt; ++i) {
        vkDestroyImageView(app.device, app.image_views[i], NULL);
    }
    free(app.framebuffers);
    free(app.image_views);
    vkDestroySwapchainKHR(app.device, app.swapchain, NULL);
    vkDestroyRenderPass(app.device, app.render_pass, NULL);
    vkDestroySurfaceKHR(app.instance, app.surface, NULL);
    vkDestroyDevice(app.device, NULL);
    vkDestroyInstance(app.instance, NULL);
}
