#pragma once

#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Texture.h"

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
	u32 TextureIndex;
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
	SpriteRenderSystem(Vulkan::Renderer const& renderer, Vulkan::ShaderCompiler const& inShaderCompiler, const mage::Array<mage::StringView>& texturePaths);

	void RenderSprites(Vulkan::RenderFrameData const& frameData, const std::vector<SpriteRenderData>& data);

private:
	void SetupDynamicState(vk::CommandBuffer inCommandBuffer) const;

	Vulkan::Renderer const& mRenderer;

	Vulkan::Pipeline mPipeline;

	mage::Array<Vulkan::Buffer> mUniformBuffers;

	mage::Array<Vulkan::Texture> mTextures;

	Vulkan::Buffer mVertexBuffer = nullptr;

	Vulkan::Pipeline CreatePipeline(Vulkan::ShaderCompiler const& inShaderCompiler);

	void CreateVertexBuffer();
};
