#include "Assets/Font.h"
#include "Engine/Engine.h"

Font::GlyphData const& Font::GetGlyphData(u32 inGlyphIndex) const
{
	if (mGlyphs.contains(inGlyphIndex))
		return mGlyphs.at(inGlyphIndex);

	mage_check(mGlyphs.contains(0xFFFF));
	return mGlyphs.at(0xFFFF);
}

vk::DeviceAddress Font::GetGlyphBufferDeviceAddress() const
{
	return mGlyphBuffer.GetDeviceAddress();
}

u16 Font::GetUnitsPerEm() const
{
	return mUnitsPerEm;
}

void Font::CreateGlyphBuffer()
{
	mage::Array<glm::vec2> curveData;

	for (auto& glyph : mGlyphs)
	{
		glyph.second.BufferOffset = curveData.GetSize() * sizeof(glm::vec2);

		for (GlyphData::Contour const& contour : glyph.second.Contours)
			for (u32 i = 1; i < contour.GetSize(); i += 2)
			{
				curveData.Add(contour[i - 1]);
				curveData.Add(contour[i]);
				curveData.Add(contour[i + 1]);
			}
	}

	vk::DeviceSize dataSize = curveData.GetSize() * sizeof(glm::vec2);

	Vulkan::Buffer::CreateInfo stagingBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	Vulkan::Buffer stagingBuffer(stagingBufferCreateInfo);

	stagingBuffer.Map();
	stagingBuffer.Write((void*)curveData.GetData(), dataSize);

	Vulkan::Buffer::CreateInfo glyphBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	mGlyphBuffer.Create(glyphBufferCreateInfo);

	gEngine->mVulkanDevice.SubmitSingleTimeCommands([this, &stagingBuffer](vk::CommandBuffer inCommandBuffer)
		{
			mGlyphBuffer.CopyFromBuffer(inCommandBuffer, stagingBuffer);
		});
}
