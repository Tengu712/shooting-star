use super::*;

impl VulkanApp {
    pub(super) fn load(
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
