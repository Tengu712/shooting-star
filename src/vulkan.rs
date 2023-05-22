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

mod additions;
mod new;
mod terminate;

pub struct VulkanApp {
    instance: VkInstance,
    phys_device_mem_props: VkPhysicalDeviceMemoryProperties,
    device: VkDevice,
    command_pool: VkCommandPool,
    surface: VkSurfaceKHR,
    swapchain: VkSwapchainKHR,
    image_views: Vec<VkImageView>,
    render_pass: VkRenderPass,
    framebuffers: Vec<VkFramebuffer>,
}
