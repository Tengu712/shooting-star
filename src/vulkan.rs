use crate::log::*;
use crate::tpl::*;

use std::os::raw::{c_ulong, c_void};
use std::ptr::{null, null_mut};

macro_rules! check {
    ($p: expr, $m: expr) => {
        if unsafe { $p != VkResult_VK_SUCCESS } {
            ss_error($m);
        }
    };
}

macro_rules! check_res {
    ($p: expr, $m: expr) => {
        #[allow(unused_unsafe)]
        if unsafe { $p != VkResult_VK_SUCCESS } {
            return Err(String::from($m));
        }
    };
}

mod additions;
mod buffer;
mod model;
mod new;
mod render;
mod terminate;

use additions::*;
use buffer::*;
use model::*;

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
    wait_semaphore: VkSemaphore,
    signal_semaphore: VkSemaphore,
    // renderer
    surface: VkSurfaceKHR,
    surface_capabilities: VkSurfaceCapabilitiesKHR,
    swapchain: VkSwapchainKHR,
    image_views: Vec<VkImageView>,
    render_pass: VkRenderPass,
    framebuffers: Vec<VkFramebuffer>,
    // pipeline
    vert_shader: VkShaderModule,
    frag_shader: VkShaderModule,
    sampler: VkSampler,
    pipeline_layout: VkPipelineLayout,
    pipeline: VkPipeline,
    // descriptor set
    descriptor_set_layout: VkDescriptorSetLayout,
    descriptor_pool: VkDescriptorPool,
    descriptor_set: VkDescriptorSet,
    // resources
    uniform_buffer: Buffer,
    square: Model,
}

#[repr(C)]
pub struct PushConstant {
    pub scl: [f32; 4],
    pub rot: [f32; 4],
    pub trs: [f32; 4],
    pub col: [f32; 4],
    pub uv: [f32; 4],
    pub param: i32,
}

#[repr(C)]
pub struct UniformBuffer {
    pub view: [f32; 16],
    pub perse: [f32; 16],
    pub ortho: [f32; 16],
}

#[repr(C)]
struct Vertex {
    in_pos: [f32; 3],
    in_uv: [f32; 2],
}

pub fn create_perse(pov: f32, aspect: f32, near: f32, far: f32) -> [f32; 16] {
    let div_tanpov = 1.0 / f32::tan(std::f32::consts::PI * pov / 180.0);
    let div_depth = 1.0 / (far - near);
    [
        div_tanpov,
        0.0,
        0.0,
        0.0,
        0.0,
        div_tanpov * aspect,
        0.0,
        0.0,
        0.0,
        0.0,
        far * div_depth,
        1.0,
        0.0,
        0.0,
        -far * near * div_depth,
        0.0,
    ]
}

pub fn create_ortho(width: f32, height: f32, depth: f32) -> [f32; 16] {
    [
        1.0 / width,
        0.0,
        0.0,
        0.0,
        0.0,
        1.0 / height,
        0.0,
        0.0,
        0.0,
        0.0,
        1.0 / depth,
        0.0,
        0.0,
        0.0,
        0.0,
        1.0,
    ]
}

fn get_memory_type_index(
    phys_device_mem_prop: VkPhysicalDeviceMemoryProperties,
    reqs: &VkMemoryRequirements,
    flags: VkMemoryPropertyFlags,
) -> Option<u32> {
    phys_device_mem_prop
        .memoryTypes
        .iter()
        .enumerate()
        .position(|(i, n)| reqs.memoryTypeBits & (1 << i) > 0 && n.propertyFlags & flags > 0)
        .map(|i| i as u32)
}

fn map_memory(
    device: VkDevice,
    memory: VkDeviceMemory,
    data: *const c_void,
    size: c_ulong,
) -> Result<(), String> {
    let mut p = null_mut();
    check_res!(
        vkMapMemory(device, memory, 0, u64::MAX, 0, &mut p),
        "failed to map a memory."
    );
    unsafe { memcpy(p, data, size) };
    unsafe { vkUnmapMemory(device, memory) };
    Ok(())
}
