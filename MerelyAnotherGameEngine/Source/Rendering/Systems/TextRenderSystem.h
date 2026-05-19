#pragma once

#include "Assets/Font.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"

class AssetManager;

namespace Vulkan
{
	class Renderer;
	struct RenderFrameData;
}

struct TextRenderData
{
	mage::StringView Text;
	glm::vec4 Color;
	glm::vec2 ScreenPosition;
	f32 Scale;
	AssetHandle<Font> Font;
};

struct BezierCurve
{
	glm::vec2 p0;
	glm::vec2 p1;
	glm::vec2 p2;
};

class TextRenderSystem : public NonCopyableClass
{
	struct PushConstantData
	{
		alignas(16) glm::vec4 Color;
		alignas(8) vk::DeviceAddress Curves;
		alignas(4) i32 CurveCount;
		alignas(4) glm::vec2 ScreenPos;
		alignas(4) glm::vec2 ScreenSize;
		alignas(4) glm::vec2 GlyphBoundsMin;
		alignas(4) glm::vec2 GlyphBoundsMax;
	};

public:
	TextRenderSystem(Vulkan::Renderer const& renderer);

	void RenderText(Vulkan::RenderFrameData const& frameData, mage::Array<TextRenderData> const& data);

private:
	void SetupDynamicState(vk::CommandBuffer inCommandBuffer) const;

	Vulkan::Renderer const& mRenderer;

	Vulkan::Pipeline mPipeline;

	Vulkan::Buffer mVertexBuffer = nullptr;

	Vulkan::Pipeline CreatePipeline();

	void CreateVertexBuffer();
};
