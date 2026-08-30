#include "Rendering/Systems/SpriteRenderSystem.h"
#include "Engine/Engine.h"
#include "Framework/GameWorld.h"
#include "Game/SpriteObjectComponent.h"
#include "Vulkan/Renderer.h"

SpriteRenderSystem::SpriteRenderSystem(GameWorld& inWorld) : GameSystemWithPrerequisites(inWorld)
{
	CreatePipeline();

	u32 uniformBufferCount = Vulkan::Renderer::cMaxFramesInFlight;

	Vulkan::Buffer::CreateInfo bufferCreateInfo
	{
		.Size = sizeof(SpriteUBO),
		.UsageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible
	};

	mUniformBuffers.Reserve(uniformBufferCount);
	for (u32 i = 0; i < uniformBufferCount; ++i)
	{
		mUniformBuffers.AddConstruct(bufferCreateInfo);
		mUniformBuffers[i].Map();
	}

	CreateVertexBuffer();
}

void SpriteRenderSystem::Update(f32 inDeltaTime)
{
	Vulkan::RenderFrameData frameData = Get<Vulkan::Renderer>().GetCurrentFrameData();
	mage::Array<SpriteRenderData> spriteData;

	mWorld.ForEachObject([&spriteData](GameObject* object)
	{
		for (SpriteObjectComponent const* spriteComp : object->GetComponentsOfClass<SpriteObjectComponent>())
			spriteData.AddConstruct(
				spriteComp->GetScreenCoordsMin(),
				spriteComp->GetScreenCoordsMax(),
				spriteComp->GetTextureCoordsMin(),
				spriteComp->GetTextureCoordsMax(),
				spriteComp->GetTexture());

		return true;
	});

	RenderSprites(frameData, spriteData);
}

void SpriteRenderSystem::RenderSprites(Vulkan::RenderFrameData const& frameData, mage::Array<SpriteRenderData> const& data) const
{
	SetupDynamicState(frameData.CommandBuffer);
	mPipeline.Bind(frameData.CommandBuffer);

	VkExtent2D screenExtent = frameData.Extent;

	SpriteUBO ubo;
	ubo.ScreenTransform = { -1.0f, -1.0f, 2.0f / 1920.0f, 2.0f / 1080.0f };

	Vulkan::Buffer const& uniformBuffer = mUniformBuffers[frameData.Index];
	uniformBuffer.Write(&ubo, sizeof(ubo));
	uniformBuffer.Flush();

	mVertexBuffer.BindVertexBuffer(frameData.CommandBuffer);

	for (const SpriteRenderData& spriteData : data)
	{
		Texture const* texture = spriteData.Texture.GetAsset();
		mage_check(texture);

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

			push.UniformBuffer = uniformBuffer.GetDeviceAddress();

			vk::PushConstantsInfo pushInfo
			{
				.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
				.offset = 0,
				.size = sizeof(PushConstantData),
				.pValues = &push
			};
	
			mPipeline.PushConstants(frameData.CommandBuffer, pushInfo);
		}

		{
			vk::DescriptorImageInfo imageInfo = texture->GetDescriptorInfo();

			mage::Array<vk::WriteDescriptorSet> descriptorWrites
			{
				{
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eCombinedImageSampler,
					.pImageInfo = &imageInfo
				}
			};

			vk::PushDescriptorSetInfo pushInfo
			{
				.stageFlags = vk::ShaderStageFlagBits::eFragment,
				.set = 0,
				.descriptorWriteCount = descriptorWrites.GetSize(),
				.pDescriptorWrites = descriptorWrites.GetData()
			};

			mPipeline.PushDescriptorSet(frameData.CommandBuffer, pushInfo);
		}

		vkCmdDraw(frameData.CommandBuffer, 4, 1, 0, 0);
	}
}

void SpriteRenderSystem::SetupDynamicState(vk::CommandBuffer inCommandBuffer) const
{
	inCommandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleStrip);
	inCommandBuffer.setDepthWriteEnable(vk::False);
	inCommandBuffer.setConservativeRasterizationModeEXT(vk::ConservativeRasterizationModeEXT::eDisabled);
}

void SpriteRenderSystem::CreatePipeline()
{
	Vulkan::Pipeline::CreateInfo pipelineCreateInfo
	{
		.ShaderCode = gEngine->mShaderCompiler.CompileFromFile("Source/Shaders/SpriteShader.slang"),
		.ShaderStages
		{
			{ vk::ShaderStageFlagBits::eVertex, "vertMain" },
			{ vk::ShaderStageFlagBits::eFragment, "fragMain" }
		},
		.InputBindingDescriptions = {{ 0, sizeof(f32), vk::VertexInputRate::eVertex }},
		.InputAttributeDescriptions = {{ 0, 0, vk::Format::eR32Sfloat, 0 }},
		.DescriptorSetBindings
		{
			{
				.binding = 0,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eFragment
			}
		},
		.PushConstantRanges
		{{
			.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			.offset = 0,
			.size = sizeof(PushConstantData)
		}}
	};

	mPipeline.Create(pipelineCreateInfo);
}

void SpriteRenderSystem::CreateVertexBuffer()
{
	vk::DeviceSize dataSize = 4 * sizeof(f32);

	f32 vertexData[4] = { 0.0f, 1.0f, 2.0f, 3.0f };

	Vulkan::Buffer::CreateInfo stagingBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	Vulkan::Buffer stagingBuffer(stagingBufferCreateInfo);

	stagingBuffer.Map();
	stagingBuffer.Write((void*)vertexData, dataSize);

	Vulkan::Buffer::CreateInfo vertexBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	mVertexBuffer.Create(vertexBufferCreateInfo);

	gEngine->mVulkanDevice.SubmitSingleTimeCommands([this, &stagingBuffer](vk::CommandBuffer inCommandBuffer)
		{
			mVertexBuffer.CopyFromBuffer(inCommandBuffer, stagingBuffer);
		});
}
