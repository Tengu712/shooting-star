use super::*;

use std::mem::size_of;

pub(super) struct Model {
    pub(super) index_cnt: u32,
    pub(super) vertex_buffer: Buffer,
    pub(super) index_buffer: Buffer,
}

impl Model {
    pub(super) fn new(
        device: VkDevice,
        phys_device_mem_prop: VkPhysicalDeviceMemoryProperties,
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
            VkBufferUsageFlagBits_VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VkMemoryPropertyFlagBits_VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        )?;
        let index_buffer = Buffer::new(
            device,
            phys_device_mem_prop,
            idxs_size as u64,
            VkBufferUsageFlagBits_VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VkMemoryPropertyFlagBits_VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        )?;

        // map memories
        map_memory(
            device,
            vertex_buffer.memory,
            vtxs.as_ptr() as *const c_void,
            vtxs_size as c_ulong,
        )?;
        map_memory(
            device,
            index_buffer.memory,
            idxs.as_ptr() as *const c_void,
            idxs_size as c_ulong,
        )?;

        // finish
        Ok(Self {
            index_cnt: idxs.len() as u32,
            vertex_buffer,
            index_buffer,
        })
    }

    pub(super) fn terminate(self, device: VkDevice) {
        self.index_buffer.terminate(device);
        self.vertex_buffer.terminate(device);
    }
}
