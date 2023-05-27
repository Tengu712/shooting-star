use super::*;

use std::mem::size_of;
use std::os::raw::c_void;

pub fn load_image_texture(
    vulkan_app: &mut VulkanApp,
    id: usize,
    width: u32,
    height: u32,
    pixels: &[u8],
) -> Result<(), String> {
    if width * height * 4 != pixels.len() as u32 {
        return Err(format!(
            "width {width} and height {height} passed so pixels array length expected {} but passed {}.",
            width * height * 4,
            pixels.len(),
        ));
    }
    let flags = VkImageUsageFlagBits_VK_IMAGE_USAGE_TRANSFER_DST_BIT
        | VkImageUsageFlagBits_VK_IMAGE_USAGE_SAMPLED_BIT;
    let img_tex = Texture::new(
        vulkan_app.device,
        &vulkan_app.phys_device_mem_props,
        VkFormat_VK_FORMAT_R8G8B8A8_UNORM,
        width,
        height,
        flags as VkImageUsageFlags,
        VkImageAspectFlagBits_VK_IMAGE_ASPECT_COLOR_BIT as VkImageAspectFlags,
    )?;
    copy_memory(
        vulkan_app.device,
        &vulkan_app.phys_device_mem_props,
        vulkan_app.queue,
        vulkan_app.command_pool,
        width,
        height,
        img_tex.image,
        size_of::<u8>() * pixels.len(),
        pixels.as_ptr() as *const c_void,
    )?;
    vulkan_app.load(id, img_tex)?;
    Ok(())
}
