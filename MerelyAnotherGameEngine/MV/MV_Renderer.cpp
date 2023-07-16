#include "MV/MV_Renderer.h"

MV::Renderer::Renderer(Window& window, Device& device) :
	privWindow(window), privDevice(device)
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
	check(!privIsFrameInProgress);

	VkResult result = privSwapchain->AcquireNextImage(&privCurrentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
		return nullptr;
	}

	check(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

	privIsFrameInProgress = true;

	VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	check(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS);

	return commandBuffer;
}

void MV::Renderer::EndFrame()
{
	check(privIsFrameInProgress);

	VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

	privIsFrameInProgress = false;

	check(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS);

	VkResult result = privSwapchain->SubmitCommandBuffers(&commandBuffer, &privCurrentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || privWindow.WasWindowResized())
	{
		privWindow.ResetWindowResizedFlag();
		RecreateSwapchain();
	}
	else
	{
		check(result == VK_SUCCESS);
	}

	privCurrentFrameIndex = (privCurrentFrameIndex + 1) % privCommandBuffers.size();
}

void MV::Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	check(privIsFrameInProgress);
	check(commandBuffer == GetCurrentCommandBuffer());

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = privSwapchain->GetRenderPass();
	renderPassInfo.framebuffer = privSwapchain->GetFrameBuffer(privCurrentImageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = privSwapchain->GetExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(privSwapchain->GetExtent().width);
	viewport.height = static_cast<float>(privSwapchain->GetExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{ { 0,0 }, privSwapchain->GetExtent() };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void MV::Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	check(privIsFrameInProgress);
	check(commandBuffer == GetCurrentCommandBuffer());

	vkCmdEndRenderPass(commandBuffer);
}

VkCommandBuffer MV::Renderer::GetCurrentCommandBuffer() const
{
	check(privIsFrameInProgress);
	return privCommandBuffers[privCurrentFrameIndex];
}

int32_t MV::Renderer::GetCurrentFrameIndex() const
{
	check(privIsFrameInProgress);
	return privCurrentFrameIndex;
}

VkRenderPass MV::Renderer::GetSwapchainRenderPass() const
{
	return privSwapchain->GetRenderPass();
}

void MV::Renderer::CreateCommandBuffers()
{
	privCommandBuffers.resize(Swapchain::gMaxFramesInFlight);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = privDevice.GetCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(privCommandBuffers.size());

	check(vkAllocateCommandBuffers(privDevice.GetDevice(), &allocInfo, privCommandBuffers.data()) == VK_SUCCESS);
}

void MV::Renderer::FreeCommandBuffers()
{
	vkFreeCommandBuffers(
		privDevice.GetDevice(),
		privDevice.GetCommandPool(),
		static_cast<uint32_t>(privCommandBuffers.size()),
		privCommandBuffers.data());
}

void MV::Renderer::RecreateSwapchain()
{
	VkExtent2D extent = privWindow.GetExtent();

	while (extent.width == 0 || extent.height == 0)
	{
		extent = privWindow.GetExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(privDevice.GetDevice());

	if (privSwapchain != nullptr)
	{
		std::shared_ptr<Swapchain> oldSwapchain = std::move(privSwapchain);

		privSwapchain = std::make_unique<MV::Swapchain>(privDevice, extent, oldSwapchain);

		check(privSwapchain->CompareSwapFormats(*oldSwapchain));
	}
	else
	{
		privSwapchain = std::make_unique<MV::Swapchain>(privDevice, extent);
	}
}