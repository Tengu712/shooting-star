use sstar::{
    font::*,
    log::*,
    vulkan::{image::*, *},
    window::*,
};

#[inline(always)]
fn check(p: Result<(), String>) {
    match p {
        Ok(n) => n,
        Err(e) => ss_warning(&e),
    }
}

fn main() {
    // create sstar objects
    let mut window_app = WindowApp::new("Sample Program\0", 640, 480);
    let mut vulkan_app = VulkanApp::new(&window_app, 10);

    // create and load an image texture
    check(load_image_texture(
        &mut vulkan_app,
        1,
        2,
        2,
        &[
            255, 0, 0, 255, // red (upper left)
            0, 255, 0, 255, // green (upper right)
            0, 0, 255, 255, // blue (lower left)
            0, 0, 0, 255, // black (lower right)
        ],
    ));

    // rasterize a text
    let rasterizer = GlyphRasterizer::new("./ipaexm.ttf").unwrap();
    let (pxs, w, h) = rasterizer.rasterize("Hello 世界!", 32.0);
    // create pixels and map the text
    let text_pixels_width = 2_usize.pow((w as f64).log2().ceil() as u32);
    let text_pixels_height = 2_usize.pow((h as f64).log2().ceil() as u32);
    let mut pixels_text = vec![0; text_pixels_width * text_pixels_height * 4];
    for j in 0..h {
        for i in 0..w {
            let idx = j * text_pixels_width + i;
            pixels_text[idx * 4 + 0] = 255;
            pixels_text[idx * 4 + 1] = 255;
            pixels_text[idx * 4 + 2] = 255;
            pixels_text[idx * 4 + 3] = pxs[j * w + i];
        }
    }
    // create and load an image texture of the text
    check(load_image_texture(
        &mut vulkan_app,
        2,
        text_pixels_width as u32,
        text_pixels_height as u32,
        &pixels_text,
    ));

    // mainloop
    let mut cnt = 0;
    while window_app.do_events() {
        // get and check inputs
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

        // push constant for a rotating square
        let mut pc_rotating = PushConstant::default();
        pc_rotating.scl = [100.0, 100.0, 1.0, 0.0];
        pc_rotating.trs = [100.0, 100.0, 0.0, 0.0];
        pc_rotating.rot[0] = cnt as f32 / 180.0 * std::f32::consts::PI;

        // push constant for a text
        let mut pc_text = PushConstant::default();
        pc_text.scl = [
            text_pixels_width as f32,
            text_pixels_height as f32,
            1.0,
            0.0,
        ];

        // render
        check(vulkan_app.render(
            None,
            &[
                RenderTask::SetImageTexture(1),
                RenderTask::Draw(pc_rotating),
                RenderTask::SetImageTexture(2),
                RenderTask::Draw(pc_text),
            ],
        ));

        // next loop
        cnt += 1;
    }

    // terminate
    vulkan_app.terminate();
    window_app.terminate();
}
