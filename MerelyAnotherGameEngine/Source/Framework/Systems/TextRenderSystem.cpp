#include "Framework/Systems/TextRenderSystem.h"
#include "Engine/Engine.h"

WorldComponentFactoryFunction TextRenderSystemFactoryFunction("TextRenderSystem", [](GameWorld& inWorld, PropertyContainer const& inProperties)
{
	inWorld.CreateComponent<TextRenderSystem>();
});

TextRenderSystem::TextRenderSystem(GameWorld& inWorld) : GameSystemWithPrerequisites(inWorld)
{
	CreatePipeline();
	CreateVertexBuffer();
}

void TextRenderSystem::Update(f32 inDeltaTime)
{
	Vulkan::RenderFrameData frameData = Get<Vulkan::Renderer>().GetCurrentFrameData();
	mage::Array<TextRenderData> textData;

	for (TextEntity* textEntity : mWorld.GetEntities<TextEntity>())
	{
		textData.AddConstruct(
			textEntity->mText,
			textEntity->mColor,
			textEntity->mScreenPosition,
			textEntity->mScale,
			textEntity->mFont);
	}

	RenderText(frameData, textData);
}

void TextRenderSystem::RenderText(Vulkan::RenderFrameData const& frameData, mage::Array<TextRenderData> const& data) const
{
	SetupDynamicState(frameData.CommandBuffer);
	mPipeline.Bind(frameData.CommandBuffer);

	mVertexBuffer.BindVertexBuffer(frameData.CommandBuffer);

	glm::vec2 extent = { 1920.0f, 1080.0f };

	for (TextRenderData const& textData : data)
	{
		Font const* font = textData.Font.GetAsset();
		if (!mage_ensure(font))
			continue;

		cstr text = textData.Text.GetCString();
		glm::vec2 position = textData.ScreenPosition;
		f32 scale = textData.Scale / font->GetUnitsPerEm();

		while (*text)
		{
			u32 glyph = *(text++);

			if (glyph == '\n')
			{
				position.x = textData.ScreenPosition.x;
				position.y += 1.25f * textData.Scale;
				continue;
			}

			Font::GlyphData const& glyphData = font->GetGlyphData(glyph);

			if (glyphData.Contours.GetSize() > 0)
			{
				glm::vec2 offset = { f32(glyphData.LeftSideBearing), -glyphData.MinCoords.y };

				{
					PushConstantData push;
					push.Color = textData.Color;
					push.Curves = font->GetGlyphBufferDeviceAddress() + glyphData.BufferOffset;
					push.CurveCount = glyphData.TotalCurveCount;
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

void TextRenderSystem::CreatePipeline()
{
	Vulkan::Pipeline::CreateInfo pipelineCreateInfo
	{
		.ShaderCode = gEngine->mShaderCompiler.CompileFromFile("Source/Shaders/TextShader.slang"),
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

	mPipeline.Create(pipelineCreateInfo);
}

void TextRenderSystem::CreateVertexBuffer()
{
	vk::DeviceSize dataSize = 4 * sizeof(f32);

	f32 vertexData[4] = { 0.0f, 1.0f, 2.0f, 3.0f };

	Vulkan::Buffer::CreateInfo stagingBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	Vulkan::Buffer stagingBuffer(stagingBufferCreateInfo);

	stagingBuffer.Map();
	stagingBuffer.Write((void*)vertexData, dataSize);

	Vulkan::Buffer::CreateInfo vertexBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	mVertexBuffer.Create(vertexBufferCreateInfo);

	gEngine->mVulkanDevice.SubmitSingleTimeCommands([this, &stagingBuffer](vk::CommandBuffer inCommandBuffer)
	{
		mVertexBuffer.CopyFromBuffer(inCommandBuffer, stagingBuffer);
	});
}
