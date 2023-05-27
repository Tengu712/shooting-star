use super::*;

use std::mem::size_of;
use std::os::raw::c_void;

#[repr(C)]
pub(in crate::vulkan) struct Vertex {
    pub(in crate::vulkan) in_pos: [f32; 3],
    pub(in crate::vulkan) in_uv: [f32; 2],
}

pub(in crate::vulkan) struct Model {
    pub(in crate::vulkan) index_cnt: u32,
    pub(in crate::vulkan) vertex_buffer: Buffer,
    pub(in crate::vulkan) index_buffer: Buffer,
}

impl Model {
    pub(in crate::vulkan) fn new(
        device: VkDevice,
        phys_device_mem_prop: &VkPhysicalDeviceMemoryProperties,
        vtxs: &[Vertex],
        idxs: &[u32],
    ) -> Result<Self, String> {
        let vtxs_size = size_of::<Vertex>() * vtxs.len();
        let idxs_size = size_of::<u32>() * idxs.len();

        // create buffers
        let vertex_buffer = Buffer::new(
            device,
            phys_device_mem_prop,
            vtxs_size as u64,
            VkBufferUsageFlagBits_VK_BUFFER_USAGE_VERTEX_BUFFER_BIT as VkBufferUsageFlags,
            VkMemoryPropertyFlagBits_VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT as VkMemoryPropertyFlags,
        )?;
        let index_buffer = Buffer::new(
            device,
            phys_device_mem_prop,
            idxs_size as u64,
            VkBufferUsageFlagBits_VK_BUFFER_USAGE_INDEX_BUFFER_BIT as VkBufferUsageFlags,
            VkMemoryPropertyFlagBits_VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT as VkMemoryPropertyFlags,
        )?;

        // map memories
        map_memory(
            device,
            vertex_buffer.memory,
            vtxs.as_ptr() as *const c_void,
            vtxs_size,
        )?;
        map_memory(
            device,
            index_buffer.memory,
            idxs.as_ptr() as *const c_void,
            idxs_size,
        )?;

        // finish
        Ok(Self {
            index_cnt: idxs.len() as u32,
            vertex_buffer,
            index_buffer,
        })
    }

    pub(in crate::vulkan) fn terminate(self, device: VkDevice) {
        self.index_buffer.terminate(device);
        self.vertex_buffer.terminate(device);
    }
}
