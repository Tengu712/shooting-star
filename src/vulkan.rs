use crate::log::*;
use crate::tpl::*;
use std::ptr::{null, null_mut};

macro_rules! check {
    ($p: expr, $m: expr) => {
        if !($p) {
            ss_error($m);
        }
    };
}
macro_rules! check_vk {
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
    phys_device_mem_prop: VkPhysicalDeviceMemoryProperties,
    device: VkDevice,
}
