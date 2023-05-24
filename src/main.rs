mod log;
#[allow(non_upper_case_globals)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
mod tpl;
mod vulkan;
mod window;

use log::*;

fn main() {
    let window_app = window::WindowApp::new("Sample Program\0", 640, 480);
    let vulkan_app = vulkan::VulkanApp::new(&window_app, 2);
    while window_app.do_events() {
        if let Err(e) = vulkan_app.render() {
            ss_warning(&e);
        }
    }
    vulkan_app.terminate();
    window_app.terminate();
}
