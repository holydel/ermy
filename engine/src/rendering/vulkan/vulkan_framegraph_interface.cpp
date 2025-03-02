#include "../framegraph_interface.h"
#ifdef ERMY_GAPI_VULKAN

#include "vulkan_interface.h"
#include <vector>
#include <cassert>
#include "vulkan_swapchain.h"
#include <array>
#include "vk_utils.h"
#include "../../graphics/canvas_interface.h"

using namespace ermy;

namespace framegraph_interface
{
	VkSemaphore gFrameGraphSemaphore = VK_NULL_HANDLE;
	u32			gFrameRingCurrent{ 0 };

	struct FrameData
	{
		VkCommandPool cmdPool = VK_NULL_HANDLE;
		VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
		u64 frameIndex = 0;
	};

	std::vector<FrameData> gFrames;

	void Initialize(u8 numFrames)
	{
		assert(numFrames > 1);

		const uint64_t initialValue = (numFrames - 1);

		VkSemaphoreTypeCreateInfo timelineCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.pNext = nullptr,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = initialValue,
		};

		const VkSemaphoreCreateInfo semaphoreCreateInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = &timelineCreateInfo };
		VK_CALL(vkCreateSemaphore(gVKDevice, &semaphoreCreateInfo, nullptr, &gFrameGraphSemaphore));

		vk_utils::debug::SetName(gFrameGraphSemaphore, "FrameGraph semaphore");

		gFrames.resize(numFrames);

		const VkCommandPoolCreateInfo cmdPoolCreateInfo{
	.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	.queueFamilyIndex = 0, //TODO: get from device
		};

		for (ermy::u8 i = 0; i < numFrames; ++i)
		{
			auto& f = gFrames[i];
			f.frameIndex = i;  // Track frame index for synchronization
			VK_CALL(vkCreateCommandPool(gVKDevice, &cmdPoolCreateInfo, nullptr, &f.cmdPool));

			vk_utils::debug::SetName(f.cmdPool, "Frame Command Pool (%d)",i);

			const VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
				 .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				 .commandPool = f.cmdPool,
				 .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				 .commandBufferCount = 1,
			};
			VK_CALL(vkAllocateCommandBuffers(gVKDevice, &commandBufferAllocateInfo, &f.cmdBuffer));

			vk_utils::debug::SetName(f.cmdBuffer, "Frame Command Buffer (%d)", i);
		}
	}

	void Shutdown()
	{
		vkDestroySemaphore(gVKDevice, gFrameGraphSemaphore, nullptr);
		gFrameGraphSemaphore = VK_NULL_HANDLE;
		for (auto& frame : gFrames)
		{
			vkDestroyCommandPool(gVKDevice, frame.cmdPool, nullptr);
			frame.cmdPool = VK_NULL_HANDLE;
			frame.cmdBuffer = VK_NULL_HANDLE;
		}
		gFrames.clear();
	}

	void* BeginFrame()
	{
		swapchain::ReInitIfNeeded();

		auto& frame = gFrames[gFrameRingCurrent];

		const uint64_t            waitValue = frame.frameIndex;
		const VkSemaphoreWaitInfo waitInfo = {
			 .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			 .semaphoreCount = 1,
			 .pSemaphores = &gFrameGraphSemaphore,
			 .pValues = &waitValue,
		};

		vkWaitSemaphores(gVKDevice, &waitInfo, std::numeric_limits<uint64_t>::max());
		swapchain::AcquireNextImage();

		VK_CALL(vkResetCommandPool(gVKDevice, frame.cmdPool, 0));


		VkCommandBuffer cmd = frame.cmdBuffer;
		// Begin the command buffer recording for the frame
		const VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
												 .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };
		VK_CALL(vkBeginCommandBuffer(cmd, &beginInfo));
		return cmd;
	}

	void BeginFinalRenderPass()
	{
		auto& frame = gFrames[gFrameRingCurrent];
		VkCommandBuffer cmd = frame.cmdBuffer;

		if(gVKConfig.useDynamicRendering)
		{
			VkClearValue clearValue;
			clearValue.color = { canvas_interface::BgColor[0], canvas_interface::BgColor[1], canvas_interface::BgColor[2], canvas_interface::BgColor[3] };

			vk_utils::ImageTransition(frame.cmdBuffer, swapchain::GetCurrentImage(), swapchain::GetCurrentImageLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,VK_IMAGE_ASPECT_COLOR_BIT);
			
			
			VkRenderingAttachmentInfo colorAttachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
			colorAttachment.imageView = swapchain::GetCurrentImageView();
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue = clearValue;

			VkRenderingInfo renderInfo{VK_STRUCTURE_TYPE_RENDERING_INFO};
			renderInfo.renderArea = {{0, 0}, {gVKSurfaceCaps.currentExtent.width, gVKSurfaceCaps.currentExtent.height}};
			renderInfo.layerCount = 1;
			renderInfo.colorAttachmentCount = 1;
			renderInfo.pColorAttachments = &colorAttachment;

			vkCmdBeginRendering(frame.cmdBuffer, &renderInfo);

		}
		else
		{
			VkClearValue clearValues[3] = {};
			int colorAttachmentIndex = 0;

			if (gVKSurfaceDepthFormat != VK_FORMAT_UNDEFINED)
			{
				clearValues[0].depthStencil.depth = 1.0f;
				clearValues[0].depthStencil.stencil = 0;
				colorAttachmentIndex++;
			}

			for(int i = 0;i<4;++i)
				clearValues[colorAttachmentIndex].color.float32[i] = canvas_interface::BgColor[i];
			
			colorAttachmentIndex++;


			VkRenderPassBeginInfo rpass{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
			rpass.clearValueCount = colorAttachmentIndex;
			rpass.pClearValues = clearValues;
			rpass.framebuffer = swapchain::GetFramebuffer();
			rpass.renderArea = { 0,0, gVKSurfaceCaps.currentExtent.width, gVKSurfaceCaps.currentExtent.height };
			rpass.renderPass = swapchain::GetRenderPass();

			vkCmdBeginRenderPass(frame.cmdBuffer, &rpass, VK_SUBPASS_CONTENTS_INLINE);
		}
	}

	void EndFinalRenderPass()
	{
		auto& frame = gFrames[gFrameRingCurrent];
		VkCommandBuffer cmd = frame.cmdBuffer;

		
		if(gVKConfig.useDynamicRendering)
		{
			vkCmdEndRendering(frame.cmdBuffer);
			vk_utils::ImageTransition(frame.cmdBuffer, swapchain::GetCurrentImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);
		}
		else
		{
		vkCmdEndRenderPass(frame.cmdBuffer);
		}
		
	}

	void EndFrame()
	{
		auto& frame = gFrames[gFrameRingCurrent];
		VkCommandBuffer cmd = frame.cmdBuffer;
		VK_CALL(vkEndCommandBuffer(cmd));
	}

	void Submit()
	{
		auto& frame = gFrames[gFrameRingCurrent];
		VkCommandBuffer cmd = frame.cmdBuffer;
		/*--
		 * Prepare to submit the current frame for rendering
		 * First add the swapchain semaphore to wait for the image to be available.
		-*/
		std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
		std::vector<VkSemaphoreSubmitInfo> signalSemaphores;
		waitSemaphores.push_back({
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = swapchain::GetWaitSemaphores(),
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			});
		signalSemaphores.push_back({
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = swapchain::GetSignalSemaphores(),
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			});

		/*--
		 * Calculate the signal value for when this frame completes
		 * Signal value = current frame number + numFramesInFlight
		 * Example with 3 frames in flight:
		 *   Frame 0 signals value 3 (allowing Frame 3 to start when complete)
		 *   Frame 1 signals value 4 (allowing Frame 4 to start when complete)
		-*/
		const uint64_t signalFrameValue = frame.frameIndex + swapchain::GetNumFrames();
		frame.frameIndex = signalFrameValue;  // Store for next time this frame buffer is used

		/*--
		 * Add timeline semaphore to signal when GPU completes this frame
		 * The color attachment output stage is used since that's when the frame is fully rendered
		-*/
		signalSemaphores.push_back({
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = gFrameGraphSemaphore,
			.value = signalFrameValue,
			.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			});

		// Note : in this sample, we only have one command buffer per frame.
		const std::array<VkCommandBufferSubmitInfo, 1> cmdBufferInfo{ {{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = cmd,
		}} };

		// Populate the submit info to synchronize rendering and send the command buffer
		const std::array<VkSubmitInfo2, 1> submitInfo{ {{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = uint32_t(waitSemaphores.size()),    //
			.pWaitSemaphoreInfos = waitSemaphores.data(),              // Wait for the image to be available
			.commandBufferInfoCount = uint32_t(cmdBufferInfo.size()),     //
			.pCommandBufferInfos = cmdBufferInfo.data(),               // Command buffer to submit
			.signalSemaphoreInfoCount = uint32_t(signalSemaphores.size()),  //
			.pSignalSemaphoreInfos = signalSemaphores.data(),            // Signal when rendering is finished
		}} };

		// Submit the command buffer to the GPU and signal when it's done
		VK_CALL(vkQueueSubmit2(gVKMainQueue, uint32_t(submitInfo.size()), submitInfo.data(), nullptr));
	}

	void Present()
	{
		auto& frame = gFrames[gFrameRingCurrent];
		VkCommandBuffer cmd = frame.cmdBuffer;
		// Present the image
		swapchain::Present();

		// Move to the next frame
		gFrameRingCurrent = (gFrameRingCurrent + 1) % swapchain::GetNumFrames();
	}
};
#endif