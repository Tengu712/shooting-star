use crate::log::*;
use crate::tpl::*;

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

pub struct WindowApp {
    pub(crate) display: *mut Display,
    pub(crate) window: Window,
    atom_protocols: Atom,
    atom_delete_window: Atom,
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
        let c_title = title.bytes().map(|c| c as c_char).collect::<Vec<c_char>>();
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

        // show the window
        unsafe { XMapWindow(display, window) };
        unsafe { XFlush(display) };

        // finish
        Self {
            display,
            window,
            atom_protocols,
            atom_delete_window,
        }
    }

    /// A method to do all queued window events and check if the window is deleted.
    /// If the window is deleted it returns `false`, otherwise `true`.
    pub fn do_events(&self) -> bool {
        // do all window events
        while unsafe { XPending(self.display) } != 0 {
            // get a window event
            let mut event = _XEvent { type_: 0 };
            unsafe { XNextEvent(self.display, &mut event) };

            // get the event type
            let type_ = unsafe { event.type_ as u32 };

            // when it's a window deletion event, tell invoker to end the process
            if type_ == ClientMessage {
                let flag_type = unsafe { event.xclient.message_type == self.atom_protocols };
                let flag_data =
                    unsafe { event.xclient.data.l[0] == self.atom_delete_window as c_long };
                if flag_type && flag_data {
                    return false;
                }
            }
        }
        true
    }

    pub fn terminate(self) {}
}
