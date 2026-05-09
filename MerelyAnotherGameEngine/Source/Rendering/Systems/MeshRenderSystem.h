#pragma once

#include "Vulkan/Pipeline.h"
#include "Vulkan/Texture.h"

namespace Vulkan
{
	class Model;
	class Renderer;
	struct RenderFrameData;
}

struct MeshUBO
{
	alignas(16) glm::mat4 CameraTransform;
	alignas(16) glm::vec4 LightDirectionAndAmbient;
};

struct MeshRenderData
{
	const Vulkan::Model* Mesh;
	glm::mat4 Transform;
	u32 TextureIndex;
};

struct SceneRenderData
{
	glm::mat4 ViewTransform;
	glm::mat4 ProjectionTransform;

	glm::vec3 LightDirection;
	f32 AmbientLightIntensity;

	std::vector<MeshRenderData> Meshes;
};

class MeshRenderSystem : public NonCopyableClass
{
	struct PushConstantData
	{
		alignas(16) glm::mat4 Transform{ 1.0f };
		alignas(8) vk::DeviceAddress UniformBuffer;
	};

public:
	MeshRenderSystem(Vulkan::Renderer const& renderer, Vulkan::ShaderCompiler const& inShaderCompiler, const mage::Array<mage::StringView>& texturePaths);

	void RenderMeshes(Vulkan::RenderFrameData const& frameData, const SceneRenderData& data);

private:
	void SetupDynamicState(vk::CommandBuffer inCommandBuffer) const;

	Vulkan::Renderer const& mRenderer;

	Vulkan::Pipeline mPipeline;

	mage::Array<Vulkan::Buffer> mUniformBuffers;

	mage::Array<Vulkan::Texture> mTextures;

	Vulkan::Pipeline CreatePipeline(Vulkan::ShaderCompiler const& inShaderCompiler);
};
