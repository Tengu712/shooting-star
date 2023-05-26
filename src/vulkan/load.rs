use super::*;

impl VulkanApp {
    /// A method to load an image texture.
    /// The parameter `id` is the key of VulkanApp::img_texs_map.
    pub(super) fn load(&mut self, id: usize, img_tex: Texture) -> Result<(), String> {
        if self.img_texs.contains_key(&id) {
            return Err(format!("the image texture {id} has already loaded."));
        }
        let idx = self
            .unused_img_tex_idxs
            .pop()
            .unwrap_or(self.img_texs.len());
        if idx >= self.descriptor_sets.len() {
            return Err(String::from("tried to load too many image textures."));
        }
        update_descriptor_set(
            self.device,
            self.descriptor_sets[idx],
            self.uniform_buffer.buffer,
            self.sampler,
            img_tex.image_view,
        );
        self.img_texs.insert(id, (idx, img_tex));
        Ok(())
    }

    /// A method to unload an image texture with id.
    pub fn unload(&mut self, id: usize) -> Result<(), String> {
        if id == DEFAULT_IMAGE_TEXTURE_ID {
            return Err(String::from("tried to unload the default image texture."));
        }
        if let Some((idx, img_tex)) = self.img_texs.remove(&id) {
            img_tex.terminate(self.device);
            self.unused_img_tex_idxs.push(idx);
            Ok(())
        } else {
            Err(format!("tried to unload an unloaded image texture {id}."))
        }
    }
}
