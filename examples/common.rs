use sstar::{
    app::{graphics::Position, SStarApp},
    bitmap::font::GlyphRasterizer,
    log::*,
    vulkan::PushConstant,
    window::Keycode,
};

const UV_CIRCLE: [f32; 4] = [0.0, 0.0, 0.5, 1.0];
const UV_RECT: [f32; 4] = [0.5, 0.0, 1.0, 1.0];
const IMAGE_UVS: [(&str, [f32; 4]); 2] = [("circle", UV_CIRCLE), ("rect", UV_RECT)];
const TEXT: &str =
    "L'enfer est plein de bonnes volontés ou désirs.\n地獄への道は善意で舗装されている。";
const TEXTS: [&str; 1] = [TEXT];

enum TextureID {
    Image = 1,
    Texts,
}

fn main() {
    // create sstar app
    let mut app = SStarApp::new("Sample Program", 640.0, 480.0, 10);

    // create and load an image texture
    app.load_image(
        TextureID::Image as usize,
        "./examples/image.png",
        &IMAGE_UVS,
    );

    // create and load an texts texture
    let gr = GlyphRasterizer::new("./examples/mplus-2p-medium.ttf").unwrap();
    app.load_texts(&gr, TextureID::Texts as usize, 32.0, &TEXTS);

    // mainloop
    let mut cnt = 0;
    while app.update() {
        // get and check inputs
        let enter = app.get_input(Keycode::Enter);
        if enter > 0 {
            ss_debug(&format!("enter {enter}"));
        }
        let js_a = app.get_input(Keycode::JsButtonA);
        if js_a > 0 {
            ss_debug(&format!("js a {js_a}"));
        }

        // bind a texture
        app.bind_texture(TextureID::Image as usize);

        // draw a rotating circle
        app.draw_part(
            PushConstant {
                scl: [100.0, 100.0, 1.0, 0.0],
                rot: [cnt as f32 / 180.0 * std::f32::consts::PI, 0.0, 0.0, 0.0],
                trs: [-100.0, 100.0, 0.0, 0.0],
                ..Default::default()
            },
            Position::Center,
            TextureID::Image as usize,
            "circle",
        );

        // draw a rect
        app.draw_part(
            PushConstant {
                scl: [100.0, 100.0, 1.0, 0.0],
                trs: [640.0, 480.0, 0.0, 0.0],
                ..Default::default()
            },
            Position::BottomRightUI,
            TextureID::Image as usize,
            "rect",
        );

        // bind a texture
        app.bind_texture(TextureID::Texts as usize);

        // draw a text
        app.draw_part(
            PushConstant {
                trs: [320.0, 200.0, 0.0, 0.0],
                ..Default::default()
            },
            Position::CenterUI,
            TextureID::Texts as usize,
            TEXT,
        );

        // render
        app.flush();

        // next loop
        cnt += 1;
    }

    // terminate
    app.terminate();
}
