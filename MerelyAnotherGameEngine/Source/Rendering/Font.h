#pragma once

struct GlyphContourData
{
	mage::Array<glm::vec2> Points;
};

struct GlyphData
{
	mage::Array<GlyphContourData> Contours;
	glm::vec2 MinCoords;
	glm::vec2 MaxCoords;
	u16 AdvanceWidth;
	i16 LeftSideBearing;
};

struct FontData
{
	void InitFromFile(mage::StringView inPath);

	std::unordered_map<u32, GlyphData> Glyphs;
	GlyphData MissingCharacterGlyph;
	u16 UnitsPerEm;
};
