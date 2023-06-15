use sstar::app::SStarApp;

fn main() {
    let mut app = SStarApp::new("Sample Program", 640.0, 480.0, 10);
    while app.update() {
        app.flush();
    }
    app.terminate();
}
