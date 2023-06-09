use super::*;

use std::mem::size_of;
use std::os::raw::c_void;

impl VulkanApp {
    /// A method to load an image texture.
    pub fn load_image_texture(
        &mut self,
        id: usize,
        width: u32,
        height: u32,
        bitmap: &[u8],
    ) -> Result<(), String> {
        // check if the image texture has been already loaded
        if self.img_texs.contains_key(&id) {
            return Err(format!("the image texture {id} has already loaded."));
        }

        // get a descriptor set index
        let idx = self
            .unused_img_tex_idxs
            .pop()
            .unwrap_or(self.img_texs.len());
        if idx >= self.descriptor_sets.len() {
            return Err(String::from("tried to load too many image textures."));
        }

        // check if the bitmap array length is correct.
        if width * height * 4 != bitmap.len() as u32 {
            return Err(format!(
                "width {width} and height {height} passed so bitmap array length expected {} but passed {}.",
                width * height * 4,
                bitmap.len(),
            ));
        }

        // create a texture.
        let flags = VkImageUsageFlagBits_VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VkImageUsageFlagBits_VK_IMAGE_USAGE_SAMPLED_BIT;
        let img_tex = Texture::new(
            self.device,
            &self.phys_device_mem_props,
            VkFormat_VK_FORMAT_R8G8B8A8_UNORM,
            width,
            height,
            flags as VkImageUsageFlags,
            VkImageAspectFlagBits_VK_IMAGE_ASPECT_COLOR_BIT as VkImageAspectFlags,
        )?;

        // copy the bitmap data to the texture
        copy_memory(
            self.device,
            &self.phys_device_mem_props,
            self.queue,
            self.command_pool,
            width,
            height,
            img_tex.image,
            size_of::<u8>() * bitmap.len(),
            bitmap.as_ptr() as *const c_void,
        )?;

        // update the descriptor set
        update_descriptor_set(
            self.device,
            self.descriptor_sets[idx],
            self.uniform_buffer.buffer,
            self.sampler,
            img_tex.image_view,
        );

        // finish
        self.img_texs.insert(id, (idx, img_tex));
        Ok(())
    }

    /// A method to unload an image texture with id.
    pub fn unload_image_texture(&mut self, id: usize) -> Result<(), String> {
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

    /// A method to check if an image texture with id has been loaded.
    pub fn check_image_texture_loaded(&self, id: usize) -> bool {
        self.img_texs.contains_key(&id)
    }
}
