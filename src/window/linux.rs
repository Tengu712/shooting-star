#![allow(non_upper_case_globals)]

use crate::log::*;
use crate::tpl::*;

use super::Keycode;

use std::collections::HashMap;
use std::ffi::c_void;
use std::mem::size_of;
use std::os::raw::{c_char, c_int, c_long};
use std::ptr::null;

const WM_PROTOCOLS: *const c_char = [
    'W' as c_char,
    'M' as c_char,
    '_' as c_char,
    'P' as c_char,
    'R' as c_char,
    'O' as c_char,
    'T' as c_char,
    'O' as c_char,
    'C' as c_char,
    'O' as c_char,
    'L' as c_char,
    'S' as c_char,
    '\0' as c_char,
]
.as_ptr();
const WM_DELETE_WINDOW: *const c_char = [
    'W' as c_char,
    'M' as c_char,
    '_' as c_char,
    'D' as c_char,
    'E' as c_char,
    'L' as c_char,
    'E' as c_char,
    'T' as c_char,
    'E' as c_char,
    '_' as c_char,
    'W' as c_char,
    'I' as c_char,
    'N' as c_char,
    'D' as c_char,
    'O' as c_char,
    'W' as c_char,
    '\0' as c_char,
]
.as_ptr();

const DEV_INPUT_JS0: *const c_char = [
    '/' as c_char,
    'd' as c_char,
    'e' as c_char,
    'v' as c_char,
    '/' as c_char,
    'i' as c_char,
    'n' as c_char,
    'p' as c_char,
    'u' as c_char,
    't' as c_char,
    '/' as c_char,
    'j' as c_char,
    's' as c_char,
    '0' as c_char,
    '\0' as c_char,
]
.as_ptr();

const POV_THRESHOLD: __s16 = 30000;

pub struct WindowApp {
    pub(crate) display: *mut Display,
    pub(crate) window: Window,
    atom_protocols: Atom,
    atom_delete_window: Atom,
    input_states: HashMap<Keycode, i32>,
    fd: c_int,
}

impl WindowApp {
    /// A constructor.
    /// It creates and shows a window.
    pub fn new(title: &str, width: u32, height: u32) -> Self {
        // create a window
        let display = unsafe { XOpenDisplay(null()) };
        if display.is_null() {
            ss_error("failed to create a display in Xlib.");
        }
        let window = unsafe {
            XCreateSimpleWindow(
                display,
                XDefaultRootWindow(display),
                0,
                0,
                width,
                height,
                0,
                0,
                0,
            )
        };

        // change the window title
        let mut c_title = title.bytes().map(|c| c as c_char).collect::<Vec<c_char>>();
        c_title.push(0);
        let c_title = c_title.as_ptr();
        unsafe { XStoreName(display, window, c_title) };

        // make the window unresizable
        let mut size_hints = XSizeHints {
            flags: (PMinSize | PMaxSize) as i64,
            x: 0,
            y: 0,
            width: 0,
            height: 0,
            min_width: width as i32,
            min_height: height as i32,
            max_width: width as i32,
            max_height: height as i32,
            width_inc: 0,
            height_inc: 0,
            min_aspect: XSizeHints__bindgen_ty_1 { x: 0, y: 0 },
            max_aspect: XSizeHints__bindgen_ty_1 { x: 0, y: 0 },
            base_width: 0,
            base_height: 0,
            win_gravity: 0,
        };
        unsafe { XSetWMNormalHints(display, window, &mut size_hints) };

        // set the window deletion event
        let atom_protocols = unsafe { XInternAtom(display, WM_PROTOCOLS, False as c_int) };
        let mut atom_delete_window =
            unsafe { XInternAtom(display, WM_DELETE_WINDOW, False as c_int) };
        unsafe { XSetWMProtocols(display, window, &mut atom_delete_window, 1) };

        // set the window input event
        unsafe { XSelectInput(display, window, (KeyPressMask | KeyReleaseMask) as c_long) };

        // open /dev/input/js0
        let fd = unsafe { open(DEV_INPUT_JS0, O_RDONLY as c_int | O_NONBLOCK as c_int) };

        // show the window
        unsafe { XMapWindow(display, window) };
        unsafe { XFlush(display) };

        // finish
        Self {
            display,
            window,
            atom_protocols,
            atom_delete_window,
            input_states: HashMap::new(),
            fd,
        }
    }

