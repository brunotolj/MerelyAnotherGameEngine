#pragma once

#include "Asserts.h"
#include "MV/MV_Device.h"
#include "MV/MV_Swapchain.h"
#include "MV/MV_Window.h"

#include <array>
#include <memory>

namespace MV
{
	class Renderer : public NonCopyableClass
	{
	public:
		Renderer(Window& window, Device& device);

		~Renderer();

		VkCommandBuffer BeginFrame();

		void EndFrame();

		void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);

		void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

		bool IsFrameInProgress() const { return privIsFrameInProgress; }

		VkCommandBuffer GetCurrentCommandBuffer() const;

		int32_t GetCurrentFrameIndex() const;

		VkRenderPass GetSwapchainRenderPass() const;

	private:
		MV::Window& privWindow;
		MV::Device& privDevice;
		std::unique_ptr<MV::Swapchain> privSwapchain;
		std::vector<VkCommandBuffer> privCommandBuffers;

		uint32_t privCurrentImageIndex = -1;
		int32_t privCurrentFrameIndex = 0;
		bool privIsFrameInProgress = false;

		void CreateCommandBuffers();

		void FreeCommandBuffers();

		void RecreateSwapchain();
	};
}