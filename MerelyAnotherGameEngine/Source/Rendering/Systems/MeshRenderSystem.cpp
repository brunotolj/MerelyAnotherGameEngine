#include "Rendering/Systems/MeshRenderSystem.h"
#include "Vulkan/Model.h"
#include "Vulkan/Renderer.h"

MeshRenderSystem::MeshRenderSystem(Vulkan::Renderer const& renderer, Vulkan::ShaderCompiler const& inShaderCompiler, const mage::Array<mage::StringView>& texturePaths) :
	mRenderer(renderer), mPipeline(CreatePipeline(inShaderCompiler, texturePaths.GetSize()))
{
	u32 uniformBufferCount = mRenderer.cMaxFramesInFlight;
	u32 textureCount = static_cast<u32>(texturePaths.GetSize());
	mage_check(textureCount > 0);

	Vulkan::BufferCreateInfo bufferCreateInfo
	{
		.Size = sizeof(MeshUBO),
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
}

MeshRenderSystem::~MeshRenderSystem()
{
}

void MeshRenderSystem::RenderMeshes(Vulkan::RenderFrameData const& frameData, const SceneRenderData& data)
{
	mPipeline.Bind(frameData.CommandBuffer);

	MeshUBO ubo;
	ubo.CameraTransform = data.ProjectionTransform * data.ViewTransform;
	ubo.LightDirectionAndAmbient = glm::normalize(glm::vec4(data.LightDirection, 0.0f));
	ubo.LightDirectionAndAmbient.w = data.AmbientLightIntensity;

	Vulkan::Buffer& uniformBuffer = mUniformBuffers[frameData.Index];
	uniformBuffer.Write(&ubo, sizeof(ubo));
	uniformBuffer.Flush();

	vk::BindDescriptorSetsInfo bindInfo
	{
		.stageFlags = vk::ShaderStageFlagBits::eVertex,
		.firstSet = 0
	};

	mPipeline.BindDescriptorSet(frameData.CommandBuffer, bindInfo, frameData.Index);

	for (const MeshRenderData& meshData : data.Meshes)
	{
		PushConstantData push;
		push.Transform = meshData.Transform;
		push.Color = glm::vec4(meshData.Color, 1.0f);

		mage_check(meshData.TextureIndex >= 0 && meshData.TextureIndex < mTextures.GetSize());

		vk::BindDescriptorSetsInfo bindInfo
		{
			.stageFlags = vk::ShaderStageFlagBits::eFragment,
			.firstSet = 1
		};

		mPipeline.BindDescriptorSet(frameData.CommandBuffer, bindInfo, mRenderer.cMaxFramesInFlight + meshData.TextureIndex);

		vk::PushConstantsInfo pushInfo
		{
			.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			.offset = 0,
			.size = sizeof(PushConstantData),
			.pValues = &push
		};

		mPipeline.PushConstants(frameData.CommandBuffer, pushInfo);

		meshData.Mesh->Bind(frameData.CommandBuffer);
		meshData.Mesh->Draw(frameData.CommandBuffer);
	}
}

Vulkan::Pipeline MeshRenderSystem::CreatePipeline(Vulkan::ShaderCompiler const& inShaderCompiler, u32 inTextureCount)
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
		.ShaderCode = inShaderCompiler.CompileFromFile("Source/Shaders/MeshShader.slang"),
		.BindingDescriptions = Vulkan::Vertex::GetBindingDescriptions(),
		.AttributeDescriptions = Vulkan::Vertex::GetAttributeDescriptions(),
		.InputAssemblyInfo {.topology = vk::PrimitiveTopology::eTriangleList },
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

	return std::move(mRenderer.CreatePipeline(pipelineCreateInfo));
}
