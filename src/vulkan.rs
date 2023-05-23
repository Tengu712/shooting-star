use crate::log::*;
use crate::tpl::*;
use std::ptr::{null, null_mut};

macro_rules! check {
    ($p: expr, $m: expr) => {
        if unsafe { $p != VkResult_VK_SUCCESS } {
            ss_error($m);
        }
    };
}

macro_rules! check_warn {
    ($p: expr, $m: expr) => {
        if unsafe { $p != VkResult_VK_SUCCESS } {
            ss_warning($m);
            return;
        }
    };
}

mod additions;
mod new;
mod render;
mod terminate;

pub struct VulkanApp {
    // core
    instance: VkInstance,
    phys_device_mem_props: VkPhysicalDeviceMemoryProperties,
    device: VkDevice,
    // command
    queue: VkQueue,
    command_pool: VkCommandPool,
    command_buffer: VkCommandBuffer,
    fence: VkFence,
    before_semaphore: VkSemaphore,
    complete_semaphore: VkSemaphore,
    // renderer
    surface: VkSurfaceKHR,
    surface_capabilities: VkSurfaceCapabilitiesKHR,
    swapchain: VkSwapchainKHR,
    image_views: Vec<VkImageView>,
    render_pass: VkRenderPass,
    framebuffers: Vec<VkFramebuffer>,
    // pipeline
}
