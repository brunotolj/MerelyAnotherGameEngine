#include "Core/Asserts.h"
#include "Rendering/Buffer.h"
#include "Rendering/Descriptor.h"
#include "Rendering/Systems/MeshRenderSystem.h"
#include "Rendering/Model.h"
#include "Rendering/Pipeline.h"
#include "Rendering/Renderer.h"
#include "Rendering/Texture.h"

MeshRenderSystem::MeshRenderSystem(Device& device, Renderer& renderer, const std::vector<std::string>& texturePaths) :
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
			sizeof(MeshUBO),
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

MeshRenderSystem::~MeshRenderSystem()
{
	vkDestroyPipelineLayout(mDevice.GetDevice(), mPipelineLayout, nullptr);
}

float MeshRenderSystem::GetAspectRatio() const
{
	return mRenderer.GetAspectRatio();
}

void MeshRenderSystem::RenderMeshes(VkCommandBuffer commandBuffer, const SceneRenderData& data)
{
	mPipeline->Bind(commandBuffer);

	MeshUBO ubo;
	ubo.CameraTransform = data.ProjectionTransform * data.ViewTransform;
	ubo.LightDirectionAndAmbient = glm::normalize(glm::vec4(data.LightDirection, 0.0f));
	ubo.LightDirectionAndAmbient.w = data.AmbientLightIntensity;

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

	for (const MeshRenderData& meshData : data.Meshes)
	{
		PushConstantData push;
		push.Transform = meshData.Transform;
		push.Color = glm::vec4(meshData.Color, 1.0f);

		mage_check(meshData.TextureIndex >= 0 && meshData.TextureIndex < mTextures.size());

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mPipelineLayout,
			1,
			1,
			&mDescriptorSets[Swapchain::gMaxFramesInFlight + meshData.TextureIndex],
			0,
			nullptr);

		vkCmdPushConstants(
			commandBuffer,
			mPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData),
			&push);

		meshData.Mesh->Bind(commandBuffer);
		meshData.Mesh->Draw(commandBuffer);
	}
}

void MeshRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout uniformBufferDescriptorSetLayout, VkDescriptorSetLayout textureDescriptorSetLayout)
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

void MeshRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	mage_check(mPipelineLayout != nullptr);

	PipelineConfigInfo pipelineConfig{};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.mRenderPass = renderPass;
	pipelineConfig.mPipelineLayout = mPipelineLayout;
	pipelineConfig.BindingDescriptions = Model::Vertex::GetBindingDescriptions();
	pipelineConfig.AttributeDescriptions = Model::Vertex::GetAttributeDescriptions();

	mPipeline = std::make_unique<Pipeline>(
		mDevice,
		"Source/Shaders/MeshShader.vert",
		"Source/Shaders/MeshShader.frag",
		pipelineConfig);
}
