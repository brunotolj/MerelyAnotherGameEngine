#pragma once

#include "Rendering/Font.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"

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
	u32 FontIndex;
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
	TextRenderSystem(Vulkan::Renderer const& renderer, Vulkan::ShaderCompiler const& inShaderCompiler, const mage::Array<mage::StringView>& fontPaths);

	void RenderText(Vulkan::RenderFrameData const& frameData, mage::Array<TextRenderData> const& data);

private:
	void SetupDynamicState(vk::CommandBuffer inCommandBuffer) const;

	Vulkan::Renderer const& mRenderer;

	Vulkan::Pipeline mPipeline;

	Vulkan::Buffer mVertexBuffer = nullptr;
	mage::Array<Vulkan::Buffer> mGlyphBuffers;

	mage::Array<FontData> mFonts;
	mage::Array<std::unordered_map<u32, u64>> mGlyphBufferOffsets;

	Vulkan::Pipeline CreatePipeline(Vulkan::ShaderCompiler const& inShaderCompiler);

	void CreateGlyphBuffer(mage::StringView fontPath);
	void CreateVertexBuffer();
};
