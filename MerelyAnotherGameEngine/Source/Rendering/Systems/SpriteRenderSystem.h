#pragma once

#include "Assets/Texture.h"
#include "Framework/GameWorld.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Renderer.h"

class AssetManager;

namespace Vulkan
{
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

class SpriteRenderSystem : public GameSystemWithPrerequisites<Vulkan::Renderer>
{
	struct PushConstantData
	{
		alignas(16) glm::vec4 ScreenCoords{ 0.0f, 0.0f, 1.0f, 1.0f };
		alignas(16) glm::vec4 TextureCoords{ 0.0f, 0.0f, 1.0f, 1.0f };
		alignas(8) vk::DeviceAddress UniformBuffer;
	};

public:
	SpriteRenderSystem(GameWorld& inWorld);

	virtual void Update(f32 inDeltaTime) override;
	void RenderSprites(Vulkan::RenderFrameData const& frameData, mage::Array<SpriteRenderData> const& data) const;

private:
	void SetupDynamicState(vk::CommandBuffer inCommandBuffer) const;

	void CreatePipeline();
	void CreateVertexBuffer();

	Vulkan::Pipeline mPipeline = nullptr;

	mage::Array<Vulkan::Buffer> mUniformBuffers;

	Vulkan::Buffer mVertexBuffer = nullptr;
};
