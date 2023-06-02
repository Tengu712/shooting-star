use super::*;

use std::fs::File;
use std::io::Read;
use std::ptr::null_mut;

pub struct GlyphRasterizer {
    info: stbtt_fontinfo,
}

impl GlyphRasterizer {
    /// A constructor.
    pub fn new(path: &str) -> Result<Self, String> {
        // read a font file
        let mut buf = Vec::new();
        File::open(path)
            .map_err(|e| format!("failed to open a file {path} : {e}"))?
            .read_to_end(&mut buf)
            .map_err(|e| format!("failed to read a file {path} : {e}"))?;

        // get offset of font
        let offset = unsafe { stbtt_GetFontOffsetForIndex(buf.as_ptr(), 0) };
        if offset == -1 {
            return Err(format!("failed to get 0th font of {path}."));
        }

        // create a fontinfo
        let mut info = stbtt_fontinfo::default();
        if unsafe { stbtt_InitFont(&mut info, buf.as_ptr(), offset) == 0 } {
            return Err(format!("failed to init a font {path}."));
        }

        // finish
        Ok(Self { info })
    }

    /// A method to rasterize a text `txt` with a font related with self.
    ///
    /// The parameter `size` basically means the height from basic line to ascent.
    ///
    /// The return value is `(coverage array, array width, array height)`.
    /// The coverage is `[0, 255]` and the greater it is the more the pixel should be painted.
    pub fn rasterize(&self, txt: &str, size: f32) -> BitmapInfo {
        // get layout and bitmap
        let mut r_width = 0.0;
        let mut r_descend = 0;
        let chars = txt.chars();
        let mut tmps = Vec::with_capacity(chars.clone().count());
        for c in chars {
            let (met, pxs) = self.font.rasterize(c, size);
            r_width += met.advance_width;
            r_descend = std::cmp::min(met.ymin, r_descend);
            tmps.push((met, pxs));
        }

        // create buffer
        let r_width = r_width.ceil() as usize;
        let r_descend = r_descend.abs() as usize;
        let r_height = size.ceil() as usize + r_descend;
        let mut r_bitmap = vec![0; r_width * r_height];

        // map bitmap
        let mut pos = 0;
        for (met, pxs) in tmps {
            let d = r_descend as i32 + met.ymin;
            let h = met.height as i32 + d;
            let oy = std::cmp::max(r_height as i32 - h, 0) as usize;
            let ox = std::cmp::max(pos as i32 + met.xmin, 0) as usize;
            for y in 0..met.height {
                for x in 0..met.width {
                    r_bitmap[(y + oy) * r_width + (x + ox)] = pxs[y * met.width + x];
                }
            }
            pos += met.advance_width.floor() as usize;
        }

        // push buffer
        BitmapInfo {
            width: r_width as u32,
            height: r_height as u32,
            data: r_bitmap,
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
