#pragma once

#include "Assets/Texture.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"

class AssetManager;

namespace Vulkan
{
	class Renderer;
	struct RenderFrameData;
}

struct SpriteUBO
{
	alignas(16) glm::vec4 ScreenTransform;
};

struct SpriteRenderData
{
	glm::vec2 ScreenCoordsMin;
	glm::vec2 ScreenCoordsMax;
	glm::vec2 TextureCoordsMin;
	glm::vec2 TextureCoordsMax;
	AssetHandle<Texture> Texture;
};

class SpriteRenderSystem : public NonCopyableClass
{
	struct PushConstantData
	{
		alignas(16) glm::vec4 ScreenCoords{ 0.0f, 0.0f, 1.0f, 1.0f };
		alignas(16) glm::vec4 TextureCoords{ 0.0f, 0.0f, 1.0f, 1.0f };
		alignas(8) vk::DeviceAddress UniformBuffer;
	};

public:
	SpriteRenderSystem(Vulkan::Renderer const& renderer, Vulkan::ShaderCompiler const& inShaderCompiler, AssetManager const& inAssetManager);

	void RenderSprites(Vulkan::RenderFrameData const& frameData, mage::Array<SpriteRenderData> const& data);

private:
	void SetupDynamicState(vk::CommandBuffer inCommandBuffer) const;

	Vulkan::Renderer const& mRenderer;

	AssetManager const& mAssetManager;

	Vulkan::Pipeline mPipeline;

	mage::Array<Vulkan::Buffer> mUniformBuffers;

	Vulkan::Buffer mVertexBuffer = nullptr;

	Vulkan::Pipeline CreatePipeline(Vulkan::ShaderCompiler const& inShaderCompiler);

	void CreateVertexBuffer();
};
