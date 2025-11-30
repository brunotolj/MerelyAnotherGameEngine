#include "Core/Asserts.h"
#include "Rendering/Buffer.h"
#include "Rendering/Descriptor.h"
#include "Rendering/Systems/SpriteRenderSystem.h"
#include "Rendering/Model.h"
#include "Rendering/Pipeline.h"
#include "Rendering/Renderer.h"
#include "Rendering/Texture.h"

SpriteRenderSystem::SpriteRenderSystem(Device& device, Renderer& renderer, const std::vector<std::string>& texturePaths) :
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
			sizeof(SpriteUBO),
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

	CreateVertexBuffer();

	CreatePipelineLayout(mUniformBufferDescriptorSetLayout->GetDescriptorSetLayout(), mTextureDescriptorSetLayout->GetDescriptorSetLayout());
	CreatePipeline(renderer.GetSwapchainRenderPass());
}

SpriteRenderSystem::~SpriteRenderSystem()
{
	vkDestroyPipelineLayout(mDevice.GetDevice(), mPipelineLayout, nullptr);
}

float SpriteRenderSystem::GetAspectRatio() const
{
	return mRenderer.GetAspectRatio();
}

void SpriteRenderSystem::RenderSprites(VkCommandBuffer commandBuffer, const std::vector<SpriteRenderData>& data)
{
	mPipeline->Bind(commandBuffer);
	
	VkExtent2D screenExtent = mRenderer.GetExtent();

	SpriteUBO ubo;
	ubo.ScreenTransform = { -1.0f, -1.0f, 2.0f / 1920.0f, 2.0f / 1080.0f };
	
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

	{
		VkBuffer buffers[] = { mVertexBuffer->GetBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	for (const SpriteRenderData& spriteData : data)
	{
		PushConstantData push;
		
		push.ScreenCoords = {
			spriteData.ScreenCoordsMin.x,
			spriteData.ScreenCoordsMin.y,
			spriteData.ScreenCoordsMax.x - spriteData.ScreenCoordsMin.x,
			spriteData.ScreenCoordsMax.y - spriteData.ScreenCoordsMin.y };

		push.TextureCoords = {
			spriteData.TextureCoordsMin.x,
			spriteData.TextureCoordsMin.y,
			spriteData.TextureCoordsMax.x - spriteData.TextureCoordsMin.x,
			spriteData.TextureCoordsMax.y - spriteData.TextureCoordsMin.y };
	
		mage_check(spriteData.TextureIndex >= 0 && spriteData.TextureIndex < mTextures.size());
	
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mPipelineLayout,
			1,
			1,
			&mDescriptorSets[Swapchain::gMaxFramesInFlight + spriteData.TextureIndex],
			0,
			nullptr);
	
		vkCmdPushConstants(
			commandBuffer,
			mPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData),
			&push);
	
		vkCmdDraw(commandBuffer, 4, 1, 0, 0);
	}
}

void SpriteRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout uniformBufferDescriptorSetLayout, VkDescriptorSetLayout textureDescriptorSetLayout)
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

void SpriteRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	mage_check(mPipelineLayout != nullptr);

	PipelineConfigInfo pipelineConfig{};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.mInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	pipelineConfig.mRenderPass = renderPass;
	pipelineConfig.mPipelineLayout = mPipelineLayout;
	pipelineConfig.mDepthStencilInfo.depthTestEnable = VK_FALSE;
	pipelineConfig.BindingDescriptions = {{ 0, sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX }};
	pipelineConfig.AttributeDescriptions = {{ 0, 0, VK_FORMAT_R32_SFLOAT, 0 }};

	mPipeline = std::make_unique<Pipeline>(
		mDevice,
		"Source/Shaders/SpriteShader.vert",
		"Source/Shaders/SpriteShader.frag",
		pipelineConfig);
}

void SpriteRenderSystem::CreateVertexBuffer()
{
	VkDeviceSize vertexSize = sizeof(float);

	float vertexData[4] = { 0.0f, 1.0f, 2.0f, 3.0f };

	Buffer stagingBuffer(
		mDevice,
		vertexSize,
		4,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	stagingBuffer.Map();
	stagingBuffer.WriteToBuffer((void*)vertexData);

	mVertexBuffer = std::make_unique<Buffer>(
		mDevice,
		vertexSize,
		4,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	mDevice.CopyBuffer(stagingBuffer.GetBuffer(), mVertexBuffer->GetBuffer(), stagingBuffer.GetBufferSize());
}
