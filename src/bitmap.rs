use crate::log::*;
use crate::tpl::*;

pub mod font;
pub mod image;

pub struct BitmapInfo {
    pub width: u32,
    pub height: u32,
    pub data: Vec<u8>,
}