    /// A method to do all queued window events and check if the window is deleted.
    /// If the window is deleted it returns `false`, otherwise `true`.
    pub fn do_events(&mut self) -> bool {
        // do all window events
        while unsafe { XPending(self.display) } != 0 {
            // get a window event
            let mut event = _XEvent { type_: 0 };
            unsafe { XNextEvent(self.display, &mut event) };

            // get the event type
            let type_ = unsafe { event.type_ as u32 };

            // when it's a window deletion event, tell invoker to end the process
            match type_ {
                ClientMessage => {
                    let flag_type = unsafe { event.xclient.message_type == self.atom_protocols };
                    let flag_data =
                        unsafe { event.xclient.data.l[0] == self.atom_delete_window as c_long };
                    if flag_type && flag_data {
                        return false;
                    }
                }
                KeyPress => {
                    let key_sym = unsafe {
                        XkbKeycodeToKeysym(self.display, event.xkey.keycode as KeyCode, 0, 0)
                    };
                    if let Some(kc) = convert_x_to_ss(key_sym) {
                        let v = *self.input_states.entry(kc.clone()).or_insert_with(|| 0);
                        self.input_states.insert(kc, v);
                    }
                }
                KeyRelease => {
                    let key_sym = unsafe {
                        XkbKeycodeToKeysym(self.display, event.xkey.keycode as KeyCode, 0, 0)
                    };
                    if let Some(kc) = convert_x_to_ss(key_sym) {
                        self.input_states.remove(&kc);
                    }
                }
                _ => (),
            }
        }

        // update js0
        if self.fd >= 0 {
            let mut jevent = js_event {
                time: 0,
                value: 0,
                type_: 0,
                number: 0,
            };
            let js_event_size = size_of::<js_event>();
            if unsafe {
                crate::tpl::read(self.fd, &mut jevent as *mut _ as *mut c_void, js_event_size)
                    == js_event_size as isize
            } {
                if jevent.type_ as u32 & JS_EVENT_BUTTON > 0 {
                    if let Some(kc) = convert_js_to_ss(jevent.number) {
                        if jevent.value == 1 {
                            let v = *self.input_states.entry(kc.clone()).or_insert_with(|| 0);
                            self.input_states.insert(kc, v);
                        } else {
                            self.input_states.remove(&kc);
                        }
                    }
                }
                if jevent.type_ as u32 & JS_EVENT_AXIS > 0 {
                    // pov
                    if jevent.number == 6 {
                        self.update_axis(
                            Keycode::JsButtonRight,
                            Keycode::JsButtonLeft,
                            jevent.value,
                        );
                    } else if jevent.number == 7 {
                        self.update_axis(Keycode::JsButtonDown, Keycode::JsButtonUp, jevent.value);
                    }
                    // axis
                    else if jevent.number == 0 {
                        self.input_states
                            .insert(Keycode::JsAxisLX, jevent.value as i32);
                    } else if jevent.number == 1 {
                        self.input_states
                            .insert(Keycode::JsAxisLY, jevent.value as i32);
                    } else if jevent.number == 3 {
                        self.input_states
                            .insert(Keycode::JsAxisRX, jevent.value as i32);
                    } else if jevent.number == 4 {
                        self.input_states
                            .insert(Keycode::JsAxisRY, jevent.value as i32);
                    }
                }
            }
        }

        // update input states
        self.input_states.iter_mut().for_each(|(_, n)| *n += 1);
        true
    }

    /// A method to get a key or game pad button input state.
    /// It returns how many frames the key or button pressed.
    /// If you try to get game pad axis input state, it returns 0.
    pub fn get_input(&self, kc: Keycode) -> u32 {
        match kc {
            Keycode::JsAxisLX | Keycode::JsAxisLY | Keycode::JsAxisRX | Keycode::JsAxisRY => 0,
            _ => *self.input_states.get(&kc).unwrap_or(&0) as u32,
        }
    }

