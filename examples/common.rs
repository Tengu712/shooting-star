use sstar::{
    bitmap::{font::*, image::*},
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
    let image_bitmap = create_bitmap_from_file("./examples/image.png").unwrap();
    check(load_image_texture(
        &mut vulkan_app,
        1,
        image_bitmap.width,
        image_bitmap.height,
        &image_bitmap.data,
    ));

    // rasterize a text
    let rasterizer = GlyphRasterizer::new("./examples/mplus-2p-medium.ttf").unwrap();
    let text_bitmap = rasterizer.rasterize(
        "L'enfer est plein de bonnes volontés ou désirs.\n地獄への道は善意で舗装されている。",
        32.0,
    );
    // create bitmap and map the text
    let texts_bitmap_width = 2_usize.pow((text_bitmap.width as f64).log2().ceil() as u32);
    let texts_bitmap_height = 2_usize.pow((text_bitmap.height as f64).log2().ceil() as u32);
    let mut texts_bitmap = vec![0; texts_bitmap_width * texts_bitmap_height * 4];
    for j in 0..(text_bitmap.height as usize) {
        for i in 0..(text_bitmap.width as usize) {
            let idx = j * texts_bitmap_width + i;
            texts_bitmap[idx * 4 + 0] = 255;
            texts_bitmap[idx * 4 + 1] = 255;
            texts_bitmap[idx * 4 + 2] = 255;
            texts_bitmap[idx * 4 + 3] = text_bitmap.data[j * text_bitmap.width as usize + i];
        }
    }
    // create and load an image texture of the text
    check(load_image_texture(
        &mut vulkan_app,
        2,
        texts_bitmap_width as u32,
        texts_bitmap_height as u32,
        &texts_bitmap,
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
            texts_bitmap_width as f32,
            texts_bitmap_height as f32,
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
