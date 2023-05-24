use super::*;

use std::os::raw::{c_ulong, c_void};

pub(super) fn get_memory_type_index(
    phys_device_mem_props: &VkPhysicalDeviceMemoryProperties,
    reqs: &VkMemoryRequirements,
    flags: VkMemoryPropertyFlags,
) -> Option<u32> {
    phys_device_mem_props
        .memoryTypes
        .iter()
        .enumerate()
        .position(|(i, n)| reqs.memoryTypeBits & (1 << i) > 0 && n.propertyFlags & flags > 0)
        .map(|i| i as u32)
}

pub(super) fn map_memory(
    device: VkDevice,
    memory: VkDeviceMemory,
    data: *const c_void,
    size: c_ulong,
) -> Result<(), String> {
    let mut p = null_mut();
    check_res!(
        vkMapMemory(device, memory, 0, u64::MAX, 0, &mut p),
        "failed to map a memory."
    );
    unsafe { memcpy(p, data, size) };
    unsafe { vkUnmapMemory(device, memory) };
    Ok(())
}

pub(super) fn copy_memory(
    device: VkDevice,
    phys_device_mem_props: &VkPhysicalDeviceMemoryProperties,
    queue: VkQueue,
    command_pool: VkCommandPool,
    width: u32,
    height: u32,
    image: VkImage,
    size: VkDeviceSize,
    data: *const c_void,
) -> Result<(), String> {
    // create a staging buffer and map data
    let staging = Buffer::new(
        device,
        phys_device_mem_props,
        size,
        VkBufferUsageFlagBits_VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VkMemoryPropertyFlagBits_VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    )?;
    map_memory(device, staging.memory, data, size)?;

    // allocate a command buffer
    let command_buffer = {
        let ai = VkCommandBufferAllocateInfo {
            sType: VkStructureType_VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            pNext: null(),
            commandPool: command_pool,
            level: VkCommandBufferLevel_VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            commandBufferCount: 1,
        };
        let mut command_buffer = null_mut();
        check_res!(
            vkAllocateCommandBuffers(device, &ai, &mut command_buffer),
            "failed to allocate a command buffer."
        );
        command_buffer
    };

    // begin the command buffer
    let bi = VkCommandBufferBeginInfo {
        sType: VkStructureType_VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        pNext: null(),
        flags: 0,
        pInheritanceInfo: null(),
    };
    check_res!(
        vkBeginCommandBuffer(command_buffer, &bi),
        "failed to begin a command buffer for copying command."
    );

    //
    let barriers = [VkImageMemoryBarrier {
        sType: VkStructureType_VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        pNext: null(),
        srcAccessMask: 0,
        dstAccessMask: VkAccessFlagBits_VK_ACCESS_TRANSFER_WRITE_BIT,
        oldLayout: VkImageLayout_VK_IMAGE_LAYOUT_UNDEFINED,
        newLayout: VkImageLayout_VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        srcQueueFamilyIndex: VK_QUEUE_FAMILY_IGNORED as u32,
        dstQueueFamilyIndex: VK_QUEUE_FAMILY_IGNORED as u32,
        image,
        subresourceRange: VkImageSubresourceRange {
            aspectMask: VkImageAspectFlagBits_VK_IMAGE_ASPECT_COLOR_BIT,
            baseMipLevel: 0,
            levelCount: 1,
            baseArrayLayer: 0,
            layerCount: 1,
        },
    }];
    unsafe {
        vkCmdPipelineBarrier(
            command_buffer,
            VkPipelineStageFlagBits_VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VkPipelineStageFlagBits_VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            null(),
            0,
            null(),
            barriers.len() as u32,
            barriers.as_ptr(),
        )
    };

    //
    let copy_regions = [VkBufferImageCopy {
        bufferOffset: 0,
        bufferRowLength: 0,
        bufferImageHeight: 0,
        imageSubresource: VkImageSubresourceLayers {
            aspectMask: VkImageAspectFlagBits_VK_IMAGE_ASPECT_COLOR_BIT,
            mipLevel: 0,
            baseArrayLayer: 0,
            layerCount: 1,
        },
        imageOffset: VkOffset3D { x: 0, y: 0, z: 0 },
        imageExtent: VkExtent3D {
            width,
            height,
            depth: 1,
        },
    }];
    unsafe {
        vkCmdCopyBufferToImage(
            command_buffer,
            staging.buffer,
            image,
            VkImageLayout_VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            copy_regions.len() as u32,
            copy_regions.as_ptr(),
        )
    };

    //
    let barriers = [VkImageMemoryBarrier {
        sType: VkStructureType_VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        pNext: null(),
        srcAccessMask: VkAccessFlagBits_VK_ACCESS_TRANSFER_WRITE_BIT,
        dstAccessMask: VkAccessFlagBits_VK_ACCESS_SHADER_READ_BIT,
        oldLayout: VkImageLayout_VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        newLayout: VkImageLayout_VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        srcQueueFamilyIndex: VK_QUEUE_FAMILY_IGNORED as u32,
        dstQueueFamilyIndex: VK_QUEUE_FAMILY_IGNORED as u32,
        image,
        subresourceRange: VkImageSubresourceRange {
            aspectMask: VkImageAspectFlagBits_VK_IMAGE_ASPECT_COLOR_BIT,
            baseMipLevel: 0,
            levelCount: 1,
            baseArrayLayer: 0,
            layerCount: 1,
        },
    }];
    unsafe {
        vkCmdPipelineBarrier(
            command_buffer,
            VkPipelineStageFlagBits_VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkPipelineStageFlagBits_VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0,
            null(),
            0,
            null(),
            barriers.len() as u32,
            barriers.as_ptr(),
        )
    };

    // end the command buffer
    unsafe { vkEndCommandBuffer(command_buffer) };

    // submit the command buffer
    let commands = [command_buffer];
    let sis = [VkSubmitInfo {
        sType: VkStructureType_VK_STRUCTURE_TYPE_SUBMIT_INFO,
        pNext: null(),
        waitSemaphoreCount: 0,
        pWaitSemaphores: null(),
        pWaitDstStageMask: null(),
        commandBufferCount: commands.len() as u32,
        pCommandBuffers: commands.as_ptr(),
        signalSemaphoreCount: 0,
        pSignalSemaphores: null(),
    }];
    check_res!(
        vkQueueSubmit(queue, sis.len() as u32, sis.as_ptr(), null_mut()),
        "failed to submit a command buffer for copying texture."
    );

    // wait
    unsafe { vkDeviceWaitIdle(device) };

    // finish
    unsafe {
        vkFreeCommandBuffers(
            device,
            command_pool,
            commands.len() as u32,
            commands.as_ptr(),
        )
    };
    staging.terminate(device);
    Ok(())
}