    /// A method to get a game pad axis input state.
    /// It returns the tangent of axis as `[-1.0, 1.0]`.
    /// If you try to get any input state except the axis state, it returns 0.
    pub fn get_axis_input(&self, kc: Keycode) -> f32 {
        match kc {
            Keycode::JsAxisLX | Keycode::JsAxisLY | Keycode::JsAxisRX | Keycode::JsAxisRY => {
                let state = *self.input_states.get(&kc).unwrap_or(&0);
                if state > 0 {
                    state as f32 / i16::MAX as f32
                } else {
                    state as f32 / i16::MIN as f32 * -1.0
                }
            }
            _ => 0.0,
        }
    }

    pub fn terminate(self) {}

    fn update_axis(&mut self, kc_greater: Keycode, kc_less: Keycode, value: __s16) {
        if value > POV_THRESHOLD {
            let v = *self
                .input_states
                .entry(kc_greater.clone())
                .or_insert_with(|| 0);
            self.input_states.insert(kc_greater, v);
        } else if value < -POV_THRESHOLD {
            let v = *self
                .input_states
                .entry(kc_less.clone())
                .or_insert_with(|| 0);
            self.input_states.insert(kc_less, v);
        } else {
            self.input_states.remove(&kc_greater);
            self.input_states.remove(&kc_less);
        }
    }
}

fn convert_x_to_ss(key_sym: KeySym) -> Option<Keycode> {
    match key_sym as u32 {
        XK_a => Some(Keycode::KeyA),
        XK_b => Some(Keycode::KeyB),
        XK_c => Some(Keycode::KeyC),
        XK_d => Some(Keycode::KeyD),
        XK_e => Some(Keycode::KeyE),
        XK_f => Some(Keycode::KeyF),
        XK_g => Some(Keycode::KeyG),
        XK_h => Some(Keycode::KeyH),
        XK_i => Some(Keycode::KeyI),
        XK_j => Some(Keycode::KeyJ),
        XK_k => Some(Keycode::KeyK),
        XK_l => Some(Keycode::KeyL),
        XK_m => Some(Keycode::KeyM),
        XK_n => Some(Keycode::KeyN),
        XK_o => Some(Keycode::KeyO),
        XK_p => Some(Keycode::KeyP),
        XK_q => Some(Keycode::KeyQ),
        XK_r => Some(Keycode::KeyR),
        XK_s => Some(Keycode::KeyS),
        XK_t => Some(Keycode::KeyT),
        XK_u => Some(Keycode::KeyU),
        XK_v => Some(Keycode::KeyV),
        XK_w => Some(Keycode::KeyW),
        XK_x => Some(Keycode::KeyX),
        XK_y => Some(Keycode::KeyY),
        XK_z => Some(Keycode::KeyZ),
        XK_0 => Some(Keycode::Key0),
        XK_1 => Some(Keycode::Key1),
        XK_2 => Some(Keycode::Key2),
        XK_3 => Some(Keycode::Key3),
        XK_4 => Some(Keycode::Key4),
        XK_5 => Some(Keycode::Key5),
        XK_6 => Some(Keycode::Key6),
        XK_7 => Some(Keycode::Key7),
        XK_8 => Some(Keycode::Key8),
        XK_9 => Some(Keycode::Key9),
        XK_Up => Some(Keycode::Up),
        XK_Left => Some(Keycode::Left),
        XK_Down => Some(Keycode::Down),
        XK_Right => Some(Keycode::Right),
        XK_Return => Some(Keycode::Enter),
        XK_space => Some(Keycode::Space),
        XK_Shift_L | XK_Shift_R => Some(Keycode::Shift),
        XK_Tab => Some(Keycode::Tab),
        XK_Control_L | XK_Control_R => Some(Keycode::Control),
        XK_Escape => Some(Keycode::Escape),
        _ => None,
    }
}

fn convert_js_to_ss(number: __u8) -> Option<Keycode> {
    match number {
        0 => Some(Keycode::JsButtonA),
        1 => Some(Keycode::JsButtonB),
        2 => Some(Keycode::JsButtonX),
        3 => Some(Keycode::JsButtonY),
        4 => Some(Keycode::JsButtonLShoulder),
        5 => Some(Keycode::JsButtonRShoulder),
        6 => Some(Keycode::JsButtonBack),
        7 => Some(Keycode::JsButtonStart),
        9 => Some(Keycode::JsButtonLThumb),
        10 => Some(Keycode::JsButtonRThumb),
        _ => None,
    }
}
