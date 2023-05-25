use super::*;

impl VulkanApp {
    /// A method to load an image texture.
    /// The parameter `id` is the key of VulkanApp::img_texs_map.
    pub(crate) fn load(&mut self, id: usize, img_tex: Texture) -> Result<(), String> {
        if self.img_texs_map.contains_key(&id) {
            return Err(format!("the image texture {id} has already loaded."));
        }
        let idx = self
            .img_texs
            .iter()
            .position(|n| n.is_none())
            .unwrap_or(self.img_texs.len());
        if idx >= self.descriptor_sets.len() {
            return Err(String::from("tried to load too many image textures."));
        }
        VulkanApp::load_to_descriptor_set(
            self.device,
            self.descriptor_sets[idx],
            self.uniform_buffer.buffer,
            self.sampler,
            img_tex.image_view,
        );
        if idx < self.img_texs.len() {
            self.img_texs[idx] = Some(img_tex);
        } else {
            self.img_texs.push(Some(img_tex));
        }
        self.img_texs_map.insert(id, idx);
        Ok(())
    }

    /// A method to unload an image texture with id.
    /// It change
    pub(crate) fn unload(&mut self, id: usize) -> Result<(), String> {
        let idx = if let Some(n) = self.img_texs_map.get(&id) {
            *n
        } else {
            return Err(format!("tried to unload an unloaded image texture {id}."));
        };
        if idx >= self.img_texs.len() {
            return Err(format!("unexpected error happens in VulkanApp::unload : self.img_texs_map[{id}] is {idx} and is out of self.img_texs."));
        }
        if let Some(img_tex) = std::mem::replace(&mut self.img_texs[idx], None) {
            img_tex.terminate(self.device);
            self.img_texs_map.remove(&id);
            Ok(())
        } else {
            Err(format!(
                "unexpected error happens in VulkanApp::unload : self.img_texs[{idx}] is None."
            ))
        }
    }

    /// A inner method in VulkanApp::load.
    /// This is defined for VulkanApp::new.
    pub(super) fn load_to_descriptor_set(
        device: VkDevice,
        descriptor_set: VkDescriptorSet,
        buffer: VkBuffer,
        sampler: VkSampler,
        image_view: VkImageView,
    ) {
        let bi = VkDescriptorBufferInfo {
            buffer,
            offset: 0,
            range: VK_WHOLE_SIZE as u64,
        };
        let ii = VkDescriptorImageInfo {
            sampler,
            imageView: image_view,
            imageLayout: VkImageLayout_VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        let sets = [
            VkWriteDescriptorSet {
                sType: VkStructureType_VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                pNext: null(),
                dstSet: descriptor_set,
                dstBinding: 0,
                dstArrayElement: 0,
                descriptorCount: 1,
                descriptorType: VkDescriptorType_VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                pImageInfo: null(),
                pBufferInfo: &bi,
                pTexelBufferView: null(),
            },
            VkWriteDescriptorSet {
                sType: VkStructureType_VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                pNext: null(),
                dstSet: descriptor_set,
                dstBinding: 1,
                dstArrayElement: 0,
                descriptorCount: 1,
                descriptorType: VkDescriptorType_VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                pImageInfo: &ii,
                pBufferInfo: null(),
                pTexelBufferView: null(),
            },
        ];
        unsafe { vkUpdateDescriptorSets(device, sets.len() as u32, sets.as_ptr(), 0, null()) }
    }
}
