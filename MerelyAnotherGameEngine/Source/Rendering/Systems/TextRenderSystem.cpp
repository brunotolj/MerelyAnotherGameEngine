#include "Rendering/Systems/TextRenderSystem.h"
#include "Vulkan/Renderer.h"

TextRenderSystem::TextRenderSystem(Vulkan::Renderer const& renderer, Vulkan::ShaderCompiler const& inShaderCompiler, const mage::Array<mage::StringView>& fontPaths)
	: mRenderer(renderer), mPipeline(CreatePipeline(inShaderCompiler))
{
	CreateGlyphBuffer(fontPaths[0]);
	CreateGlyphBuffer(fontPaths[1]);
	CreateVertexBuffer();
}

void TextRenderSystem::RenderText(Vulkan::RenderFrameData const& frameData, mage::Array<TextRenderData> const& data)
{
	SetupDynamicState(frameData.CommandBuffer);
	mPipeline.Bind(frameData.CommandBuffer);

	mVertexBuffer.BindVertexBuffer(frameData.CommandBuffer);

	glm::vec2 extent = { 1920.0f, 1080.0f };

	for (TextRenderData const& textData : data)
	{
		FontData& fontData = mFonts[textData.FontIndex];
		Vulkan::Buffer& glyphBuffer = mGlyphBuffers[textData.FontIndex];
		auto& glyphBufferOffset = mGlyphBufferOffsets[textData.FontIndex];

		cstr text = textData.Text.GetCString();
		glm::vec2 position = textData.ScreenPosition;
		f32 scale = textData.Scale / fontData.UnitsPerEm;

		while (*text)
		{
			u32 glyph = *(text++);

			if (glyph == '\n')
			{
				position.x = textData.ScreenPosition.x;
				position.y += 1.25f * textData.Scale;
				continue;
			}

			GlyphData& glyphData = fontData.Glyphs.contains(glyph) ? fontData.Glyphs[glyph] : fontData.MissingCharacterGlyph;

			if (glyphData.Contours.GetSize() > 0)
			{
				glm::vec2 offset = { f32(glyphData.LeftSideBearing), -glyphData.MinCoords.y };

				u32 curveCount = 0;
				for (GlyphContourData const& contour : glyphData.Contours)
					curveCount += (contour.Points.GetSize() - 1) / 2;

				{
					PushConstantData push;
					push.Color = textData.Color;
					push.Curves = glyphBuffer.GetDeviceAddress() + glyphBufferOffset[glyph];
					push.CurveCount = curveCount;
					push.ScreenPos = (position + scale * offset) * (2.0f / extent) - 1.0f;
					push.ScreenSize = 2.0f * scale * (glyphData.MaxCoords - glyphData.MinCoords) / extent;
					push.GlyphBoundsMin = glyphData.MinCoords;
					push.GlyphBoundsMax = glyphData.MaxCoords;

					vk::PushConstantsInfo pushInfo
					{
						.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
						.offset = 0,
						.size = sizeof(PushConstantData),
						.pValues = &push
					};

					mPipeline.PushConstants(frameData.CommandBuffer, pushInfo);
				}

				frameData.CommandBuffer.draw(4, 1, 0, 0);
			}

			position.x += scale * f32(glyphData.AdvanceWidth);
		}
	}
}

void TextRenderSystem::SetupDynamicState(vk::CommandBuffer inCommandBuffer) const
{
	inCommandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleStrip);
	inCommandBuffer.setDepthWriteEnable(vk::False);
	inCommandBuffer.setConservativeRasterizationModeEXT(vk::ConservativeRasterizationModeEXT::eOverestimate);
	inCommandBuffer.setExtraPrimitiveOverestimationSizeEXT(0.0f);
}

Vulkan::Pipeline TextRenderSystem::CreatePipeline(Vulkan::ShaderCompiler const& inShaderCompiler)
{
	Vulkan::PipelineCreateInfo pipelineCreateInfo
	{
		.ShaderCode = inShaderCompiler.CompileFromFile("Source/Shaders/TextShader.slang"),
		.ShaderStages
		{
			{ vk::ShaderStageFlagBits::eVertex, "vertMain" },
			{ vk::ShaderStageFlagBits::eFragment, "fragMain" }
		},
		.InputBindingDescriptions = {{ 0, sizeof(f32), vk::VertexInputRate::eVertex }},
		.InputAttributeDescriptions = {{ 0, 0, vk::Format::eR32Sfloat, 0 }},
		.PushConstantRanges
		{{
			.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			.offset = 0,
			.size = sizeof(PushConstantData)
		}}
	};

	return mRenderer.CreatePipeline(pipelineCreateInfo);
}

void TextRenderSystem::CreateGlyphBuffer(mage::StringView fontPath)
{
	mFonts.AddDefault();
	FontData& font = mFonts.GetLast();
	font.InitFromFile(fontPath);

	mage::Array<BezierCurve> curveData;

	mGlyphBufferOffsets.AddDefault();
	auto& offsets = mGlyphBufferOffsets.GetLast();

	for (auto const& glyph : font.Glyphs)
	{
		offsets[glyph.first] = curveData.GetSize() * sizeof(BezierCurve);

		for (GlyphContourData const& contour : glyph.second.Contours)
			for (u32 i = 1; i < contour.Points.GetSize(); i += 2)
				curveData.Add({ contour.Points[i - 1], contour.Points[i], contour.Points[i + 1] });
	}

	vk::DeviceSize dataSize = curveData.GetSize() * sizeof(BezierCurve);

	Vulkan::BufferCreateInfo stagingBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	Vulkan::Buffer stagingBuffer = mRenderer.CreateBuffer(stagingBufferCreateInfo);

	stagingBuffer.Map();
	stagingBuffer.Write((void*)curveData.GetData(), dataSize);

	Vulkan::BufferCreateInfo glyphBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	mGlyphBuffers.Add(mRenderer.CreateBuffer(glyphBufferCreateInfo));

	mRenderer.SubmitSingleTimeCommands([this, &stagingBuffer](vk::CommandBuffer inCommandBuffer)
		{
			mGlyphBuffers.GetLast().CopyFromBuffer(inCommandBuffer, stagingBuffer);
		});
}

void TextRenderSystem::CreateVertexBuffer()
{
	vk::DeviceSize dataSize = 4 * sizeof(f32);

	f32 vertexData[4] = { 0.0f, 1.0f, 2.0f, 3.0f };

	Vulkan::BufferCreateInfo stagingBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	Vulkan::Buffer stagingBuffer = mRenderer.CreateBuffer(stagingBufferCreateInfo);

	stagingBuffer.Map();
	stagingBuffer.Write((void*)vertexData, dataSize);

	Vulkan::BufferCreateInfo vertexBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	mVertexBuffer = mRenderer.CreateBuffer(vertexBufferCreateInfo);

	mRenderer.SubmitSingleTimeCommands([this, &stagingBuffer](vk::CommandBuffer inCommandBuffer)
		{
			mVertexBuffer.CopyFromBuffer(inCommandBuffer, stagingBuffer);
		});
}
