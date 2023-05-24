use super::*;

use std::mem::size_of;

impl VulkanApp {
    pub(crate) fn render(&self) -> Result<(), String> {
        // ========================================================================================================= //
        //     prepare                                                                                               //
        // ========================================================================================================= //

        // get current image index
        let mut img_idx = 0;
        check_res!(
            vkAcquireNextImageKHR(
                self.device,
                self.swapchain,
                u64::MAX,
                self.wait_semaphore,
                null_mut(),
                &mut img_idx
            ),
            "failed to aquire the next image index."
        );
        let img_idx = img_idx;

        // reset frame data
        let fences = [self.fence];
        check_res!(
            vkWaitForFences(
                self.device,
                fences.len() as u32,
                fences.as_ptr(),
                VK_TRUE,
                u64::MAX
            ),
            "failed to wait for a fence."
        );
        check_res!(
            vkResetFences(self.device, fences.len() as u32, fences.as_ptr()),
            "failed to reset a fence."
        );
        check_res!(
            vkResetCommandBuffer(
                self.command_buffer,
                VkCommandBufferResetFlagBits_VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
            ),
            "failed to reset a command buffer."
        );

        // ========================================================================================================= //
        //     begin                                                                                                 //
        // ========================================================================================================= //

        // to record commands
        let cmd_bi = VkCommandBufferBeginInfo {
            sType: VkStructureType_VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            pNext: null(),
            flags: 0,
            pInheritanceInfo: null(),
        };
        check_res!(
            vkBeginCommandBuffer(self.command_buffer, &cmd_bi),
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
            framebuffer: self.framebuffers[img_idx as usize],
            renderArea: VkRect2D {
                offset: VkOffset2D::default(),
                extent: self.surface_capabilities.currentExtent,
            },
            clearValueCount: clear_values.len() as u32,
            pClearValues: clear_values.as_ptr(),
        };
        unsafe {
            vkCmdBeginRenderPass(
                self.command_buffer,
                &rp_bi,
                VkSubpassContents_VK_SUBPASS_CONTENTS_INLINE,
            )
        };

        // pipeline
        unsafe {
            vkCmdBindPipeline(
                self.command_buffer,
                VkPipelineBindPoint_VK_PIPELINE_BIND_POINT_GRAPHICS,
                self.pipeline,
            )
        };

        // ========================================================================================================= //
        //     draw                                                                                                  //
        // ========================================================================================================= //

        // bind a square
        let offset = 0;
        unsafe {
            vkCmdBindVertexBuffers(
                self.command_buffer,
                0,
                1,
                &self.square.vertex_buffer.buffer,
                &offset,
            )
        };
        unsafe {
            vkCmdBindIndexBuffer(
                self.command_buffer,
                self.square.index_buffer.buffer,
                offset,
                VkIndexType_VK_INDEX_TYPE_UINT32,
            )
        };

        // draw
        let push_constant = PushConstant {
            scl: [1.0, 1.0, 1.0, 1.0],
            rot: [0.0, 0.0, 0.0, 0.0],
            trs: [0.0, 0.0, 0.0, 0.0],
            col: [1.0, 1.0, 1.0, 1.0],
            uv: [0.0, 0.0, 0.0, 0.0],
            param: 0,
        };
        unsafe {
            vkCmdPushConstants(
                self.command_buffer,
                self.pipeline_layout,
                VkShaderStageFlagBits_VK_SHADER_STAGE_VERTEX_BIT,
                0,
                size_of::<PushConstant>() as u32,
                &push_constant as *const _ as *const c_void,
            )
        };
        unsafe { vkCmdDrawIndexed(self.command_buffer, self.square.index_cnt, 1, 0, 0, 0) };

        // ========================================================================================================= //
        //     end                                                                                                   //
        // ========================================================================================================= //

        // end to record commands
        unsafe { vkCmdEndRenderPass(self.command_buffer) };
        unsafe { vkEndCommandBuffer(self.command_buffer) };

        // submit
        let commands = [self.command_buffer];
        let wait_semaphores = [self.wait_semaphore];
        let signal_semaphores = [self.signal_semaphore];
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
        check_res!(
            vkQueueSubmit(self.queue, sis.len() as u32, sis.as_ptr(), self.fence,),
            "failed to submit a command buffer."
        );

        // present
        let mut res = 0;
        let swapchains = [self.swapchain];
        let pi = VkPresentInfoKHR {
            sType: VkStructureType_VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            pNext: null(),
            waitSemaphoreCount: signal_semaphores.len() as u32,
            pWaitSemaphores: signal_semaphores.as_ptr(),
            swapchainCount: swapchains.len() as u32,
            pSwapchains: swapchains.as_ptr(),
            pImageIndices: [img_idx].as_ptr(),
            pResults: &mut res,
        };
        check_res!(
            vkQueuePresentKHR(self.queue, &pi),
            "failed to enqueue a present queue."
        );
        check_res!(res, "failed to present.");

        Ok(())
    }
}
