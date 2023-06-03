use super::*;

use std::fs::File;
use std::io::Read;
use std::os::raw::c_int;
use std::ptr::null_mut;

struct Glyph {
    /// If it is None it means newline.
    idx: Option<c_int>,
    aw: c_int,
}

#[repr(C)]
pub struct GlyphRasterizer {
    info: stbtt_fontinfo,
    bin: Vec<u8>,
}

impl GlyphRasterizer {
    /// A constructor.
    pub fn new(path: &str) -> Result<Self, String> {
        // read a font file
        let mut bin = Vec::new();
        File::open(path)
            .map_err(|e| format!("failed to open a file {path} : {e}"))?
            .read_to_end(&mut bin)
            .map_err(|e| format!("failed to read a file {path} : {e}"))?;

        // get offset of font
        let offset = unsafe { stbtt_GetFontOffsetForIndex(bin.as_ptr(), 0) };
        if offset == -1 {
            return Err(format!("failed to get 0th font of {path}."));
        }

        // create a fontinfo
        let mut info = stbtt_fontinfo::default();
        if unsafe { stbtt_InitFont(&mut info, bin.as_ptr(), offset) == 0 } {
            return Err(format!("failed to init a font {path}."));
        }

        // finish
        Ok(Self { info, bin })
    }

    /// A method to rasterize a text `txt` with a font related with self.
    pub fn rasterize(&self, txt: &str, size: f32) -> BitmapInfo {
        // get the glyph index
        let scale = unsafe { stbtt_ScaleForPixelHeight(&self.info, size) };

        // get the font metrics
        let mut ascent = 0;
        let mut descent = 0;
        let mut line_gap = 0;
        unsafe { stbtt_GetFontVMetrics(&self.info, &mut ascent, &mut descent, &mut line_gap) };
        let ascent = ascent as f32 * scale;
        let descent = descent as f32 * scale;
        let line_gap = line_gap as f32 * scale;
        let line_height = (ascent - descent + line_gap) as usize;

        // get glyph indecies and the width and height of result bitmap
        let mut width_tmp = 0;
        let mut width = 0;
        let mut height = line_height;
        let mut glyphs = Vec::with_capacity(txt.chars().count());
        for c in txt.chars() {
            // if it is a newline
            if c == '\n' {
                width = std::cmp::max(width, width_tmp);
                width_tmp = 0;
                height += line_height;
                glyphs.push(Glyph { idx: None, aw: 0 });
                continue;
            }

            // find the glyph index
            let idx = unsafe { stbtt_FindGlyphIndex(&self.info, c as c_int) };
            if idx == 0 {
                ss_warning(&format!("failed to find a glyph index of {c}"));
                continue;
            }

            // get the glyph adcance width
            let mut aw = 0;
            let mut lsb = 0;
            unsafe { stbtt_GetGlyphHMetrics(&self.info, idx, &mut aw, &mut lsb) };
            let aw = (aw as f32 * scale) as c_int;

            // finish
            width_tmp += aw as usize;
            glyphs.push(Glyph { idx: Some(idx), aw });
        }
        let width = std::cmp::max(width, width_tmp);

        // rasterize
        let mut x_org = 0;
        let mut y_org = 0;
        let mut data = vec![0; width * height];
        for glyph in glyphs {
            // if it is a newline
            if glyph.idx.is_none() {
                x_org = 0;
                y_org += line_height as c_int;
                continue;
            }

            // get the glyph bitmap
            let idx = glyph.idx.unwrap();
            let mut w = 0;
            let mut h = 0;
            let mut ox = 0;
            let mut oy = 0;
            let bitmap = unsafe {
                stbtt_GetGlyphBitmap(
                    &self.info, scale, scale, idx, &mut w, &mut h, &mut ox, &mut oy,
                )
            };
            let w = w as usize;
            let h = h as usize;
            let bitmap = unsafe { Vec::from_raw_parts(bitmap, w * h, w * h) };

            // map the bitmap to the result bitmap
            for j in 0..h {
                for i in 0..w {
                    let x = x_org + ox + i as c_int;
                    let y = y_org + ascent as c_int + oy + j as c_int;
                    let x = std::cmp::min(width, std::cmp::max(0, x) as usize);
                    let y = std::cmp::min(height, std::cmp::max(0, y) as usize);
                    let ridx = y * width + x;
                    let gidx = j * w + i;
                    data[ridx] = bitmap[gidx];
                }
            }

            // finish
            x_org += glyph.aw;
        }

        // finish
        BitmapInfo {
            width: width as u32,
            height: height as u32,
            data,
        }
    }
}

impl Default for stbtt__buf {
    fn default() -> Self {
        Self {
            data: null_mut(),
            cursor: 0,
            size: 0,
        }
    }
}

impl Default for stbtt_fontinfo {
    fn default() -> Self {
        Self {
            userdata: null_mut(),
            data: null_mut(),
            fontstart: 0,
            numGlyphs: 0,
            loca: 0,
            head: 0,
            glyf: 0,
            hhea: 0,
            hmtx: 0,
            kern: 0,
            gpos: 0,
            svg: 0,
            index_map: 0,
            indexToLocFormat: 0,
            cff: stbtt__buf::default(),
            charstrings: stbtt__buf::default(),
            gsubrs: stbtt__buf::default(),
            subrs: stbtt__buf::default(),
            fontdicts: stbtt__buf::default(),
            fdselect: stbtt__buf::default(),
        }
    }
}
