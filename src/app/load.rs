use super::*;

use crate::bitmap::{font::*, image::*};
use crate::log::*;

impl SStarApp {
    /// A method to load image texture as `id`.
    pub fn load_image(&mut self, id: usize, path: &str) {
        if self.vulkan_app.check_image_texture_loaded(id) {
            ss_warning(&format!("the texture {id} has already been loaded."));
            return;
        }

        let bitmap = create_bitmap_from_file(path).unwrap_or_else(|e| ss_error(&e));
        self.vulkan_app
            .load_image_texture(id as usize, bitmap.width, bitmap.height, &bitmap.data)
            .unwrap_or_else(|e| ss_error(&e));
    }

    /// A method to check if the texture `id` has been loaded.
    pub fn check_texture_loaded(&self, id: usize) -> bool {
        self.vulkan_app.check_image_texture_loaded(id)
    }

    /// A method to load texts texture as `id`.
    pub fn load_texts(&mut self, gr: &GlyphRasterizer, id: usize, size: f32, txts: &[&str]) {
        if self.vulkan_app.check_image_texture_loaded(id) {
            ss_warning(&format!("the texture {id} has already been loaded."));
            return;
        }

        // rasterize glyphs
        let bitmap_infos = txts
            .iter()
            .map(|t| (t.to_string(), gr.rasterize(t, size)))
            .collect::<Vec<_>>();

        // create a empty bitmap
        let width = bitmap_infos.iter().map(|(_, n)| n.width).max().unwrap();
        let width = 2_usize.pow((width as f64).log2().ceil() as u32);
        let height = bitmap_infos.iter().map(|(_, n)| n.height).sum::<u32>();
        let height = 2_usize.pow((height as f64).log2().ceil() as u32);
        let mut bitmap = vec![0; width * height * 4];

        // map text bitmaps into the empty bitmap
        let mut oy = 0;
        let mut map = HashMap::new();
        for (key, info) in bitmap_infos {
            // map
            for j in 0..(info.height as usize) {
                for i in 0..(info.width as usize) {
                    let idx = (j + oy) * width + i;
                    bitmap[idx * 4 + 0] = 255;
                    bitmap[idx * 4 + 1] = 255;
                    bitmap[idx * 4 + 2] = 255;
                    bitmap[idx * 4 + 3] = info.data[j * info.width as usize + i];
                }
            }
            // register
            let width = width as f32;
            let height = height as f32;
            let new_oy = oy + info.height as usize;
            map.insert(
                key,
                TextInfo {
                    width: info.width as f32,
                    height: info.height as f32,
                    uv: [
                        0.0,
                        oy as f32 / height,
                        info.width as f32 / width,
                        new_oy as f32 / height,
                    ],
                },
            );
            oy = new_oy;
        }

        // register the map of text information
        self.text_infos.insert(id, map);

        // load
        self.vulkan_app
            .load_image_texture(id as usize, width as u32, height as u32, &bitmap)
            .unwrap_or_else(|e| ss_error(&e));
    }

    pub fn unload_texture(&mut self, id: usize) {
        self.text_infos.remove(&id);
        self.vulkan_app.unload_image_texture(id).unwrap();
    }
}
