use crate::log::*;
use crate::tpl::*;

use std::collections::HashMap;
use std::os::raw::{c_int, c_long};
use std::ptr::{null, null_mut};

use super::Keycode;

const WINDOW_CLASS_NAME: LPCWSTR = [
    'S' as WCHAR,
    'K' as WCHAR,
    'D' as WCHAR,
    '_' as WCHAR,
    'W' as WCHAR,
    'C' as WCHAR,
    '\0' as WCHAR,
]
.as_ptr();

pub struct WindowApp {
    pub(crate) inst: HINSTANCE,
    pub(crate) window: HWND,
    key_states: HashMap<Keycode, i32>,
}

impl WindowApp {
    /// A constructor.
    /// It creates and shows a window.
    pub fn new(title: &str, width: u32, height: u32) -> Self {
        // get instance handle
        let inst = unsafe { GetModuleHandleW(null()) };
        if inst.is_null() {
            ss_error("failed to get a instance handle.");
        }

        // register a window class
        let wc = WNDCLASSEXW {
            cbSize: std::mem::size_of::<WNDCLASSEXW>() as u32,
            style: CS_CLASSDC,
            lpfnWndProc: Some(window_procedure),
            cbClsExtra: 0,
            cbWndExtra: 0,
            hInstance: inst,
            hIcon: null_mut(),
            hCursor: null_mut(),
            hbrBackground: null_mut(),
            lpszMenuName: null(),
            lpszClassName: WINDOW_CLASS_NAME,
            hIconSm: null_mut(),
        };
        if unsafe { RegisterClassExW(&wc) == 0 } {
            ss_error("failed to register a window class.");
        }

        // adjust a window size
        let style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        let mut rect = RECT {
            left: 0,
            top: 0,
            right: width as c_long,
            bottom: height as c_long,
        };
        unsafe { AdjustWindowRect(&mut rect, style, 0) };

        // create a window
        let mut title_utf16 = title.encode_utf16().collect::<Vec<u16>>();
        title_utf16.push(0);
        let title_utf16 = title_utf16.as_ptr();
        let window = unsafe {
            CreateWindowExW(
                0,
                WINDOW_CLASS_NAME,
                title_utf16,
                style,
                0,
                0,
                (rect.right - rect.left) as c_int,
                (rect.bottom - rect.top) as c_int,
                null_mut(),
                null_mut(),
                inst,
                null_mut(),
            )
        };
        if window.is_null() {
            ss_error("failed to create a window.");
        }

        // show the window
        unsafe { ShowWindow(window, SW_SHOWDEFAULT as c_int) };
        unsafe { UpdateWindow(window) };

        // finish
        Self {
            inst,
            window,
            key_states: HashMap::new(),
        }
    }

    /// A method to do all queued window events and check if the window is deleted.
    /// If the window is deleted it returns `false`, otherwise `true`.
    pub fn do_events(&mut self) -> bool {
        // prepare
        let mut msg = MSG {
            hwnd: null_mut(),
            message: 0,
            wParam: 0,
            lParam: 0,
            time: 0,
            pt: POINT { x: 0, y: 0 },
        };

        // do all window events
        loop {
            // if there's no event in queue, break from the loop
            if unsafe { PeekMessageW(&mut msg, null_mut(), 0, 0, PM_REMOVE) == 0 } {
                break;
            }
            // if the event message is WM_QUIT, tell invoker to end the process
            if msg.message == WM_QUIT {
                return false;
            }
            // otherwise, do the event
            unsafe { TranslateMessage(&msg) };
            unsafe { DispatchMessageW(&msg) };
        }

        // get keyboard input state
        let mut new_key_states = HashMap::new();
        let mut states = [0; 256];
        unsafe { GetKeyboardState(states.as_mut_ptr()) };
        for i in 0..256 {
            if let Some(kc) = convert_win_to_ss(i) {
                if states[i as usize] & 0x80 > 0 {
                    let prev = *self.key_states.entry(kc.clone()).or_insert_with(|| 0);
                    new_key_states.insert(kc, prev + 1);
                }
            }
        }

        // get game pad input state
        let mut xstate = XINPUT_STATE {
            dwPacketNumber: 0,
            Gamepad: XINPUT_GAMEPAD {
                wButtons: 0,
                bLeftTrigger: 0,
                bRightTrigger: 0,
                sThumbLX: 0,
                sThumbLY: 0,
                sThumbRX: 0,
                sThumbRY: 0,
            },
        };
        if unsafe { XInputGetState(0, &mut xstate) == ERROR_SUCCESS } {
            let mut button = |mask, kc: Keycode| {
                if xstate.Gamepad.wButtons & mask as WORD > 0 {
                    let prev = *self.key_states.entry(kc.clone()).or_insert_with(|| 0);
                    new_key_states.insert(kc, prev + 1);
                }
            };
            button(XINPUT_GAMEPAD_A, Keycode::JsButtonA);
            button(XINPUT_GAMEPAD_B, Keycode::JsButtonB);
            button(XINPUT_GAMEPAD_X, Keycode::JsButtonX);
            button(XINPUT_GAMEPAD_Y, Keycode::JsButtonY);
            button(XINPUT_GAMEPAD_DPAD_UP, Keycode::JsButtonUp);
            button(XINPUT_GAMEPAD_DPAD_LEFT, Keycode::JsButtonLeft);
            button(XINPUT_GAMEPAD_DPAD_DOWN, Keycode::JsButtonDown);
            button(XINPUT_GAMEPAD_DPAD_RIGHT, Keycode::JsButtonRight);
            button(XINPUT_GAMEPAD_START, Keycode::JsButtonStart);
            button(XINPUT_GAMEPAD_BACK, Keycode::JsButtonBack);
            button(XINPUT_GAMEPAD_LEFT_THUMB, Keycode::JsButtonLThumb);
            button(XINPUT_GAMEPAD_RIGHT_THUMB, Keycode::JsButtonRThumb);
            button(XINPUT_GAMEPAD_LEFT_SHOULDER, Keycode::JsButtonLShoulder);
            button(XINPUT_GAMEPAD_RIGHT_SHOULDER, Keycode::JsButtonRShoulder);
            new_key_states.insert(Keycode::JsAxisLX, xstate.Gamepad.sThumbLX as i32);
            new_key_states.insert(Keycode::JsAxisLY, xstate.Gamepad.sThumbLY as i32);
            new_key_states.insert(Keycode::JsAxisRX, xstate.Gamepad.sThumbRX as i32);
            new_key_states.insert(Keycode::JsAxisRY, xstate.Gamepad.sThumbRY as i32);
        }

        // apply new key input states
        self.key_states = new_key_states;

        // finish
        true
    }

