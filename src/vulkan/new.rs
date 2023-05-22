use super::additions::*;
use super::*;

use crate::window::WindowApp;

use std::os::raw::c_char;

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

impl VulkanApp {
    pub fn new(window_app: &WindowApp) -> Self {
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
                .enumerate()
                .find(|(_, prop)| {
                    (prop.queueFlags & VkQueueFlagBits_VK_QUEUE_GRAPHICS_BIT as u32) > 0
                })
                .map(|(i, _)| i as u32)
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

        let frames = {
            let cmd_ai = VkCommandBufferAllocateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                pNext: null(),
                commandPool: command_pool,
                level: VkCommandBufferLevel_VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                commandBufferCount: 1,
            };
            let fence_ci = VkFenceCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                pNext: null(),
                flags: VkFenceCreateFlagBits_VK_FENCE_CREATE_SIGNALED_BIT,
            };
            let semaphore_ci = VkSemaphoreCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                pNext: null(),
                flags: 0,
            };
            let mut frames = Vec::with_capacity(image_views.len());
            for _ in 0..image_views.len() {
                let mut command_buffer = null_mut();
                let mut fence = null_mut();
                let mut semaphore = null_mut();
                check!(
                    vkAllocateCommandBuffers(device, &cmd_ai, &mut command_buffer),
                    "failed to create a command buffer."
                );
                check!(
                    vkCreateFence(device, &fence_ci, null(), &mut fence),
                    "failed to create a fence."
                );
                check!(
                    vkCreateSemaphore(device, &semaphore_ci, null(), &mut semaphore),
                    "failed to create a semaphore."
                );
                frames.push(Frame {
                    command_buffer,
                    fence,
                    semaphore,
                });
            }
            frames
        };

        Self {
            instance,
            phys_device_mem_props,
            device,
            queue,
            command_pool,
            surface,
            surface_capabilities,
            swapchain,
            image_views,
            render_pass,
            framebuffers,
            pre_img_idx: 0,
            frames,
        }
    }
}
