//! Shooting Star is a front engine for a simple 2D game.
//! 
//! # Example
//! 
//! Here is a example that creates a window and clear it default color:
//! 
//! ```rust
//! use sstar::{
//!     vulkan::*,
//!     window::*,
//! };
//! fn main() {
//!     let mut window_app = WindowApp::new("Sample Program\0", 640, 480);
//!     let vulkan_app = VulkanApp::new(&window_app, 10);
//!     while window_app.do_events() {
//!         vulkan_app.render(None, &[]).unwrap();
//!     }
//!     vulkan_app.terminate();
//!     window_app.terminate();
//! }
//! ```

pub mod bitmap;
pub mod log;
#[allow(non_upper_case_globals)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
mod tpl;
pub mod vulkan;
pub mod window;
