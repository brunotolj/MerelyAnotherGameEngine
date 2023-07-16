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

		bool IsFrameInProgress() const { return mIsFrameInProgress; }

		VkCommandBuffer GetCurrentCommandBuffer() const;

		int32_t GetCurrentFrameIndex() const;

		VkRenderPass GetSwapchainRenderPass() const;

	private:
		MV::Window& mWindow;
		MV::Device& mDevice;
		std::unique_ptr<MV::Swapchain> mSwapchain;
		std::vector<VkCommandBuffer> mCommandBuffers;

		uint32_t mCurrentImageIndex = -1;
		int32_t mCurrentFrameIndex = 0;
		bool mIsFrameInProgress = false;

	private:
		void CreateCommandBuffers();

		void FreeCommandBuffers();

		void RecreateSwapchain();
	};
}