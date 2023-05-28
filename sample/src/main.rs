use sstar::{
    log::*,
    vulkan::{image::*, *},
    window::*,
};

#[inline(always)]
fn check(p: Result<(), String>) {
    match p {
        Ok(()) => (),
        Err(e) => ss_warning(&e),
    }
}

fn main() {
    let mut window_app = WindowApp::new("Sample Program\0", 640, 480);
    let mut vulkan_app = VulkanApp::new(&window_app, 10);
    check(load_image_texture(
        &mut vulkan_app,
        1,
        2,
        2,
        &[255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255],
    ));

    let mut cnt = 0;
    while window_app.do_events() {
        let enter = window_app.get_input(Keycode::Enter);
        if enter > 0 {
            ss_debug(&format!("enter {enter}"));
        }
        let js_a = window_app.get_input(Keycode::JsButtonA);
        if js_a > 0 {
            ss_debug(&format!("js a {js_a}"));
        }
        let js_laxis = window_app.get_axis_input(Keycode::JsAxisLX);
        if js_laxis.abs() > 0.1 {
            ss_debug(&format!("js left axis left {js_laxis}"));
        }
        let mut pc1 = PushConstant::default();
        pc1.scl = [100.0, 100.0, 1.0, 0.0];
        let mut pc2 = pc1.clone();
        pc2.trs = [100.0, 100.0, 0.0, 0.0];
        pc2.rot[0] = cnt as f32 / 180.0 * std::f32::consts::PI;
        check(vulkan_app.render(
            None,
            &[
                RenderTask::SetImageTexture(1),
                RenderTask::Draw(pc1),
                RenderTask::SetImageTexture(0),
                RenderTask::Draw(pc2),
            ],
        ));
        cnt += 1;
    }
    vulkan_app.terminate();
    window_app.terminate();
}
