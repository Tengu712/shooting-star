use super::*;

impl VulkanApp {
    pub fn terminate(self) {
        unsafe {
            vkDeviceWaitIdle(self.device);

            // resources
            self.square.terminate(self.device);
            self.uniform_buffer.terminate(self.device);

            // pipeline
            vkDestroyPipeline(self.device, self.pipeline, null());
            vkDestroyPipelineLayout(self.device, self.pipeline_layout, null());
            vkDestroyDescriptorPool(self.device, self.descriptor_pool, null());
            vkDestroyDescriptorSetLayout(self.device, self.descriptor_set_layout, null());
            vkDestroySampler(self.device, self.sampler, null());
            vkDestroyShaderModule(self.device, self.frag_shader, null());
            vkDestroyShaderModule(self.device, self.vert_shader, null());

            // renderer
            for framebuffer in self.framebuffers {
                vkDestroyFramebuffer(self.device, framebuffer, null());
            }
            vkDestroyRenderPass(self.device, self.render_pass, null());
            for image_view in self.image_views {
                vkDestroyImageView(self.device, image_view, null());
            }
            vkDestroySwapchainKHR(self.device, self.swapchain, null());
            vkDestroySurfaceKHR(self.instance, self.surface, null());

            // command
            vkDestroySemaphore(self.device, self.signal_semaphore, null());
            vkDestroySemaphore(self.device, self.wait_semaphore, null());
            vkDestroyFence(self.device, self.fence, null());
            vkFreeCommandBuffers(self.device, self.command_pool, 1, &self.command_buffer);
            vkDestroyCommandPool(self.device, self.command_pool, null());

            // core
            vkDestroyDevice(self.device, null());
            vkDestroyInstance(self.instance, null());
        }
    }
}
