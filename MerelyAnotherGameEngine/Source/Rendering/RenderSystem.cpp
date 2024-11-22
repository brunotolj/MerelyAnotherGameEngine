#include "Core/Asserts.h"
#include "Rendering/Buffer.h"
#include "Rendering/Descriptor.h"
#include "Rendering/Model.h"
#include "Rendering/Pipeline.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Texture.h"

RenderSystem::RenderSystem(Device& device, Renderer& renderer, const std::vector<std::string>& texturePaths) :
	mDevice(device), mRenderer(renderer)
{
	uint32_t uniformBufferCount = Swapchain::gMaxFramesInFlight;
	uint32_t textureCount = static_cast<uint32_t>(texturePaths.size());
	mage_check(textureCount > 0);

	mDescriptorPool = DescriptorPool::Builder(mDevice)
		.SetMaxSets(uniformBufferCount + textureCount)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBufferCount)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureCount)
		.Build();

	mDescriptorSets.resize(uniformBufferCount + textureCount);

	mUniformBuffers.resize(uniformBufferCount);
	for (size_t i = 0; i < uniformBufferCount; ++i)
	{
		std::unique_ptr<Buffer>& buffer = mUniformBuffers[i];
		buffer = std::make_unique<Buffer>(
			device,
			sizeof(GlobalUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			device.properties.limits.minUniformBufferOffsetAlignment);

		buffer->Map();
	}

	mUniformBufferDescriptorSetLayout = DescriptorSetLayout::Builder(mDevice)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build();

	for (size_t i = 0; i < uniformBufferCount; ++i)
	{
		VkDescriptorBufferInfo bufferInfo = mUniformBuffers[i]->DescriptorInfo();
		DescriptorWriter(*mUniformBufferDescriptorSetLayout, *mDescriptorPool)
			.WriteBuffer(0, &bufferInfo)
			.Build(mDescriptorSets[i]);
	}

	mTextures.resize(textureCount);
	for (size_t i = 0; i < textureCount; ++i)
	{
		std::unique_ptr<Texture>& texture = mTextures[i];
		texture = std::make_unique<Texture>(
			device,
			texturePaths[i]);
	}

	mTextureDescriptorSetLayout = DescriptorSetLayout::Builder(mDevice)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build();

	for (size_t i = 0; i < textureCount; ++i)
	{
		VkDescriptorImageInfo imageInfo = mTextures[i]->DescriptorInfo();
		DescriptorWriter(*mTextureDescriptorSetLayout, *mDescriptorPool)
			.WriteImage(0, &imageInfo)
			.Build(mDescriptorSets[uniformBufferCount + i]);
	}

	CreatePipelineLayout(mUniformBufferDescriptorSetLayout->GetDescriptorSetLayout(), mTextureDescriptorSetLayout->GetDescriptorSetLayout());
	CreatePipeline(renderer.GetSwapchainRenderPass());
}

RenderSystem::~RenderSystem()
{
	vkDestroyPipelineLayout(mDevice.GetDevice(), mPipelineLayout, nullptr);
}

float RenderSystem::GetAspectRatio() const
{
	return mRenderer.GetAspectRatio();
}

void RenderSystem::SetCamera(ICamera const* camera)
{
	mCamera = camera;
}

void RenderSystem::RenderScene(VkCommandBuffer commandBuffer)
{
	mPipeline->Bind(commandBuffer);

	GlobalUBO ubo;
	ubo.CameraTransform = mCamera ? mCamera->GetProjectionTransform() * mCamera->GetViewTransform() : glm::mat4(1.0f);
	ubo.LightDirectionAndAmbient = glm::normalize(glm::vec4(3.0, 2.0, -2.5, 0.0));
	ubo.LightDirectionAndAmbient.w = 0.05f;

	int frameIndex = mRenderer.GetCurrentFrameIndex();
	std::unique_ptr<Buffer>& uniformBuffer = mUniformBuffers[frameIndex];
	uniformBuffer->WriteToBuffer(&ubo);
	uniformBuffer->Flush();

	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		mPipelineLayout,
		0,
		1,
		&mDescriptorSets[frameIndex],
		0,
		nullptr);

	for (IRenderable const* renderable : mRenderables)
	{
		const glm::mat4 transform = renderable->GetTransform().Matrix();
		const glm::vec4 color = glm::vec4(renderable->GetColor(), 1.0f);

		PushConstantData push;
		push.Transform = transform;
		push.Color = color;

		mage_check(renderable->GetTextureIndex() >= 0 && renderable->GetTextureIndex() < mTextures.size());

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mPipelineLayout,
			1,
			1,
			&mDescriptorSets[Swapchain::gMaxFramesInFlight + renderable->GetTextureIndex()],
			0,
			nullptr);

		vkCmdPushConstants(
			commandBuffer,
			mPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData),
			&push);

		renderable->Bind(commandBuffer);
		renderable->Draw(commandBuffer);
	}
}

void RenderSystem::AddRenderable(IRenderable const* renderable)
{
	mRenderables.insert(renderable);
}

void RenderSystem::RemoveRenderable(IRenderable const* renderable)
{
	mRenderables.erase(renderable);
}

void RenderSystem::CreatePipelineLayout(VkDescriptorSetLayout uniformBufferDescriptorSetLayout, VkDescriptorSetLayout textureDescriptorSetLayout)
{
	VkPushConstantRange pushConstantRange;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstantData);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ uniformBufferDescriptorSetLayout, textureDescriptorSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	mage_check(vkCreatePipelineLayout(mDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) == VK_SUCCESS);
}

void RenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	mage_check(mPipelineLayout != nullptr);

	PipelineConfigInfo pipelineConfig{};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.mRenderPass = renderPass;
	pipelineConfig.mPipelineLayout = mPipelineLayout;
	mPipeline = std::make_unique<Pipeline>(
		mDevice,
		"Source/Shaders/SimpleShader.vert",
		"Source/Shaders/SimpleShader.frag",
		pipelineConfig);
}
