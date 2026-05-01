#include "Rendering/Systems/SpriteRenderSystem.h"
#include "Vulkan/Renderer.h"

SpriteRenderSystem::SpriteRenderSystem(Vulkan::Renderer const& renderer, Vulkan::ShaderCompiler const& inShaderCompiler, const mage::Array<mage::StringView>& texturePaths) :
	mRenderer(renderer), mPipeline(CreatePipeline(inShaderCompiler, texturePaths.GetSize()))
{
	u32 uniformBufferCount = Vulkan::Renderer::cMaxFramesInFlight;
	u32 textureCount = static_cast<u32>(texturePaths.GetSize());
	mage_check(textureCount > 0);

	Vulkan::BufferCreateInfo bufferCreateInfo
	{
		.Size = sizeof(SpriteUBO),
		.UsageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible
	};

	mUniformBuffers.Reserve(uniformBufferCount);
	for (u32 i = 0; i < uniformBufferCount; ++i)
	{
		mUniformBuffers.Add(mRenderer.CreateBuffer(bufferCreateInfo));
		mUniformBuffers[i].Map();
	}

	mTextures.Reserve(textureCount);
	for (u32 i = 0; i < textureCount; ++i)
		mTextures.AddConstruct(mRenderer, Vulkan::Texture::LoadFromFile(texturePaths[i]));

	for (u32 i = 0; i < uniformBufferCount; ++i)
	{
		vk::DescriptorBufferInfo bufferInfo = mUniformBuffers[i].GetDescriptorInfo();

		vk::WriteDescriptorSet descriptorWrite
		{
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &bufferInfo
		};

		mPipeline.UpdateDescriptorSet(descriptorWrite, i);
	}

	for (u32 i = 0; i < textureCount; ++i)
	{
		vk::DescriptorImageInfo imageInfo = mTextures[i].GetDescriptorInfo();

		vk::WriteDescriptorSet descriptorWrite
		{
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.pImageInfo = &imageInfo
		};

		mPipeline.UpdateDescriptorSet(descriptorWrite, uniformBufferCount + i);
	}

	CreateVertexBuffer();
}

SpriteRenderSystem::~SpriteRenderSystem()
{
}

void SpriteRenderSystem::RenderSprites(Vulkan::RenderFrameData const& frameData, const std::vector<SpriteRenderData>& data)
{
	mPipeline.Bind(frameData.CommandBuffer);
	
	VkExtent2D screenExtent = frameData.Extent;

	SpriteUBO ubo;
	ubo.ScreenTransform = { -1.0f, -1.0f, 2.0f / 1920.0f, 2.0f / 1080.0f };

	Vulkan::Buffer& uniformBuffer = mUniformBuffers[frameData.Index];
	uniformBuffer.Write(&ubo, sizeof(ubo));
	uniformBuffer.Flush();

	vk::BindDescriptorSetsInfo bindInfo
	{
		.stageFlags = vk::ShaderStageFlagBits::eVertex,
		.firstSet = 0
	};

	mPipeline.BindDescriptorSet(frameData.CommandBuffer, bindInfo, frameData.Index);

	mVertexBuffer.BindVertexBuffer(frameData.CommandBuffer);

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
	
		mage_check(spriteData.TextureIndex >= 0 && spriteData.TextureIndex < mTextures.GetSize());

		vk::BindDescriptorSetsInfo bindInfo
		{
			.stageFlags = vk::ShaderStageFlagBits::eFragment,
			.firstSet = 1
		};

		mPipeline.BindDescriptorSet(frameData.CommandBuffer, bindInfo, mRenderer.cMaxFramesInFlight + spriteData.TextureIndex);

		vk::PushConstantsInfo pushInfo
		{
			.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			.offset = 0,
			.size = sizeof(PushConstantData),
			.pValues = &push
		};

		mPipeline.PushConstants(frameData.CommandBuffer, pushInfo);

		vkCmdDraw(frameData.CommandBuffer, 4, 1, 0, 0);
	}
}

Vulkan::Pipeline SpriteRenderSystem::CreatePipeline(Vulkan::ShaderCompiler const& inShaderCompiler, u32 inTextureCount)
{
	Vulkan::DescriptorSetLayoutInfo layoutInfo0
	{
		.Bindings
		{{
				.binding = 0,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eVertex
		}},
		.Count = mRenderer.cMaxFramesInFlight
	};

	Vulkan::DescriptorSetLayoutInfo layoutInfo1
	{
		.Bindings
		{{
				.binding = 0,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eFragment
		}},
		.Count = inTextureCount
	};

	Vulkan::PipelineCreateInfo pipelineCreateInfo
	{
		.ShaderCode = inShaderCompiler.CompileFromFile("Source/Shaders/SpriteShader.slang"),
		.BindingDescriptions = {{ 0, sizeof(f32), vk::VertexInputRate::eVertex }},
		.AttributeDescriptions = {{ 0, 0, vk::Format::eR32Sfloat, 0 }},
		.InputAssemblyInfo {.topology = vk::PrimitiveTopology::eTriangleStrip },
		.DescriptorSetLayouts { layoutInfo0, layoutInfo1 },
		.DescriptorPoolSizes
		{
			{ .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = mRenderer.cMaxFramesInFlight },
			{ .type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = inTextureCount }
		},
		.PushConstantRanges
		{{
			.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			.offset = 0,
			.size = sizeof(PushConstantData)
		}}
	};

	return mRenderer.CreatePipeline(pipelineCreateInfo);
}

void SpriteRenderSystem::CreateVertexBuffer()
{
	vk::DeviceSize dataSize = 4 * sizeof(f32);

	f32 vertexData[4] = { 0.0f, 1.0f, 2.0f, 3.0f };

	Vulkan::BufferCreateInfo stagingBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	Vulkan::Buffer stagingBuffer = mRenderer.CreateBuffer(stagingBufferCreateInfo);

	stagingBuffer.Map();
	stagingBuffer.Write((void*)vertexData, dataSize);

	Vulkan::BufferCreateInfo vertexBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	mVertexBuffer = mRenderer.CreateBuffer(vertexBufferCreateInfo);

	mRenderer.SubmitSingleTimeCommands([this, &stagingBuffer](vk::CommandBuffer inCommandBuffer)
		{
			mVertexBuffer.CopyFromBuffer(inCommandBuffer, stagingBuffer);
		});
}
