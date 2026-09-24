#include "Framework/Systems/MeshRenderSystem.h"
#include "Engine/Engine.h"
#include "Vulkan/Renderer.h"

WorldComponentFactoryFunction MeshRenderSystemFactoryFunction("MeshRenderSystem", [](GameWorld& inWorld, PropertyContainer const& inProperties)
{
	inWorld.CreateComponent<MeshRenderSystem>();
});

MeshRenderSystem::MeshRenderSystem(GameWorld& inWorld) : GameSystemWithPrerequisites(inWorld)
{
	CreatePipeline();

	u32 uniformBufferCount = Vulkan::Renderer::cMaxFramesInFlight;

	Vulkan::Buffer::CreateInfo bufferCreateInfo
	{
		.Size = sizeof(MeshUBO),
		.UsageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible
	};

	mUniformBuffers.Reserve(uniformBufferCount);
	for (u32 i = 0; i < uniformBufferCount; ++i)
	{
		mUniformBuffers.AddConstruct(bufferCreateInfo);
		mUniformBuffers[i].Map();
	}
}

glm::mat4 CalcProjectionTransform(f32 nearPlane, f32 farPlane, f32 horizontalFOV, f32 aspectRatio)
{
	mage_check(nearPlane >= 0.0f && farPlane > nearPlane);
	mage_check(horizontalFOV > 0.0f && glm::degrees(horizontalFOV) < 180.0f);

	f32 fovFactor = 1.0f / glm::tan(horizontalFOV / 2.0f);
	f32 planeDelta = farPlane - nearPlane;

	return
	{
		{ fovFactor, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, farPlane / planeDelta, 1.0f },
		{ 0.0f, -fovFactor * aspectRatio, 0.0f, 0.0f },
		{ 0.0f, 0.0f, -farPlane * nearPlane / planeDelta, 0.0f }
	};
}

void MeshRenderSystem::Update(f32 inDeltaTime)
{
	Vulkan::RenderFrameData frameData = Get<Vulkan::Renderer>().GetCurrentFrameData();
	SceneRenderData sceneData;

	sceneData.LightDirection = glm::vec3(-3.0f, 2.0f, -2.5f);
	sceneData.AmbientLightIntensity = 0.05f;

	f32 aspectRatio = f32(frameData.Extent.width) / f32(frameData.Extent.height);
	sceneData.ProjectionTransform = CalcProjectionTransform(0.1f, 1000.0f, glm::radians(90.0f), aspectRatio);

	TransformTreeEntryId cameraTransformId = mage::InvalidIndex;
	for (CameraEntity* cameraEntity : mWorld.GetEntities<CameraEntity>())
	{
		cameraTransformId = cameraEntity->GetParentEntity().mTransformId;
		break;
	}

	if (cameraTransformId != mage::InvalidIndex)
		sceneData.ViewTransform = Get<TransformTree>().GetGlobalTransform(cameraTransformId).Inverse().Matrix();
	else
		sceneData.ViewTransform = mage::Transform().Matrix();

	for (StaticMeshEntity* staticMeshEntity : mWorld.GetEntities<StaticMeshEntity>())
	{
		sceneData.Meshes.AddConstruct(
			Get<TransformTree>().GetGlobalTransform(staticMeshEntity->GetParentEntity().mTransformId).Matrix(),
			staticMeshEntity->mMesh,
			staticMeshEntity->mTexture);
	}

	RenderMeshes(frameData, sceneData);
}

void MeshRenderSystem::RenderMeshes(Vulkan::RenderFrameData const& inFrameData, SceneRenderData const& inData) const
{
	SetupDynamicState(inFrameData.CommandBuffer);
	mPipeline.Bind(inFrameData.CommandBuffer);

	MeshUBO ubo;
	ubo.CameraTransform = inData.ProjectionTransform * inData.ViewTransform;
	ubo.LightDirectionAndAmbient = glm::normalize(glm::vec4(inData.LightDirection, 0.0f));
	ubo.LightDirectionAndAmbient.w = inData.AmbientLightIntensity;

	Vulkan::Buffer const& uniformBuffer = mUniformBuffers[inFrameData.Index];
	uniformBuffer.Write(&ubo, sizeof(ubo));
	uniformBuffer.Flush();

	for (MeshRenderData const& meshData : inData.Meshes)
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

			mPipeline.PushConstants(inFrameData.CommandBuffer, pushInfo);
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

			mPipeline.PushDescriptorSet(inFrameData.CommandBuffer, pushInfo);
		}

		mesh->Bind(inFrameData.CommandBuffer);
		mesh->Draw(inFrameData.CommandBuffer);
	}
}

void MeshRenderSystem::SetupDynamicState(vk::CommandBuffer inCommandBuffer) const
{
	inCommandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
	inCommandBuffer.setDepthWriteEnable(vk::True);
	inCommandBuffer.setConservativeRasterizationModeEXT(vk::ConservativeRasterizationModeEXT::eDisabled);
}

void MeshRenderSystem::CreatePipeline()
{
	Vulkan::Pipeline::CreateInfo pipelineCreateInfo
	{
		.ShaderCode = gEngine->mShaderCompiler.CompileFromFile("Source/Shaders/MeshShader.slang"),
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

	mPipeline.Create(pipelineCreateInfo);
}
