use super::*;

impl VulkanApp {
    pub fn terminate(self) {
        unsafe {
            vkDeviceWaitIdle(self.device);
            for frame in self.frames {
                vkDestroySemaphore(self.device, frame.semaphore, null());
                vkDestroyFence(self.device, frame.fence, null());
                vkFreeCommandBuffers(self.device, self.command_pool, 1, &frame.command_buffer);
            }
            for framebuffer in self.framebuffers {
                vkDestroyFramebuffer(self.device, framebuffer, null());
            }
            vkDestroyRenderPass(self.device, self.render_pass, null());
            for image_view in self.image_views {
                vkDestroyImageView(self.device, image_view, null());
            }
            vkDestroySwapchainKHR(self.device, self.swapchain, null());
            vkDestroySurfaceKHR(self.instance, self.surface, null());
            vkDestroyCommandPool(self.device, self.command_pool, null());
            vkDestroyDevice(self.device, null());
            vkDestroyInstance(self.instance, null());
        }
    }
}
