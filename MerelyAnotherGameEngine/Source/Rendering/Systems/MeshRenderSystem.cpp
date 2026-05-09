#include "Rendering/Systems/MeshRenderSystem.h"
#include "Vulkan/Model.h"
#include "Vulkan/Renderer.h"

MeshRenderSystem::MeshRenderSystem(Vulkan::Renderer const& renderer, Vulkan::ShaderCompiler const& inShaderCompiler, const mage::Array<mage::StringView>& texturePaths) :
	mRenderer(renderer), mPipeline(CreatePipeline(inShaderCompiler))
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
}

void MeshRenderSystem::RenderMeshes(Vulkan::RenderFrameData const& frameData, const SceneRenderData& data)
{
	SetupDynamicState(frameData.CommandBuffer);
	mPipeline.Bind(frameData.CommandBuffer);

	MeshUBO ubo;
	ubo.CameraTransform = data.ProjectionTransform * data.ViewTransform;
	ubo.LightDirectionAndAmbient = glm::normalize(glm::vec4(data.LightDirection, 0.0f));
	ubo.LightDirectionAndAmbient.w = data.AmbientLightIntensity;

	Vulkan::Buffer& uniformBuffer = mUniformBuffers[frameData.Index];
	uniformBuffer.Write(&ubo, sizeof(ubo));
	uniformBuffer.Flush();

	for (const MeshRenderData& meshData : data.Meshes)
	{
		{
			PushConstantData push;
			push.Transform = meshData.Transform;
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
			mage_check(meshData.TextureIndex >= 0 && meshData.TextureIndex < mTextures.GetSize());

			vk::DescriptorImageInfo imageInfo = mTextures[meshData.TextureIndex].GetDescriptorInfo();

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

		meshData.Mesh->Bind(frameData.CommandBuffer);
		meshData.Mesh->Draw(frameData.CommandBuffer);
	}
}

void MeshRenderSystem::SetupDynamicState(vk::CommandBuffer inCommandBuffer) const
{
	inCommandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
}

Vulkan::Pipeline MeshRenderSystem::CreatePipeline(Vulkan::ShaderCompiler const& inShaderCompiler)
{
	Vulkan::PipelineCreateInfo pipelineCreateInfo
	{
		.ShaderCode = inShaderCompiler.CompileFromFile("Source/Shaders/MeshShader.slang"),
		.ShaderStages
		{
			{ vk::ShaderStageFlagBits::eVertex, "vertMain" },
			{ vk::ShaderStageFlagBits::eFragment, "fragMain" }
		},
		.InputBindingDescriptions = Vulkan::Model::Vertex::GetBindingDescriptions(),
		.InputAttributeDescriptions = Vulkan::Model::Vertex::GetAttributeDescriptions(),
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

	return mRenderer.CreatePipeline(pipelineCreateInfo);
}
