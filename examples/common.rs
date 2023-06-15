use sstar::{
    app::{graphics::Position, SStarApp},
    bitmap::font::GlyphRasterizer,
    log::*,
    vulkan::PushConstant,
    window::Keycode,
};

const TEXT: &str =
    "L'enfer est plein de bonnes volontés ou désirs.\n地獄への道は善意で舗装されている。";

fn main() {
    // create sstar app
    let mut app = SStarApp::new("Sample Program", 640.0, 480.0, 10);

    // create and load an image texture
    app.load_image(1, "./examples/image.png");

    // create and load an texts texture
    let gr = GlyphRasterizer::new("./examples/mplus-2p-medium.ttf").unwrap();
    app.load_texts(&gr, 2, 32.0, &[TEXT]);

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

        // bind a image texture `"./example/image.png"`
        app.bind_texture(1);

        // push constant for a rotating square
        let mut pc = PushConstant::default();
        pc.scl[0] = 100.0;
        pc.scl[1] = 100.0;
        pc.trs[0] = 100.0;
        pc.trs[1] = 100.0;
        pc.rot[0] = cnt as f32 / 180.0 * std::f32::consts::PI;
        app.draw(pc, Position::Center);

        // bind a texts texture `&[TEXT]`
        app.bind_texture(2);

        // push constant for a text
        let mut pc = PushConstant::default();
        pc.trs[0] = 320.0;
        pc.trs[1] = 200.0;
        app.draw_text(pc, Position::CenterUI, 2, TEXT);

        // render
        app.flush();

        // next loop
        cnt += 1;
    }

    // terminate
    app.terminate();
}
