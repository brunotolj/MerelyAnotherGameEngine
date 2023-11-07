#include "Rendering/Renderer.h"

Renderer::Renderer(Window& window, Device& device) :
	mWindow(window), mDevice(device)
{
	RecreateSwapchain();
	CreateCommandBuffers();
}

Renderer::~Renderer()
{
	FreeCommandBuffers();
}

VkCommandBuffer Renderer::BeginFrame()
{
	mage_check(!mIsFrameInProgress);

	VkResult result = mSwapchain->AcquireNextImage(&mCurrentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
		return nullptr;
	}

	mage_check(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

	mIsFrameInProgress = true;

	VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	mage_check(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS);

	return commandBuffer;
}

void Renderer::EndFrame()
{
	mage_check(mIsFrameInProgress);

	VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

	mIsFrameInProgress = false;

	mage_check(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS);

	VkResult result = mSwapchain->SubmitCommandBuffers(&commandBuffer, &mCurrentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mWindow.WasWindowResized())
	{
		mWindow.ResetWindowResizedFlag();
		RecreateSwapchain();
	}
	else
	{
		mage_check(result == VK_SUCCESS);
	}

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mCommandBuffers.size();
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	mage_check(mIsFrameInProgress);
	mage_check(commandBuffer == GetCurrentCommandBuffer());

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mSwapchain->GetRenderPass();
	renderPassInfo.framebuffer = mSwapchain->GetFrameBuffer(mCurrentImageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = mSwapchain->GetExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(mSwapchain->GetExtent().width);
	viewport.height = static_cast<float>(mSwapchain->GetExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{ { 0,0 }, mSwapchain->GetExtent() };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	mage_check(mIsFrameInProgress);
	mage_check(commandBuffer == GetCurrentCommandBuffer());

	vkCmdEndRenderPass(commandBuffer);
}

VkCommandBuffer Renderer::GetCurrentCommandBuffer() const
{
	mage_check(mIsFrameInProgress);
	return mCommandBuffers[mCurrentFrameIndex];
}

int32_t Renderer::GetCurrentFrameIndex() const
{
	mage_check(mIsFrameInProgress);
	return mCurrentFrameIndex;
}

VkRenderPass Renderer::GetSwapchainRenderPass() const
{
	return mSwapchain->GetRenderPass();
}

void Renderer::CreateCommandBuffers()
{
	mCommandBuffers.resize(Swapchain::gMaxFramesInFlight);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mDevice.GetCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

	mage_check(vkAllocateCommandBuffers(mDevice.GetDevice(), &allocInfo, mCommandBuffers.data()) == VK_SUCCESS);
}

void Renderer::FreeCommandBuffers()
{
	vkFreeCommandBuffers(
		mDevice.GetDevice(),
		mDevice.GetCommandPool(),
		static_cast<uint32_t>(mCommandBuffers.size()),
		mCommandBuffers.data());
}

void Renderer::RecreateSwapchain()
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

		mSwapchain = std::make_unique<Swapchain>(mDevice, extent, oldSwapchain);

		mage_check(mSwapchain->CompareSwapFormats(*oldSwapchain));
	}
	else
	{
		mSwapchain = std::make_unique<Swapchain>(mDevice, extent);
	}
}
