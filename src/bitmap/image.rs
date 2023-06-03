use super::*;

use std::os::raw::c_char;

/// A function to create a bitmap from an image file.
pub fn create_bitmap_from_file(path: &str) -> Result<BitmapInfo, String> {
    // prepare
    let mut filename = path.bytes().map(|n| n as c_char).collect::<Vec<c_char>>();
    filename.push(0);

    // load a bitmap as pointer
    let mut x = 0;
    let mut y = 0;
    let mut channels_in_file = 0;
    let bitmap = unsafe { stbi_load(filename.as_ptr(), &mut x, &mut y, &mut channels_in_file, 4) };
    if bitmap.is_null() {
        return Err(format!("failed to create a bitmap from a file {path}."));
    }

    // create a vector from the pointer
    let len = (x * y * channels_in_file) as usize;
    let data = unsafe { Vec::from_raw_parts(bitmap, len, len) };

    // finish
    Ok(BitmapInfo {
        width: x as u32,
        height: y as u32,
        data,
    })
}
