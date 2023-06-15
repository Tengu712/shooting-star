use super::*;

use crate::log::*;

const KB_STR_KEY: [(&'static str, Keycode); 46] = [
    ("a", Keycode::KeyA),
    ("b", Keycode::KeyB),
    ("c", Keycode::KeyC),
    ("d", Keycode::KeyD),
    ("e", Keycode::KeyE),
    ("f", Keycode::KeyF),
    ("g", Keycode::KeyG),
    ("h", Keycode::KeyH),
    ("i", Keycode::KeyI),
    ("j", Keycode::KeyJ),
    ("k", Keycode::KeyK),
    ("l", Keycode::KeyL),
    ("m", Keycode::KeyM),
    ("n", Keycode::KeyN),
    ("o", Keycode::KeyO),
    ("p", Keycode::KeyP),
    ("q", Keycode::KeyQ),
    ("r", Keycode::KeyR),
    ("s", Keycode::KeyS),
    ("t", Keycode::KeyT),
    ("u", Keycode::KeyU),
    ("v", Keycode::KeyV),
    ("w", Keycode::KeyW),
    ("x", Keycode::KeyX),
    ("y", Keycode::KeyY),
    ("z", Keycode::KeyZ),
    ("0", Keycode::Key0),
    ("1", Keycode::Key1),
    ("2", Keycode::Key2),
    ("3", Keycode::Key3),
    ("4", Keycode::Key4),
    ("5", Keycode::Key5),
    ("6", Keycode::Key6),
    ("7", Keycode::Key7),
    ("8", Keycode::Key8),
    ("9", Keycode::Key9),
    ("up", Keycode::Up),
    ("left", Keycode::Left),
    ("down", Keycode::Down),
    ("right", Keycode::Right),
    ("enter", Keycode::Enter),
    ("space", Keycode::Space),
    ("shift", Keycode::Shift),
    ("tab", Keycode::Tab),
    ("control", Keycode::Control),
    ("escape", Keycode::Escape),
];

// TODO: add axis
const JS_STR_KEY: [(&'static str, Keycode); 13] = [
    ("a", Keycode::JsButtonA),
    ("b", Keycode::JsButtonB),
    ("x", Keycode::JsButtonX),
    ("y", Keycode::JsButtonY),
    ("left", Keycode::JsButtonLeft),
    ("right", Keycode::JsButtonRight),
    ("up", Keycode::JsButtonUp),
    ("down", Keycode::JsButtonDown),
    ("left-shoulder", Keycode::JsButtonLShoulder),
    ("right-shoulder", Keycode::JsButtonRShoulder),
    ("left-thumb", Keycode::JsButtonLThumb),
    ("right-thumb", Keycode::JsButtonRThumb),
    ("start", Keycode::JsButtonStart),
];

pub(super) fn configure(settings: &mut HashMap<String, String>) -> HashMap<Keycode, Keycode> {
    let js_str_key = HashMap::from(JS_STR_KEY);
    let mut js_map = HashMap::new();
    for (s, kkc) in KB_STR_KEY {
        if let Some(v) = settings.remove(s) {
            let jkc = js_str_key
                .get(v.as_str())
                .unwrap_or_else(|| ss_error(&format!("invalid value '{v}' in settings.cfg.")));
            js_map.insert(kkc, jkc.clone());
        }
    }
    js_map
}

impl SStarApp {
    // TODO: add axis
    pub fn get_input(&self, kc: Keycode) -> u32 {
        let kb = self.window_app.get_input(kc.clone());
        let js = self
            .js_map
            .get(&kc)
            .map(|n| self.window_app.get_input(n.clone()))
            .unwrap_or(0);
        std::cmp::max(kb, js)
    }
}
