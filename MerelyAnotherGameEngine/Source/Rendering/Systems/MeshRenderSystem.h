#pragma once

#include "Assets/StaticMesh.h"
#include "Assets/Texture.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"

class AssetManager;

namespace Vulkan
{
	struct RenderFrameData;
}

struct MeshUBO
{
	alignas(16) glm::mat4 CameraTransform;
	alignas(16) glm::vec4 LightDirectionAndAmbient;
};

struct MeshRenderData
{
	glm::mat4 Transform;
	AssetHandle<StaticMesh> Mesh;
	AssetHandle<Texture> Texture;
};

struct SceneRenderData
{
	glm::mat4 ViewTransform;
	glm::mat4 ProjectionTransform;

	glm::vec3 LightDirection;
	f32 AmbientLightIntensity;

	mage::Array<MeshRenderData> Meshes;
};

class MeshRenderSystem : public NonCopyableClass
{
	struct PushConstantData
	{
		alignas(16) glm::mat4 Transform{ 1.0f };
		alignas(8) vk::DeviceAddress UniformBuffer;
	};

public:
	MeshRenderSystem();

	void RenderMeshes(Vulkan::RenderFrameData const& frameData, SceneRenderData const& data);

private:
	void SetupDynamicState(vk::CommandBuffer inCommandBuffer) const;
	void CreatePipeline();

	Vulkan::Pipeline mPipeline = nullptr;

	mage::Array<Vulkan::Buffer> mUniformBuffers;
};
