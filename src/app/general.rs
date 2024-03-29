use super::*;

use crate::log::*;

use std::fs::File;
use std::io::{BufRead, BufReader};

impl SStarApp {
    /// A constructor.
    /// - `title` - the window title.
    /// - `base_width` - the basic scene width.
    /// - `base_height` - the basic scene height.
    /// - `max_tex_cnt` - the max number of bitmap textures.
    pub fn new(title: &str, base_width: f32, base_height: f32, max_tex_cnt: u32) -> Self {
        // read settings.cfg into a hashmap
        let mut settings = HashMap::new();
        match File::open("settings.cfg") {
            Ok(file) => {
                for l in BufReader::new(file).lines() {
                    let l = l.unwrap();
                    let (k, v) = l.split_once('=').unwrap_or_else(|| {
                        ss_error(&format!("invalid line '{l}' found in settings.cfg."))
                    });
                    settings.insert(k.to_string(), v.to_string());
                }
            }
            _ => ss_info("settings.cfg not found. default settings will be used."),
        }

        // configure
        let scene_scale = graphics::configure(&mut settings);
        let js_map = input::configure(&mut settings);

        // create instances
        let sw = (base_width * scene_scale) as u32;
        let sh = (base_height * scene_scale) as u32;
        let window_app = WindowApp::new(title, sw, sh);
        let vulkan_app = VulkanApp::new(&window_app, max_tex_cnt);

        // finish
        Self {
            settings,
            scene_scale,
            base_width,
            base_height,
            window_app,
            vulkan_app,
            uv_infos: HashMap::with_capacity(max_tex_cnt as usize),
            js_map,
            ub: None,
            tasks: Vec::new(),
        }
    }

    pub fn get_setting(&self, key: &str) -> Option<String> {
        self.settings.get(key).map(|n| n.clone())
    }

    pub fn update(&mut self) -> bool {
        self.window_app.do_events()
    }

    pub fn terminate(self) {
        self.vulkan_app.terminate();
        self.window_app.terminate();
    }
}
