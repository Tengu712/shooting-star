use crate::log::*;
use crate::tpl::*;

use std::collections::HashMap;
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
mod common;
pub mod math;
mod new;
mod obj;
mod render;
mod terminate;
mod texture;

use additions::*;
use common::*;
use obj::{buffer::*, model::*, texture::*};

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
    descriptor_sets: Vec<VkDescriptorSet>,
    // resources
    uniform_buffer: Buffer,
    square: Model,
    /// A hashmap:
    ///   - whose key is an image texture id
    ///   - whose value is (descriptor sets index, image texture)
    img_texs: HashMap<usize, (usize, Texture)>,
    unused_img_tex_idxs: Vec<usize>,
}

#[repr(C)]
#[derive(Clone)]
pub struct PushConstant {
    pub scl: [f32; 4],
    pub rot: [f32; 4],
    pub trs: [f32; 4],
    pub col: [f32; 4],
    pub uv: [f32; 4],
    pub param: [i32; 4],
}
impl Default for PushConstant {
    fn default() -> Self {
        Self {
            scl: [1.0, 1.0, 1.0, 0.0],
            rot: [0.0; 4],
            trs: [0.0; 4],
            col: [1.0; 4],
            uv: [0.0, 0.0, 1.0, 1.0],
            param: [0; 4],
        }
    }
}

#[repr(C)]
pub struct UniformBuffer {
    pub view: [f32; 16],
    pub perse: [f32; 16],
    pub ortho: [f32; 16],
}

pub enum RenderTask {
    Draw(PushConstant),
    SetImageTexture(usize),
}

pub const DEFAULT_IMAGE_TEXTURE_ID: usize = 0;
