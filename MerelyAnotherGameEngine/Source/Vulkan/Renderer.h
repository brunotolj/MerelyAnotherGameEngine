#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Buffer;
	class DescriptorSetLayout;
	class Instance;
	class Pipeline;
	class Texture;
	class Window;
	struct BufferCreateInfo;
	struct PipelineCreateInfo;
	struct TextureCreateInfo;

	struct TransitionImageLayoutParams
	{
		vk::PipelineStageFlags2 SrcStageMask;
		vk::AccessFlags2 SrcAccessMask;
		vk::PipelineStageFlags2 DstStageMask;
		vk::AccessFlags2 DstAccessMask;
		vk::ImageLayout OldLayout;
		vk::ImageLayout NewLayout;
		vk::ImageAspectFlags AspectMask;
	};

	struct RenderFrameData
	{
		vk::CommandBuffer CommandBuffer;
		vk::Extent2D Extent;
		u32 Index;
	};

	class Renderer : public NonMovableClass
	{
	public:
		Renderer(Instance const& inInstance, Window& inWindow);

		using RenderFrameFunction = std::function<void(RenderFrameData const&)>;
		void RenderFrame(RenderFrameFunction&& inFunction);

		using SingleTimeCommandsFunction = std::function<void(vk::CommandBuffer)>;
		void SubmitSingleTimeCommands(SingleTimeCommandsFunction&& inFunction) const;

		void WaitIdle() const;

		Pipeline CreatePipeline(PipelineCreateInfo const& inPipelineCreateInfo) const;
		Buffer CreateBuffer(BufferCreateInfo const& inBufferCreateInfo) const;
		Texture CreateTexture(TextureCreateInfo const& inTextureCreateInfo) const;

		void CopyBuffer(vk::CommandBuffer inCommandBuffer, Buffer const& inSrcBuffer, Buffer const& inDstBuffer, vk::DeviceSize inSize) const;
		void CopyBufferToImage(vk::CommandBuffer inCommandBufer, Buffer const& inSrcBuffer, vk::Image inDstImage, vk::Extent3D inImageSize) const;
		void TransitionImageLayout(vk::CommandBuffer inCommandBuffer, vk::Image inImage, TransitionImageLayoutParams inParams) const;

		mage::Array<cstr> GetRequiredDeviceExtensions() const;

		u32 SelectMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties) const;

		static constexpr u32 cMaxFramesInFlight = 2;

	private:
		vk::raii::PhysicalDevice PickPhysicalDevice(Instance const& inInstance) const;
		void SetupGraphicsQueue();
		void RecreateSwapchain();

		vk::SurfaceFormatKHR ChooseSwapchainFormat(mage::Array<vk::SurfaceFormatKHR> const& inFormats) const;
		vk::PresentModeKHR ChooseSwapchainPresentMode(mage::Array<vk::PresentModeKHR> const& inPresentModes) const;
		vk::Extent2D ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR const& inCapabilities, vk::Extent2D inWindowExtent) const;
		u32 ChooseSwapchainMinImageCount(vk::SurfaceCapabilitiesKHR const& inCapabilities) const;

		vk::raii::PhysicalDevice mPhysicalDevice = nullptr;
		vk::raii::SurfaceKHR mSurface = nullptr;

		vk::raii::Device mDevice = nullptr;
		vk::raii::Queue mGraphicsQueue = nullptr;

		vk::raii::CommandPool mCommandPool = nullptr;
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
		
		vk::raii::Image mDepthImage = nullptr;
		vk::raii::DeviceMemory mDepthImageMemory = nullptr;
		vk::raii::ImageView mDepthImageView = nullptr;

		u32 mCurrentImageIndex = u32(-1);
		u32 mCurrentFrameIndex = 0;

		vk::Extent2D mWindowSize;
		bool mWasWindowResized = false;

	};
}
