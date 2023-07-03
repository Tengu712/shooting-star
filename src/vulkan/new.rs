use super::math::*;
use super::*;

use crate::shaders::*;
use crate::window::WindowApp;

use std::mem::size_of;
use std::os::raw::{c_char, c_void};

const APP_NAME: *const c_char = [
    'V' as c_char,
    'k' as c_char,
    'A' as c_char,
    'p' as c_char,
    'p' as c_char,
    '\0' as c_char,
]
.as_ptr();

#[cfg(debug_assertions)]
const INST_LAYER_NAMES: [*const c_char; 1] = [PCHAR_VK_LAYER_KHRONOS_validation];
#[cfg(not(debug_assertions))]
const INST_LAYER_NAMES: [*const c_char; 0] = [];

#[cfg(target_os = "linux")]
const INST_EXT_NAME_SURFACE: *const c_char = PCHAR_VK_KHR_xlib_surface;
#[cfg(target_os = "windows")]
const INST_EXT_NAME_SURFACE: *const c_char = PCHAR_VK_KHR_win32_surface;

#[cfg(debug_assertions)]
const INST_EXT_NAMES: [*const c_char; 4] = [
    PCHAR_VK_EXT_debug_report,
    PCHAR_VK_EXT_debug_utils,
    PCHAR_VK_KHR_surface,
    INST_EXT_NAME_SURFACE,
];
#[cfg(not(debug_assertions))]
const INST_EXT_NAMES: [*const c_char; 2] = [PCHAR_VK_KHR_surface, INST_EXT_NAME_SURFACE];

const DEVICE_EXT_NAMES: [*const c_char; 1] = [PCHAR_VK_KHR_swapchain];

const SHADER_ENTRY_NAME: *const c_char = [
    'm' as c_char,
    'a' as c_char,
    'i' as c_char,
    'n' as c_char,
    '\0' as c_char,
]
.as_ptr();

const SQUARE_VTXS: [Vertex; 4] = [
    Vertex {
        in_pos: [-0.5, 0.5, 0.0],
        in_uv: [0.0, 1.0],
    },
    Vertex {
        in_pos: [0.5, 0.5, 0.0],
        in_uv: [1.0, 1.0],
    },
    Vertex {
        in_pos: [0.5, -0.5, 0.0],
        in_uv: [1.0, 0.0],
    },
    Vertex {
        in_pos: [-0.5, -0.5, 0.0],
        in_uv: [0.0, 0.0],
    },
];
const SQUARE_IDXS: [u32; 6] = [0, 1, 2, 0, 2, 3];

const DEF_IMG_TEX_WIDTH: u32 = 2;
const DEF_IMG_TEX_HEIGHT: u32 = 2;
const DEF_IMG_TEX_PIXELS: [u8; 16] = [
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
];
const DEF_IMG_TEX_PIXELS_SIZE: usize = size_of::<u8>() * DEF_IMG_TEX_PIXELS.len();

