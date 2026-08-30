#pragma once

#include "Framework/GameWorld.h"
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

	class Renderer : public GameUtility
	{
	public:
		Renderer(GameWorld& inWorld, WindowHandle inWindow);

		virtual void PreSystemsUpdate() override;
		virtual void PostSystemsUpdate() override;

		void BeginFrame(RenderFrameData& outFrameData);
		void EndFrame();

		const RenderFrameData& GetCurrentFrameData() const { return mCurrentFrameData; }

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

		vk::SampleCountFlagBits mMsaaSamples = vk::SampleCountFlagBits::e1;

		bool mIsFrameInProgress = false;
		u32 mCurrentImageIndex = mage::InvalidIndex;
		u32 mCurrentFrameIndex = 0;

		RenderFrameData mCurrentFrameData;

		vk::Extent2D mWindowSize;
		bool mShouldRecreateSwapchain = false;
	};
}
