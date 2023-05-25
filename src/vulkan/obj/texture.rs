use super::*;

pub(in crate::vulkan) struct Texture {
    pub(in crate::vulkan) image: VkImage,
    pub(in crate::vulkan) image_view: VkImageView,
    pub(in crate::vulkan) memory: VkDeviceMemory,
}

impl Texture {
    pub(in crate::vulkan) fn new(
        device: VkDevice,
        phys_device_mem_props: &VkPhysicalDeviceMemoryProperties,
        format: VkFormat,
        width: u32,
        height: u32,
        usage: VkImageUsageFlags,
        aspect: VkImageAspectFlags,
    ) -> Result<Self, String> {
        // create an image
        let image = {
            let ci = VkImageCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                imageType: VkImageType_VK_IMAGE_TYPE_2D,
                format,
                extent: VkExtent3D {
                    width,
                    height,
                    depth: 1,
                },
                mipLevels: 1,
                arrayLayers: 1,
                samples: VkSampleCountFlagBits_VK_SAMPLE_COUNT_1_BIT,
                tiling: VkImageTiling_VK_IMAGE_TILING_OPTIMAL,
                usage,
                sharingMode: VkSharingMode_VK_SHARING_MODE_EXCLUSIVE,
                queueFamilyIndexCount: 0,
                pQueueFamilyIndices: null(),
                initialLayout: VkImageLayout_VK_IMAGE_LAYOUT_UNDEFINED,
            };
            let mut image = null_mut();
            check_res!(
                vkCreateImage(device, &ci, null(), &mut image),
                "failed to create an image."
            );
            image
        };

        // create a memory
        let memory = {
            let mut reqs = VkMemoryRequirements::default();
            unsafe { vkGetImageMemoryRequirements(device, image, &mut reqs) };
            let index = get_memory_type_index(
                phys_device_mem_props,
                &reqs,
                VkMemoryPropertyFlagBits_VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            )
            .ok_or(String::from("failed to get a memory type index."))?;
            let ai = VkMemoryAllocateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                pNext: null(),
                allocationSize: reqs.size,
                memoryTypeIndex: index,
            };
            let mut memory = null_mut();
            check_res!(
                vkAllocateMemory(device, &ai, null(), &mut memory),
                "failed to allocate a memory."
            );
            memory
        };

        // bind an image with a memory
        check_res!(
            vkBindImageMemory(device, image, memory, 0),
            "failed to bind an image with a memory."
        );

        // create an image view
        let image_view = {
            let ci = VkImageViewCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                pNext: null(),
                flags: 0,
                image,
                viewType: VkImageViewType_VK_IMAGE_VIEW_TYPE_2D,
                format,
                components: VkComponentMapping {
                    r: VkComponentSwizzle_VK_COMPONENT_SWIZZLE_R,
                    g: VkComponentSwizzle_VK_COMPONENT_SWIZZLE_G,
                    b: VkComponentSwizzle_VK_COMPONENT_SWIZZLE_B,
                    a: VkComponentSwizzle_VK_COMPONENT_SWIZZLE_A,
                },
                subresourceRange: VkImageSubresourceRange {
                    aspectMask: aspect,
                    baseMipLevel: 0,
                    levelCount: 1,
                    baseArrayLayer: 0,
                    layerCount: 1,
                },
            };
            let mut image_view = null_mut();
            check_res!(
                vkCreateImageView(device, &ci, null(), &mut image_view),
                "failed to create an image view."
            );
            image_view
        };

        // finish
        Ok(Self {
            image,
            image_view,
            memory,
        })
    }

    pub(in crate::vulkan) fn terminate(self, device: VkDevice) {
        unsafe {
            vkDestroyImageView(device, self.image_view, null());
            vkFreeMemory(device, self.memory, null());
            vkDestroyImage(device, self.image, null());
        }
    }
}
