use super::*;

impl VulkanApp {
    pub fn terminate(self) {
        unsafe {
            vkDeviceWaitIdle(self.device);
            for framebuffer in self.framebuffers {
                vkDestroyFramebuffer(self.device, framebuffer, null());
            }
            vkDestroyRenderPass(self.device, self.render_pass, null());
            for image_view in self.image_views {
                vkDestroyImageView(self.device, image_view, null());
            }
            vkDestroySwapchainKHR(self.device, self.swapchain, null());
            vkDestroySurfaceKHR(self.instance, self.surface, null());
            vkDestroySemaphore(self.device, self.signal_semaphore, null());
            vkDestroySemaphore(self.device, self.wait_semaphore, null());
            vkDestroyFence(self.device, self.fence, null());
            vkFreeCommandBuffers(self.device, self.command_pool, 1, &self.command_buffer);
            vkDestroyCommandPool(self.device, self.command_pool, null());
            vkDestroyDevice(self.device, null());
            vkDestroyInstance(self.instance, null());
        }
    }
}
