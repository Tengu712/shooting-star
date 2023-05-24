use super::*;

pub(super) struct Buffer {
    pub(super) buffer: VkBuffer,
    pub(super) memory: VkDeviceMemory,
}

impl Buffer {
    pub(super) fn new(
        device: VkDevice,
        phys_device_mem_prop: VkPhysicalDeviceMemoryProperties,
        size: VkDeviceSize,
        usage: VkBufferUsageFlags,
        flags: VkMemoryPropertyFlags,
    ) -> Result<Self, String> {
        // create a buffer
        let buffer = {
            let ci = VkBufferCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                pNext: null(),
                flags: 0,
                size,
                usage,
                sharingMode: VkSharingMode_VK_SHARING_MODE_EXCLUSIVE,
                queueFamilyIndexCount: 0,
                pQueueFamilyIndices: null(),
            };
            let mut buffer = null_mut();
            check_res!(
                vkCreateBuffer(device, &ci, null(), &mut buffer),
                "failed to create a buffer."
            );
            buffer
        };

        // get memory requirements
        let reqs = {
            let mut reqs = VkMemoryRequirements::default();
            unsafe { vkGetBufferMemoryRequirements(device, buffer, &mut reqs) };
            reqs
        };

        // allocate memory
        let memory = {
            let index = get_memory_type_index(phys_device_mem_prop, &reqs, flags)
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

        // bind buffer with memory
        check_res!(
            vkBindBufferMemory(device, buffer, memory, 0),
            "failed to bind a buffer with a memory."
        );

        // finish
        Ok(Self { buffer, memory })
    }

    pub(super) fn terminate(self, device: VkDevice) {
        unsafe {
            vkFreeMemory(device, self.memory, null());
            vkDestroyBuffer(device, self.buffer, null());
        }
    }
}
