#pragma once

#include "Assets/Asset.h"
#include "Vulkan/Buffer.h"

namespace Vulkan
{
	class Renderer;
}

class Font : public Asset
{
	friend class Factory<Font>;

public:
	struct GlyphData
	{
		using Contour = mage::Array<glm::vec2>;

		mage::Array<Contour> Contours;
		glm::vec2 MinCoords;
		glm::vec2 MaxCoords;
		u64 BufferOffset;
		u32 TotalCurveCount;
		u16 AdvanceWidth;
		i16 LeftSideBearing;
	};

	GlyphData const& GetGlyphData(u32 inGlyphIndex) const;

	vk::DeviceAddress GetGlyphBufferDeviceAddress() const;
	u16 GetUnitsPerEm() const;

private:
	Font() {}

	void CreateGlyphBuffer(Vulkan::Renderer const& inRenderer);

	std::unordered_map<u32, GlyphData> mGlyphs;
	Vulkan::Buffer mGlyphBuffer = nullptr;
	u16 mUnitsPerEm;
};
