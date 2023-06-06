use sstar::{vulkan::*, window::*};

fn main() {
    let mut window_app = WindowApp::new("Sample Program\0", 640, 480);
    let vulkan_app = VulkanApp::new(&window_app, 10);
    while window_app.do_events() {
        vulkan_app.render(None, &[]).unwrap();
    }
    vulkan_app.terminate();
    window_app.terminate();
}
