mod log;
#[allow(non_upper_case_globals)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
mod tpl;
mod vulkan;
mod window;

fn main() {
    let window_app = window::WindowApp::new("Sample Program\0", 640, 480);
    let mut vulkan_app = vulkan::VulkanApp::new(&window_app);
    while window_app.do_events() {
        vulkan_app.render();
    }
    vulkan_app.terminate();
    window_app.terminate();
}
