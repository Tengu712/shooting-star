mod general;
pub mod graphics;
mod input;
mod load;

use crate::{vulkan::*, window::*};

use std::collections::HashMap;

struct TexPartInfo {
    width: f32,
    height: f32,
    uv: [f32; 4],
}

pub struct SStarApp {
    settings: HashMap<String, String>,
    /// A constant for adjusting the value of PushConstants based on the runtime scene size.
    /// It is 1.0 when the scene size is 1280x960.
    scene_scale: f32,
    base_width: f32,
    base_height: f32,
    window_app: WindowApp,
    vulkan_app: VulkanApp,
    /// <texture id, <texture fragment id, texture fragment uv>>
    uv_infos: HashMap<usize, HashMap<String, TexPartInfo>>,
    /// A hashmap to get a user configured joystick keycode from a keyboard keycode.
    js_map: HashMap<Keycode, Keycode>,
    /// It is passed to the VulkanApp::render method.
    /// It is set to None every frame.
    ub: Option<UniformBuffer>,
    /// It is passed to the VulkanApp::render method.
    /// It is cleared every frame.
    tasks: Vec<RenderTask>,
}
