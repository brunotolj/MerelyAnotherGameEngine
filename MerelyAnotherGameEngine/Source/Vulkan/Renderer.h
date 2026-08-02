#pragma once

#include "Vulkan/Image.h"

#include <vulkan/vulkan_raii.hpp>

class WindowHandle;

namespace Vulkan
{
	struct RenderFrameData
	{
		vk::CommandBuffer CommandBuffer;
		vk::Extent2D Extent;
		u32 Index;
	};

	class Renderer : public NonMovableClass
	{
	public:
		Renderer(WindowHandle inWindow);

		using RenderFrameFunction = std::function<void(RenderFrameData const&)>;
		void RenderFrame(RenderFrameFunction&& inFunction);

		using SingleTimeCommandsFunction = std::function<void(vk::CommandBuffer)>;

		static constexpr u32 cMaxFramesInFlight = 2;

	private:
		bool IsWindowSizeValid() const;
		void RecreateSwapchain();

		void InitializeDynamicState(vk::CommandBuffer inCommandBuffer);

		vk::SurfaceFormatKHR ChooseSwapchainFormat(mage::Array<vk::SurfaceFormatKHR> const& inFormats) const;
		vk::PresentModeKHR ChooseSwapchainPresentMode(mage::Array<vk::PresentModeKHR> const& inPresentModes) const;
		vk::Extent2D ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR const& inCapabilities, vk::Extent2D inWindowExtent) const;
		u32 ChooseSwapchainMinImageCount(vk::SurfaceCapabilitiesKHR const& inCapabilities) const;

		vk::raii::SurfaceKHR mSurface = nullptr;

		mage::Array<vk::raii::CommandBuffer> mCommandBuffers;

		mage::Array<vk::raii::Semaphore> mPresentCompleteSemaphores;
		mage::Array<vk::raii::Semaphore> mRenderFinishedSemaphores;
		mage::Array<vk::raii::Fence> mDrawFences;
		mage::Array<vk::raii::Fence> mPresentFences;

		vk::SurfaceFormatKHR mSwapchainSurfaceFormat;
		vk::Extent2D mSwapchainExtent;

		vk::raii::SwapchainKHR mSwapchain = nullptr;
		mage::Array<vk::Image> mSwapchainImages;
		mage::Array<vk::raii::ImageView> mSwapchainImageViews;
		
		Image mColorImage = nullptr;
		Image mDepthImage = nullptr;

		vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

		u32 mCurrentImageIndex = mage::InvalidIndex;
		u32 mCurrentFrameIndex = 0;

		vk::Extent2D mWindowSize;
		bool mShouldRecreateSwapchain = false;
	};
}
