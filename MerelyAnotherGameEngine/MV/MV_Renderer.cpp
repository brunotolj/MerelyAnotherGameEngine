#include "MV/MV_Renderer.h"

MV::Renderer::Renderer(Window& window, Device& device) :
	mWindow(window), mDevice(device)
{
	RecreateSwapchain();
	CreateCommandBuffers();
}

MV::Renderer::~Renderer()
{
	FreeCommandBuffers();
}

VkCommandBuffer MV::Renderer::BeginFrame()
{
	check(!mIsFrameInProgress);

	VkResult result = mSwapchain->acquireNextImage(&mCurrentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
		return nullptr;
	}

	check(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

	mIsFrameInProgress = true;

	VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	check(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS);

	return commandBuffer;
}

void MV::Renderer::EndFrame()
{
	check(mIsFrameInProgress);

	VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

	mIsFrameInProgress = false;

	check(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS);

	VkResult result = mSwapchain->submitCommandBuffers(&commandBuffer, &mCurrentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mWindow.WasWindowResized())
	{
		mWindow.ResetWindowResizedFlag();
		RecreateSwapchain();
	}
	else
	{
		check(result == VK_SUCCESS);
	}

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mCommandBuffers.size();
}

void MV::Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	check(mIsFrameInProgress);
	check(commandBuffer == GetCurrentCommandBuffer());

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mSwapchain->getRenderPass();
	renderPassInfo.framebuffer = mSwapchain->getFrameBuffer(mCurrentImageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = mSwapchain->getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(mSwapchain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(mSwapchain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{ { 0,0 }, mSwapchain->getSwapChainExtent() };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void MV::Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	check(mIsFrameInProgress);
	check(commandBuffer == GetCurrentCommandBuffer());

	vkCmdEndRenderPass(commandBuffer);
}

VkCommandBuffer MV::Renderer::GetCurrentCommandBuffer() const
{
	check(mIsFrameInProgress);
	return mCommandBuffers[mCurrentFrameIndex];
}

int32_t MV::Renderer::GetCurrentFrameIndex() const
{
	check(mIsFrameInProgress);
	return mCurrentFrameIndex;
}

VkRenderPass MV::Renderer::GetSwapchainRenderPass() const
{
	return mSwapchain->getRenderPass();
}

void MV::Renderer::CreateCommandBuffers()
{
	mCommandBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mDevice.GetCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

	check(vkAllocateCommandBuffers(mDevice.GetDevice(), &allocInfo, mCommandBuffers.data()) == VK_SUCCESS);
}

void MV::Renderer::FreeCommandBuffers()
{
	vkFreeCommandBuffers(
		mDevice.GetDevice(),
		mDevice.GetCommandPool(),
		static_cast<uint32_t>(mCommandBuffers.size()),
		mCommandBuffers.data());
}

void MV::Renderer::RecreateSwapchain()
{
	VkExtent2D extent = mWindow.GetExtent();

	while (extent.width == 0 || extent.height == 0)
	{
		extent = mWindow.GetExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(mDevice.GetDevice());

	if (mSwapchain != nullptr)
	{
		std::shared_ptr<Swapchain> oldSwapchain = std::move(mSwapchain);

		mSwapchain = std::make_unique<MV::Swapchain>(mDevice, extent, oldSwapchain);

		check(mSwapchain->CompareSwapFormats(*oldSwapchain));
	}
	else
	{
		mSwapchain = std::make_unique<MV::Swapchain>(mDevice, extent);
	}
}