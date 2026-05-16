#include "Rendering/Systems/MeshRenderSystem.h"
#include "Assets/AssetManager.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Vulkan/Renderer.h"

MeshRenderSystem::MeshRenderSystem(Vulkan::Renderer const& renderer, Vulkan::ShaderCompiler const& inShaderCompiler, AssetManager const& inAssetManager) :
	mRenderer(renderer), mAssetManager(inAssetManager), mPipeline(CreatePipeline(inShaderCompiler))
{
	u32 uniformBufferCount = mRenderer.cMaxFramesInFlight;

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
}

void MeshRenderSystem::RenderMeshes(Vulkan::RenderFrameData const& frameData, SceneRenderData const& data)
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
		StaticMesh const* mesh = meshData.Mesh.GetAsset();
		mage_check(mesh);

		Texture const* texture = meshData.Texture.GetAsset();
		mage_check(texture);

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

		mesh->Bind(frameData.CommandBuffer);
		mesh->Draw(frameData.CommandBuffer);
	}
}

void MeshRenderSystem::SetupDynamicState(vk::CommandBuffer inCommandBuffer) const
{
	inCommandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
	inCommandBuffer.setDepthWriteEnable(vk::True);
	inCommandBuffer.setConservativeRasterizationModeEXT(vk::ConservativeRasterizationModeEXT::eDisabled);
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
		.InputBindingDescriptions = StaticMesh::Vertex::GetBindingDescriptions(),
		.InputAttributeDescriptions = StaticMesh::Vertex::GetAttributeDescriptions(),
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
