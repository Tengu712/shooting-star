use super::*;

impl VulkanApp {
    pub fn render(&mut self) {
        // prepare

        // get current image index
        let mut cur_img_idx = 0;
        check_warn!(
            vkAcquireNextImageKHR(
                self.device,
                self.swapchain,
                u64::MAX,
                self.frames[self.pre_img_idx].semaphore, // XXX: what is it?
                null_mut(),
                &mut cur_img_idx
            ),
            "failed to aquire the next image index."
        );
        let cur_img_idx = cur_img_idx as usize;
        let pre_img_idx = self.pre_img_idx;
        let command = self.frames[cur_img_idx].command_buffer;

        // reset frame data
        let fences = [self.frames[cur_img_idx].fence];
        check_warn!(
            vkWaitForFences(
                self.device,
                fences.len() as u32,
                fences.as_ptr(),
                VK_TRUE,
                u64::MAX
            ),
            "failed to wait for a fence."
        );
        check_warn!(
            vkResetFences(self.device, fences.len() as u32, fences.as_ptr()),
            "failed to reset a fence."
        );
        check_warn!(
            vkResetCommandBuffer(
                command,
                VkCommandBufferResetFlagBits_VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
            ),
            "failed to reset a command buffer."
        );

        // begin

        // to record commands
        let cmd_bi = VkCommandBufferBeginInfo {
            sType: VkStructureType_VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            pNext: null(),
            flags: 0,
            pInheritanceInfo: null(),
        };
        check_warn!(
            vkBeginCommandBuffer(command, &cmd_bi),
            "failed to begin to record commands."
        );

        // render pass
        let clear_values = [VkClearValue {
            color: VkClearColorValue {
                float32: [0.25, 0.25, 0.25, 1.0],
            },
        }];
        let rp_bi = VkRenderPassBeginInfo {
            sType: VkStructureType_VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            pNext: null(),
            renderPass: self.render_pass,
            framebuffer: self.framebuffers[cur_img_idx],
            renderArea: VkRect2D {
                offset: VkOffset2D::default(),
                extent: self.surface_capabilities.currentExtent,
            },
            clearValueCount: clear_values.len() as u32,
            pClearValues: clear_values.as_ptr(),
        };
        unsafe {
            vkCmdBeginRenderPass(
                command,
                &rp_bi,
                VkSubpassContents_VK_SUBPASS_CONTENTS_INLINE,
            )
        };

        // end

        // end to record commands
        unsafe { vkCmdEndRenderPass(command) };
        unsafe { vkEndCommandBuffer(command) };

        // submit
        let commands = [command];
        // XXX: what should i pass?
        let wait_semaphores = [self.frames[pre_img_idx].semaphore];
        let signal_semaphores = [self.frames[cur_img_idx].semaphore];
        let sis = [VkSubmitInfo {
            sType: VkStructureType_VK_STRUCTURE_TYPE_SUBMIT_INFO,
            pNext: null(),
            waitSemaphoreCount: wait_semaphores.len() as u32,
            pWaitSemaphores: wait_semaphores.as_ptr(),
            pWaitDstStageMask:
                &VkPipelineStageFlagBits_VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            commandBufferCount: commands.len() as u32,
            pCommandBuffers: commands.as_ptr(),
            signalSemaphoreCount: signal_semaphores.len() as u32,
            pSignalSemaphores: signal_semaphores.as_ptr(),
        }];
        check_warn!(
            vkQueueSubmit(
                self.queue,
                sis.len() as u32,
                sis.as_ptr(),
                self.frames[cur_img_idx].fence
            ),
            "failed to submit a command buffer."
        );

        // present
        let mut res = 0;
        let swapchains = [self.swapchain];
        let image_indices = [cur_img_idx as u32];
        let pi = VkPresentInfoKHR {
            sType: VkStructureType_VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            pNext: null(),
            waitSemaphoreCount: signal_semaphores.len() as u32,
            pWaitSemaphores: signal_semaphores.as_ptr(),
            swapchainCount: swapchains.len() as u32,
            pSwapchains: swapchains.as_ptr(),
            pImageIndices: image_indices.as_ptr(),
            pResults: &mut res,
        };
        check_warn!(
            vkQueuePresentKHR(self.queue, &pi),
            "failed to enqueue a present queue."
        );
        check_warn!(res, "failed to present.");

        // finish
        self.pre_img_idx = cur_img_idx;
    }
}
