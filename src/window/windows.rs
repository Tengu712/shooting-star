use crate::log::*;
use crate::tpl::*;

use std::os::raw::{c_int, c_long};
use std::ptr::{null, null_mut};

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
    pub inst: HINSTANCE,
    pub window: HWND,
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
        let title_utf16 = title.encode_utf16().collect::<Vec<u16>>().as_ptr();
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
        Self { inst, window }
    }

    /// A method to do all queued window events and check if the window is deleted.
    /// If the window is deleted it returns `false`, otherwise `true`.
    pub fn do_events(&self) -> bool {
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
        true
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
