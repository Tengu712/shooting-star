use fontdue::*;
use std::fs::File;
use std::io::Read;

pub struct GlyphRasterizer {
    font: Font,
}

impl GlyphRasterizer {
    /// A constructor.
    pub fn new(font_path: &str) -> Result<Self, String> {
        // read a font file
        let mut buf = Vec::new();
        File::open(font_path)
            .map_err(|e| format!("failed to open a file {font_path} : {e}"))?
            .read_to_end(&mut buf)
            .map_err(|e| format!("failed to read a file {font_path} : {e}"))?;

        // create a fontdue::Font
        let font = Font::from_bytes(buf.as_slice(), FontSettings::default())
            .map_err(|e| format!("failed to create a font : {e}"))?;

        // finish
        Ok(Self { font })
    }

    /// A method to rasterize a text `txt` with a font related with self.
    /// 
    /// The parameter `size` basically means the height from basic line to ascent.
    /// 
    /// The return value is `(coverage array, array width, array height)`. 
    /// The coverage is `[0, 255]` and the greater it is the more the pixel should be painted.
    pub fn rasterize(&self, txt: &str, size: f32) -> (Vec<u8>, usize, usize) {
        // get layout and pixels
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
        let mut r_pixels = vec![0; r_width * r_height];

        // map pixels
        let mut pos = 0;
        for (met, pxs) in tmps {
            let d = r_descend as i32 + met.ymin;
            let h = met.height as i32 + d;
            let oy = std::cmp::max(r_height as i32 - h, 0) as usize;
            let ox = std::cmp::max(pos as i32 + met.xmin, 0) as usize;
            for y in 0..met.height {
                for x in 0..met.width {
                    r_pixels[(y + oy) * r_width + (x + ox)] = pxs[y * met.width + x];
                }
            }
            pos += met.advance_width.floor() as usize;
        }

        // push buffer
        (r_pixels, r_width, r_height)
    }
}