impl VulkanApp {
    /// A constructor.
    /// The param `max_img_tex_cnt` is the max num of image textures and is the same as the num of descriptor sets in this app.
    /// The param `max_img_tex_cnt` must be greater than 0.
    pub fn new(window_app: &WindowApp, max_img_tex_cnt: u32) -> Self {
        if max_img_tex_cnt == 0 {
            ss_error("the max number of image texture must be greater than 0.");
        }

        // ========================================================================================================= //
        //     core                                                                                                  //
        // ========================================================================================================= //

        let instance = {
            let ai = VkApplicationInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_APPLICATION_INFO,
                pNext: null(),
                pApplicationName: APP_NAME,
                applicationVersion: 0,
                pEngineName: APP_NAME,
                engineVersion: VK_MAKE_VERSION(0, 1, 0),
                apiVersion: VK_MAKE_API_VERSION(0, 1, 2, 0),
            };
            let ci = VkInstanceCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                pApplicationInfo: &ai,
                enabledLayerCount: INST_LAYER_NAMES.len() as u32,
                ppEnabledLayerNames: INST_LAYER_NAMES.as_ptr(),
                enabledExtensionCount: INST_EXT_NAMES.len() as u32,
                ppEnabledExtensionNames: INST_EXT_NAMES.as_ptr(),
            };
            let mut instance = null_mut();
            check!(
                vkCreateInstance(&ci, null(), &mut instance),
                "failed to create a Vulkan instance."
            );
            instance
        };

        let phys_device = {
            let mut cnt = 0;
            check!(
                vkEnumeratePhysicalDevices(instance, &mut cnt, null_mut()),
                "failed to get the number of physical devices."
            );
            let mut phys_devices = vec![null_mut(); cnt as usize];
            check!(
                vkEnumeratePhysicalDevices(instance, &mut cnt, phys_devices.as_mut_ptr()),
                "failed to enumerate physical devices."
            );
            // TODO: select a physical device better.
            phys_devices
                .get(0)
                .unwrap_or_else(|| ss_error("tried to get a physical device out of index."))
                .clone()
        };

        let phys_device_mem_props = {
            let mut props = VkPhysicalDeviceMemoryProperties::default();
            unsafe { vkGetPhysicalDeviceMemoryProperties(phys_device, &mut props) };
            props
        };

        let queue_family_index = {
            let mut cnt = 0;
            unsafe { vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &mut cnt, null_mut()) };
            let mut props = vec![VkQueueFamilyProperties::default(); cnt as usize];
            unsafe {
                vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &mut cnt, props.as_mut_ptr())
            };
            props
                .into_iter()
                .position(|n| (n.queueFlags & VkQueueFlagBits_VK_QUEUE_GRAPHICS_BIT as u32) > 0)
                .map(|i| i as u32)
                .unwrap_or_else(|| ss_error("failed to find a queue family index."))
        };

        let device = {
            let queue_cis = [VkDeviceQueueCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                queueFamilyIndex: queue_family_index,
                // REVIEW: should i changed:
                // REVIEW:   let queue_priorities = [1.0];
                // REVIEW:     queueCount: queue_priorities.len() as u32,
                // REVIEW:     pQueuePriorities: queue_priorities.as_ptr(),
                queueCount: 1,
                pQueuePriorities: [1.0].as_ptr(),
            }];
            let ci = VkDeviceCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                queueCreateInfoCount: 1,
                pQueueCreateInfos: queue_cis.as_ptr(),
                enabledLayerCount: 0,
                ppEnabledLayerNames: null(),
                enabledExtensionCount: DEVICE_EXT_NAMES.len() as u32,
                ppEnabledExtensionNames: DEVICE_EXT_NAMES.as_ptr(),
                pEnabledFeatures: null(),
            };
            let mut device = null_mut();
            check!(
                vkCreateDevice(phys_device, &ci, null(), &mut device),
                "failed to create a device."
            );
            device
        };

        // ========================================================================================================= //
        //     command                                                                                               //
        // ========================================================================================================= //

        let queue = {
            let mut queue = null_mut();
            unsafe { vkGetDeviceQueue(device, queue_family_index, 0, &mut queue) };
            queue
        };

        let command_pool = {
            let ci = VkCommandPoolCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                pNext: null(),
                flags: VkCommandPoolCreateFlagBits_VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
                    as u32,
                queueFamilyIndex: queue_family_index,
            };
            let mut command_pool = null_mut();
            check!(
                vkCreateCommandPool(device, &ci, null(), &mut command_pool),
                "failed to create a command pool."
            );
            command_pool
        };

        let command_buffer = {
            let ai = VkCommandBufferAllocateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                pNext: null(),
                commandPool: command_pool,
                level: VkCommandBufferLevel_VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                commandBufferCount: 1,
            };
            let mut command_buffer = null_mut();
            check!(
                vkAllocateCommandBuffers(device, &ai, &mut command_buffer),
                "failed to create a command buffer."
            );
            command_buffer
        };

        let fence = {
            let ci = VkFenceCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                pNext: null(),
                flags: VkFenceCreateFlagBits_VK_FENCE_CREATE_SIGNALED_BIT as VkFenceCreateFlags,
            };
            let mut fence = null_mut();
            check!(
                vkCreateFence(device, &ci, null(), &mut fence),
                "failed to create a fence."
            );
            fence
        };

        let (wait_semaphore, signal_semaphore) = {
            let ci = VkSemaphoreCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                pNext: null(),
                flags: 0,
            };
            let mut wait_semaphore = null_mut();
            check!(
                vkCreateSemaphore(device, &ci, null(), &mut wait_semaphore),
                "failed to create a semaphore."
            );
            let mut signal_semaphore = null_mut();
            check!(
                vkCreateSemaphore(device, &ci, null(), &mut signal_semaphore),
                "failed to create a semaphore."
            );
            (wait_semaphore, signal_semaphore)
        };

        // ========================================================================================================= //
        //     renderer                                                                                              //
        // ========================================================================================================= //

        #[cfg(target_os = "linux")]
        let surface = {
            let ci = VkXlibSurfaceCreateInfoKHR {
                sType: VkStructureType_VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
                pNext: null(),
                flags: 0,
                dpy: window_app.display,
                window: window_app.window,
            };
            let mut surface = null_mut();
            check!(
                vkCreateXlibSurfaceKHR(instance, &ci, null(), &mut surface),
                "failed to create a xlib surface."
            );
            surface
        };
        #[cfg(target_os = "windows")]
        let surface = {
            let ci = VkWin32SurfaceCreateInfoKHR {
                sType: VkStructureType_VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                pNext: null(),
                flags: 0,
                hinstance: window_app.inst,
                hwnd: window_app.window,
            };
            let mut surface = null_mut();
            check!(
                vkCreateWin32SurfaceKHR(instance, &ci, null(), &mut surface),
                "failed to create a win32 surface."
            );
            surface
        };

        let surface_format = {
            let mut cnt = 0;
            check!(
                vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &mut cnt, null_mut()),
                "failed to get the number of surface formats."
            );
            let mut formats = vec![VkSurfaceFormatKHR::default(); cnt as usize];
            check!(
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    phys_device,
                    surface,
                    &mut cnt,
                    formats.as_mut_ptr()
                ),
                "failed to get surface formats."
            );
            formats
                .into_iter()
                .find(|n| n.format == VkFormat_VK_FORMAT_B8G8R8A8_UNORM)
                .unwrap_or_else(|| ss_error("failed to get B8G8R8A8_UNORM surface format."))
        };

        let surface_capabilities = {
            let mut surface_capabilities = VkSurfaceCapabilitiesKHR::default();
            check!(
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                    phys_device,
                    surface,
                    &mut surface_capabilities,
                ),
                "failed to get surface capabilities."
            );
            surface_capabilities
        };

        let swapchain = {
            let min_image_count = std::cmp::max(surface_capabilities.minImageCount, 2);
            let ci = VkSwapchainCreateInfoKHR {
                sType: VkStructureType_VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                pNext: null(),
                flags: 0,
                surface,
                minImageCount: min_image_count,
                imageFormat: surface_format.format,
                imageColorSpace: surface_format.colorSpace,
                imageExtent: surface_capabilities.currentExtent,
                imageArrayLayers: 1,
                imageUsage: VkImageUsageFlagBits_VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT as u32,
                imageSharingMode: VkSharingMode_VK_SHARING_MODE_EXCLUSIVE,
                queueFamilyIndexCount: 0,
                pQueueFamilyIndices: null(),
                preTransform: surface_capabilities.currentTransform,
                compositeAlpha: VkCompositeAlphaFlagBitsKHR_VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                presentMode: VkPresentModeKHR_VK_PRESENT_MODE_FIFO_KHR,
                clipped: VK_TRUE,
                oldSwapchain: null_mut(),
            };
            let mut swapchain = null_mut();
            check!(
                vkCreateSwapchainKHR(device, &ci, null(), &mut swapchain),
                "failed to create a swapchain."
            );
            swapchain
        };

        let image_views = {
            let mut cnt = 0;
            check!(
                vkGetSwapchainImagesKHR(device, swapchain, &mut cnt, null_mut()),
                "failed to get the number of swapchain images."
            );
            let mut images = vec![null_mut(); cnt as usize];
            check!(
                vkGetSwapchainImagesKHR(device, swapchain, &mut cnt, images.as_mut_ptr()),
                "failed to get the number of swapchain images."
            );
            let mut ci = VkImageViewCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                pNext: null(),
                flags: 0,
                image: null_mut(),
                viewType: VkImageViewType_VK_IMAGE_VIEW_TYPE_2D,
                format: surface_format.format,
                components: VkComponentMapping {
                    r: VkComponentSwizzle_VK_COMPONENT_SWIZZLE_R,
                    g: VkComponentSwizzle_VK_COMPONENT_SWIZZLE_G,
                    b: VkComponentSwizzle_VK_COMPONENT_SWIZZLE_B,
                    a: VkComponentSwizzle_VK_COMPONENT_SWIZZLE_A,
                },
                subresourceRange: VkImageSubresourceRange {
                    aspectMask: VkImageAspectFlagBits_VK_IMAGE_ASPECT_COLOR_BIT as u32,
                    baseMipLevel: 0,
                    levelCount: 1,
                    baseArrayLayer: 0,
                    layerCount: 1,
                },
            };
            let mut image_views = Vec::with_capacity(cnt as usize);
            for image in images {
                ci.image = image;
                let mut image_view = null_mut();
                check!(
                    vkCreateImageView(device, &ci, null(), &mut image_view),
                    "failed to create an image view."
                );
                image_views.push(image_view);
            }
            image_views
        };

        let (render_pass, attachments_count) = {
            let attachment_descs = [VkAttachmentDescription {
                flags: 0,
                format: surface_format.format,
                samples: VkSampleCountFlagBits_VK_SAMPLE_COUNT_1_BIT,
                loadOp: VkAttachmentLoadOp_VK_ATTACHMENT_LOAD_OP_CLEAR,
                storeOp: VkAttachmentStoreOp_VK_ATTACHMENT_STORE_OP_STORE,
                stencilLoadOp: VkAttachmentLoadOp_VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                stencilStoreOp: VkAttachmentStoreOp_VK_ATTACHMENT_STORE_OP_DONT_CARE,
                initialLayout: VkImageLayout_VK_IMAGE_LAYOUT_UNDEFINED,
                finalLayout: VkImageLayout_VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            }];
            let color_refs = [VkAttachmentReference {
                attachment: 0,
                layout: VkImageLayout_VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            }];
            let subpass_descs = [VkSubpassDescription {
                flags: 0,
                pipelineBindPoint: VkPipelineBindPoint_VK_PIPELINE_BIND_POINT_GRAPHICS,
                inputAttachmentCount: 0,
                pInputAttachments: null(),
                colorAttachmentCount: color_refs.len() as u32,
                pColorAttachments: color_refs.as_ptr(),
                pResolveAttachments: null(),
                pDepthStencilAttachment: null(),
                preserveAttachmentCount: 0,
                pPreserveAttachments: null(),
            }];
            let ci = VkRenderPassCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                pNext: null(),
                flags: 0,
                attachmentCount: attachment_descs.len() as u32,
                pAttachments: attachment_descs.as_ptr(),
                subpassCount: subpass_descs.len() as u32,
                pSubpasses: subpass_descs.as_ptr(),
                dependencyCount: 0,
                pDependencies: null(),
            };
            let mut render_pass = null_mut();
            check!(
                vkCreateRenderPass(device, &ci, null(), &mut render_pass),
                "failed to create a render pass."
            );
            (render_pass, attachment_descs.len() as u32)
        };

        let framebuffers = {
            let mut ci = VkFramebufferCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                pNext: null(),
                flags: 0,
                renderPass: render_pass,
                attachmentCount: attachments_count,
                pAttachments: null(),
                width: surface_capabilities.currentExtent.width,
                height: surface_capabilities.currentExtent.height,
                layers: 1,
            };
            let mut framebuffers = Vec::with_capacity(image_views.len());
            for image_view in image_views.iter() {
                let attachments = [image_view.clone()];
                ci.pAttachments = attachments.as_ptr();
                let mut framebuffer = null_mut();
                check!(
                    vkCreateFramebuffer(device, &ci, null(), &mut framebuffer),
                    "failed to create a framebuffer."
                );
                framebuffers.push(framebuffer);
            }
            framebuffers
        };

        // ========================================================================================================= //
        //     pipeline                                                                                              //
        // ========================================================================================================= //

        let vert_shader = {
            let bin = get_vertex_shader_binary();
            let ci = VkShaderModuleCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                codeSize: bin.len(),
                pCode: bin.as_ptr() as *const u32,
            };
            let mut shader = null_mut();
            check!(
                vkCreateShaderModule(device, &ci, null(), &mut shader),
                &format!("failed to create vertex shader.")
            );
            shader
        };

        let frag_shader = {
            let bin = get_fragment_shader_binary();
            let ci = VkShaderModuleCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                codeSize: bin.len(),
                pCode: bin.as_ptr() as *const u32,
            };
            let mut shader = null_mut();
            check!(
                vkCreateShaderModule(device, &ci, null(), &mut shader),
                &format!("failed to create fragment shader.")
            );
            shader
        };

        let sampler = {
            let ci = VkSamplerCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                pNext: null(),
                flags: 0,
                magFilter: VkFilter_VK_FILTER_NEAREST,
                minFilter: VkFilter_VK_FILTER_LINEAR,
                mipmapMode: VkSamplerMipmapMode_VK_SAMPLER_MIPMAP_MODE_NEAREST,
                addressModeU: VkSamplerAddressMode_VK_SAMPLER_ADDRESS_MODE_REPEAT,
                addressModeV: VkSamplerAddressMode_VK_SAMPLER_ADDRESS_MODE_REPEAT,
                addressModeW: VkSamplerAddressMode_VK_SAMPLER_ADDRESS_MODE_REPEAT,
                mipLodBias: 0.0,
                anisotropyEnable: 0,
                maxAnisotropy: 1.0,
                compareEnable: 0,
                compareOp: VkCompareOp_VK_COMPARE_OP_NEVER,
                minLod: 0.0,
                maxLod: 0.0,
                borderColor: VkBorderColor_VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
                unnormalizedCoordinates: 0,
            };
            let mut sampler = null_mut();
            check!(
                vkCreateSampler(device, &ci, null(), &mut sampler),
                "failed to create a sampler."
            );
            sampler
        };

        let (descriptor_set_layout, descriptor_pool) = {
            // layout
            let binds = [
                VkDescriptorSetLayoutBinding {
                    binding: 0,
                    descriptorType: VkDescriptorType_VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    descriptorCount: 1,
                    stageFlags: VkShaderStageFlagBits_VK_SHADER_STAGE_VERTEX_BIT
                        as VkShaderStageFlags,
                    pImmutableSamplers: null(),
                },
                VkDescriptorSetLayoutBinding {
                    binding: 1,
                    descriptorType: VkDescriptorType_VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descriptorCount: 1,
                    stageFlags: VkShaderStageFlagBits_VK_SHADER_STAGE_FRAGMENT_BIT
                        as VkShaderStageFlags,
                    pImmutableSamplers: null(),
                },
            ];
            let ci = VkDescriptorSetLayoutCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                pNext: null(),
                flags: 0,
                bindingCount: binds.len() as u32,
                pBindings: binds.as_ptr(),
            };
            let mut descriptor_set_layout = null_mut();
            check!(
                vkCreateDescriptorSetLayout(device, &ci, null(), &mut descriptor_set_layout),
                "failed to create a descriptor set layout."
            );

            // pool
            let sizes = binds
                .iter()
                .map(|n| VkDescriptorPoolSize {
                    type_: n.descriptorType,
                    descriptorCount: max_img_tex_cnt,
                })
                .collect::<Vec<VkDescriptorPoolSize>>();
            let ci = VkDescriptorPoolCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                pNext: null(),
                flags: 0,
                maxSets: max_img_tex_cnt,
                poolSizeCount: sizes.len() as u32,
                pPoolSizes: sizes.as_ptr(),
            };
            let mut descriptor_pool = null_mut();
            check!(
                vkCreateDescriptorPool(device, &ci, null(), &mut descriptor_pool),
                "failed to create a descriptor pool."
            );

            // finish
            (descriptor_set_layout, descriptor_pool)
        };

        let descriptor_sets = {
            let ai = VkDescriptorSetAllocateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                pNext: null(),
                descriptorPool: descriptor_pool,
                descriptorSetCount: 1,
                // REVIEW: should i make [descriptor_set_layout]
                pSetLayouts: &descriptor_set_layout,
            };
            let mut descriptor_sets = Vec::with_capacity(max_img_tex_cnt as usize);
            for _ in 0..max_img_tex_cnt {
                let mut descriptor_set = null_mut();
                check!(
                    vkAllocateDescriptorSets(device, &ai, &mut descriptor_set),
                    "failed to allocate descriptor sets."
                );
                descriptor_sets.push(descriptor_set);
            }
            descriptor_sets
        };

        let pipeline_layout = {
            let ranges = [VkPushConstantRange {
                stageFlags: VkShaderStageFlagBits_VK_SHADER_STAGE_VERTEX_BIT as VkShaderStageFlags,
                offset: 0,
                size: size_of::<PushConstant>() as u32,
            }];
            let ci = VkPipelineLayoutCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                pNext: null(),
                flags: 0,
                // REVIEW: should i change:
                // REVIEW:   [descriptor_set_layout]
                // REVIEW:     setLayoutCount: descriptor_set_layouts.len() as u32,
                // REVIEW:     pSetLayouts: descriptor_set_layouts.as_ptr(),
                setLayoutCount: 1,
                pSetLayouts: &descriptor_set_layout,
                pushConstantRangeCount: ranges.len() as u32,
                pPushConstantRanges: ranges.as_ptr(),
            };
            let mut pipeline_layout = null_mut();
            check!(
                vkCreatePipelineLayout(device, &ci, null(), &mut pipeline_layout),
                "failed to create a pipeline layout."
            );
            pipeline_layout
        };

        let pipeline = {
            // shader stage
            let shaders = [
                VkPipelineShaderStageCreateInfo {
                    sType: VkStructureType_VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    pNext: null(),
                    flags: 0,
                    stage: VkShaderStageFlagBits_VK_SHADER_STAGE_VERTEX_BIT,
                    module: vert_shader,
                    pName: SHADER_ENTRY_NAME,
                    pSpecializationInfo: null(),
                },
                VkPipelineShaderStageCreateInfo {
                    sType: VkStructureType_VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    pNext: null(),
                    flags: 0,
                    stage: VkShaderStageFlagBits_VK_SHADER_STAGE_FRAGMENT_BIT,
                    module: frag_shader,
                    pName: SHADER_ENTRY_NAME,
                    pSpecializationInfo: null(),
                },
            ];

            // vertex input state
            let input_bind_dcs = [VkVertexInputBindingDescription {
                binding: 0,
                stride: size_of::<Vertex>() as u32,
                inputRate: VkVertexInputRate_VK_VERTEX_INPUT_RATE_VERTEX,
            }];
            let input_attr_dcs = [
                VkVertexInputAttributeDescription {
                    location: 0,
                    binding: 0,
                    format: VkFormat_VK_FORMAT_R32G32B32_SFLOAT,
                    offset: 0,
                },
                VkVertexInputAttributeDescription {
                    location: 1,
                    binding: 0,
                    format: VkFormat_VK_FORMAT_R32G32_SFLOAT,
                    offset: size_of::<f32>() as u32 * 3,
                },
            ];
            let input = VkPipelineVertexInputStateCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                vertexBindingDescriptionCount: input_bind_dcs.len() as u32,
                pVertexBindingDescriptions: input_bind_dcs.as_ptr(),
                vertexAttributeDescriptionCount: input_attr_dcs.len() as u32,
                pVertexAttributeDescriptions: input_attr_dcs.as_ptr(),
            };

            // input assembly state
            let inp_as = VkPipelineInputAssemblyStateCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                topology: VkPrimitiveTopology_VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                primitiveRestartEnable: VK_FALSE,
            };

            // viewport state
            let viewports = [VkViewport {
                x: 0.0,
                y: 0.0,
                width: surface_capabilities.currentExtent.width as f32,
                height: surface_capabilities.currentExtent.height as f32,
                minDepth: 0.0,
                maxDepth: 1.0,
            }];
            let scissors = [VkRect2D {
                offset: VkOffset2D { x: 0, y: 0 },
                extent: surface_capabilities.currentExtent,
            }];
            let viewport = VkPipelineViewportStateCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                viewportCount: viewports.len() as u32,
                pViewports: viewports.as_ptr(),
                scissorCount: scissors.len() as u32,
                pScissors: scissors.as_ptr(),
            };

            // rasterization state
            let raster = VkPipelineRasterizationStateCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                depthClampEnable: VK_FALSE,
                rasterizerDiscardEnable: VK_FALSE,
                polygonMode: VkPolygonMode_VK_POLYGON_MODE_FILL,
                cullMode: VkCullModeFlagBits_VK_CULL_MODE_NONE as VkCullModeFlags,
                frontFace: VkFrontFace_VK_FRONT_FACE_COUNTER_CLOCKWISE,
                depthBiasEnable: VK_FALSE,
                depthBiasConstantFactor: 0.0,
                depthBiasClamp: 0.0,
                depthBiasSlopeFactor: 0.0,
                lineWidth: 1.0,
            };

            // multisample state
            let multisample = VkPipelineMultisampleStateCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                rasterizationSamples: VkSampleCountFlagBits_VK_SAMPLE_COUNT_1_BIT,
                sampleShadingEnable: VK_FALSE,
                minSampleShading: 0.0,
                pSampleMask: null(),
                alphaToCoverageEnable: VK_FALSE,
                alphaToOneEnable: VK_FALSE,
            };

            // color blend state
            let color_write_mask = VkColorComponentFlagBits_VK_COLOR_COMPONENT_R_BIT
                | VkColorComponentFlagBits_VK_COLOR_COMPONENT_G_BIT
                | VkColorComponentFlagBits_VK_COLOR_COMPONENT_B_BIT
                | VkColorComponentFlagBits_VK_COLOR_COMPONENT_A_BIT;
            let color_blend_attachments = [VkPipelineColorBlendAttachmentState {
                blendEnable: VK_TRUE,
                srcColorBlendFactor: VkBlendFactor_VK_BLEND_FACTOR_SRC_ALPHA,
                dstColorBlendFactor: VkBlendFactor_VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                colorBlendOp: VkBlendOp_VK_BLEND_OP_ADD,
                srcAlphaBlendFactor: VkBlendFactor_VK_BLEND_FACTOR_SRC_ALPHA,
                dstAlphaBlendFactor: VkBlendFactor_VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                alphaBlendOp: VkBlendOp_VK_BLEND_OP_ADD,
                colorWriteMask: color_write_mask as VkColorComponentFlags,
            }];
            let color_blend = VkPipelineColorBlendStateCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                logicOpEnable: VK_FALSE,
                logicOp: 0,
                attachmentCount: color_blend_attachments.len() as u32,
                pAttachments: color_blend_attachments.as_ptr(),
                blendConstants: [0.0; 4],
            };

            // pipeline
            let cis = [VkGraphicsPipelineCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                stageCount: shaders.len() as u32,
                pStages: shaders.as_ptr(),
                pVertexInputState: &input,
                pInputAssemblyState: &inp_as,
                pTessellationState: null(),
                pViewportState: &viewport,
                pRasterizationState: &raster,
                pMultisampleState: &multisample,
                pDepthStencilState: null(),
                pColorBlendState: &color_blend,
                pDynamicState: null(),
                layout: pipeline_layout,
                renderPass: render_pass,
                subpass: 0,
                basePipelineHandle: null_mut(),
                basePipelineIndex: 0,
            }];
            let mut pipeline = null_mut();
            check!(
                vkCreateGraphicsPipelines(
                    device,
                    null_mut(),
                    cis.len() as u32,
                    cis.as_ptr(),
                    null(),
                    &mut pipeline
                ),
                "failed to create a pipeline."
            );
            pipeline
        };

        // ========================================================================================================= //
        //     resources                                                                                             //
        // ========================================================================================================= //

        let uniform_buffer = {
            let data = UniformBuffer {
                view: create_view(
                    [
                        0.0,
                        0.0,
                        surface_capabilities.currentExtent.width as f32 / -2.0,
                    ],
                    [0.0; 3],
                ),
                perse: create_perse(
                    45.0,
                    surface_capabilities.currentExtent.width as f32
                        / surface_capabilities.currentExtent.height as f32,
                    100.0,
                    1000.0,
                ),
                ortho: create_ortho(
                    surface_capabilities.currentExtent.width as f32,
                    surface_capabilities.currentExtent.height as f32,
                    1000.0,
                ),
            };
            let uniform_buffer_flags = VkMemoryPropertyFlagBits_VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                | VkMemoryPropertyFlagBits_VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            let uniform_buffer = Buffer::new(
                device,
                &phys_device_mem_props,
                size_of::<UniformBuffer>() as VkDeviceSize,
                VkBufferUsageFlagBits_VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT as VkBufferUsageFlags,
                uniform_buffer_flags as VkMemoryPropertyFlags,
            )
            .unwrap_or_else(|e| ss_error(&format!("failed to create a uniform buffer : {e}")));
            map_memory(
                device,
                uniform_buffer.memory,
                &data as *const _ as *const c_void,
                size_of::<UniformBuffer>(),
            )
            .unwrap_or_else(|e| ss_error(&format!("failed to map a uniform buffer : {e}")));
            uniform_buffer
        };

        let square = Model::new(device, &phys_device_mem_props, &SQUARE_VTXS, &SQUARE_IDXS)
            .unwrap_or_else(|e| ss_error(&format!("failed to create a square : {e}")));

        let def_img_tex = {
            let usage = VkImageUsageFlagBits_VK_IMAGE_USAGE_TRANSFER_DST_BIT
                | VkImageUsageFlagBits_VK_IMAGE_USAGE_SAMPLED_BIT;
            let texture = Texture::new(
                device,
                &phys_device_mem_props,
                VkFormat_VK_FORMAT_R8G8B8A8_UNORM,
                DEF_IMG_TEX_WIDTH,
                DEF_IMG_TEX_HEIGHT,
                usage as VkImageUsageFlags,
                VkImageAspectFlagBits_VK_IMAGE_ASPECT_COLOR_BIT as VkImageAspectFlags,
            )
            .unwrap_or_else(|e| {
                ss_error(&format!("failed to create a default image texture : {e}"))
            });
            copy_memory(
                device,
                &phys_device_mem_props,
                queue,
                command_pool,
                DEF_IMG_TEX_WIDTH,
                DEF_IMG_TEX_HEIGHT,
                texture.image,
                DEF_IMG_TEX_PIXELS_SIZE,
                DEF_IMG_TEX_PIXELS.as_ptr() as *const c_void,
            )
            .unwrap_or_else(|e| {
                ss_error(&format!(
                    "failed to copy a default image texture data : {e}"
                ))
            });
            texture
        };

        update_descriptor_set(
            device,
            descriptor_sets[0],
            uniform_buffer.buffer,
            sampler,
            def_img_tex.image_view,
        );

        let img_texs = HashMap::from([(DEFAULT_IMAGE_TEXTURE_ID, (0, def_img_tex))]);
        let unused_img_tex_idxs = Vec::new();

        // ========================================================================================================= //
        //     finish                                                                                                //
        // ========================================================================================================= //

        Self {
            // core
            instance,
            phys_device_mem_props,
            device,
            // command
            queue,
            command_pool,
            command_buffer,
            fence,
            wait_semaphore,
            signal_semaphore,
            // renderer
            surface,
            surface_capabilities,
            swapchain,
            image_views,
            render_pass,
            framebuffers,
            // pipeline
            vert_shader,
            frag_shader,
            sampler,
            pipeline_layout,
            pipeline,
            // descriptor set
            descriptor_set_layout,
            descriptor_pool,
            descriptor_sets,
            // resources
            uniform_buffer,
            square,
            img_texs,
            unused_img_tex_idxs,
        }
    }
}