    /// A method to get a key or game pad button input state.
    /// It returns how many frames the key or button pressed.
    /// If you try to get game pad axis input state, it returns 0.
    pub fn get_input(&self, kc: Keycode) -> u32 {
        match kc {
            Keycode::JsAxisLX | Keycode::JsAxisLY | Keycode::JsAxisRX | Keycode::JsAxisRY => 0,
            _ => *self.key_states.get(&kc).unwrap_or(&0) as u32,
        }
    }

    /// A method to get a game pad axis input state.
    /// It returns the tangent of axis as `[-1.0, 1.0]`.
    /// If you try to get any input state except the axis state, it returns 0.
    pub fn get_axis_input(&self, kc: Keycode) -> f32 {
        match kc {
            Keycode::JsAxisLX | Keycode::JsAxisLY | Keycode::JsAxisRX | Keycode::JsAxisRY => {
                let state = *self.key_states.get(&kc).unwrap_or(&0);
                if state > 0 {
                    state as f32 / i16::MAX as f32
                } else {
                    state as f32 / i16::MIN as f32 * -1.0
                }
            }
            _ => 0.0,
        }
    }

    pub fn terminate(self) {
        unsafe { DestroyWindow(self.window) };
        unsafe { UnregisterClassW(WINDOW_CLASS_NAME, self.inst) };
    }
}

extern "C" fn window_procedure(window: HWND, msg: u32, wparam: WPARAM, lparam: LPARAM) -> LRESULT {
    match msg {
        WM_DESTROY => {
            unsafe { PostQuitMessage(0) };
            return 0;
        }
        _ => unsafe { DefWindowProcW(window, msg, wparam, lparam) },
    }
}

fn convert_win_to_ss(i: u32) -> Option<Keycode> {
    match i {
        0x41 => Some(Keycode::KeyA),
        0x42 => Some(Keycode::KeyB),
        0x43 => Some(Keycode::KeyC),
        0x44 => Some(Keycode::KeyD),
        0x45 => Some(Keycode::KeyE),
        0x46 => Some(Keycode::KeyF),
        0x47 => Some(Keycode::KeyG),
        0x48 => Some(Keycode::KeyH),
        0x49 => Some(Keycode::KeyI),
        0x4A => Some(Keycode::KeyJ),
        0x4B => Some(Keycode::KeyK),
        0x4C => Some(Keycode::KeyL),
        0x4D => Some(Keycode::KeyM),
        0x4E => Some(Keycode::KeyN),
        0x4F => Some(Keycode::KeyO),
        0x50 => Some(Keycode::KeyP),
        0x51 => Some(Keycode::KeyQ),
        0x52 => Some(Keycode::KeyR),
        0x53 => Some(Keycode::KeyS),
        0x54 => Some(Keycode::KeyT),
        0x55 => Some(Keycode::KeyU),
        0x56 => Some(Keycode::KeyV),
        0x57 => Some(Keycode::KeyW),
        0x58 => Some(Keycode::KeyX),
        0x59 => Some(Keycode::KeyY),
        0x5A => Some(Keycode::KeyZ),
        0x30 => Some(Keycode::Key0),
        0x31 => Some(Keycode::Key1),
        0x32 => Some(Keycode::Key2),
        0x33 => Some(Keycode::Key3),
        0x34 => Some(Keycode::Key4),
        0x35 => Some(Keycode::Key5),
        0x36 => Some(Keycode::Key6),
        0x37 => Some(Keycode::Key7),
        0x38 => Some(Keycode::Key8),
        0x39 => Some(Keycode::Key9),
        VK_UP => Some(Keycode::Up),
        VK_LEFT => Some(Keycode::Left),
        VK_DOWN => Some(Keycode::Down),
        VK_RIGHT => Some(Keycode::Right),
        VK_RETURN => Some(Keycode::Enter),
        VK_SPACE => Some(Keycode::Space),
        VK_SHIFT | VK_LSHIFT | VK_RSHIFT => Some(Keycode::Shift),
        VK_TAB => Some(Keycode::Tab),
        VK_CONTROL | VK_LCONTROL | VK_RCONTROL => Some(Keycode::Control),
        VK_ESCAPE => Some(Keycode::Escape),
        _ => None,
    }
}
