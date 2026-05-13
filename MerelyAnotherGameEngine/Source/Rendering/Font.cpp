#include "Rendering/Font.h"

void FontData::InitFromFile(mage::StringView inPath)
{
	u8* reader;
	std::unordered_map<u32, u32> tableLocations;

	auto read8 = [&reader]() -> u8 { return *(reader++); };
	auto read16 = [&reader]() -> u16 { u16 res = *(reader++); res = (res << 8) | *(reader++); return res; };
	auto read32 = [&reader]() -> u32 { u32 res = *(reader++); res = (res << 8) | *(reader++); res = (res << 8) | *(reader++); res = (res << 8) | *(reader++); return res; };
	auto location = [&tableLocations](cstr table) { return tableLocations[u32(table[0]) << 24 | u32(table[1]) << 16 | u32(table[2]) << 8 | u32(table[3])]; };

	mage::Array<u8> file = mage::ReadFile(inPath);

	reader = file.GetData();
	reader += 4;

	u16 numTables = read16();
	reader += 6;

	for (u16 i = 0; i < numTables; ++i)
	{
		u32 tag = read32();

		reader += 4;
		tableLocations[tag] = read32();
		reader += 4;
	}

	reader = file.GetData() + location("head") + 18;

	UnitsPerEm = read16();

	reader += 30;

	bool isLongEntry = read16();
	u32 glyphLocationStride = isLongEntry ? 4 : 2;

	reader = file.GetData() + location("maxp") + 4;

	u16 numGlyphs = read16();
	mage::Array<u32> glyphLocations(numGlyphs + 1);
	mage::Array<u16> advanceWidths(numGlyphs);
	mage::Array<i16> leftSideBearings(numGlyphs);

	for (u32 i = 0; i <= numGlyphs; ++i)
	{
		reader = file.GetData() + location("loca") + i * glyphLocationStride;
		glyphLocations[i] = location("glyf") + (isLongEntry ? read32() : 2u * read16());
	}

	reader = file.GetData() + location("hhea") + 34;

	u16 numAdvanceWidthMetrics = read16();

	reader = file.GetData() + location("hmtx");

	u16 advanceWidth = 0;
	for (u32 i = 0; i < numGlyphs; ++i)
	{
		if (i < numAdvanceWidthMetrics)
			advanceWidth = read16();

		advanceWidths[i] = advanceWidth;
		leftSideBearings[i] = i16(read16());
	}

	struct GlyphRawData
	{
		glm::i16vec2 MinCoords;
		glm::i16vec2 MaxCoords;
		mage::Array<u16> ContourEndIndices;
		mage::Array<glm::vec2> Points;
		mage::Array<bool> OnCurveFlags;
	};

	struct CompoundGlyphData
	{
		u16 Index;
		mage::Array<u16> Glyphs;
		mage::Array<glm::mat2x3> Transforms;
	};

	mage::Array<GlyphRawData> glyphRawData(numGlyphs);
	mage::Array<CompoundGlyphData> compoundGlyphData;

	for (u32 glyphIndex = 0; glyphIndex < numGlyphs; ++glyphIndex)
	{
		GlyphRawData& rawData = glyphRawData[glyphIndex];

		if (glyphLocations[glyphIndex] == glyphLocations[glyphIndex + 1])
			continue;

		reader = file.GetData() + glyphLocations[glyphIndex];
		i16 contourCount = i16(read16());

		rawData.MinCoords.x = i16(read16());
		rawData.MinCoords.y = i16(read16());
		rawData.MaxCoords.x = i16(read16());
		rawData.MaxCoords.y = i16(read16());

		if (contourCount < 0)
		{
			compoundGlyphData.AddDefault();
			CompoundGlyphData& compoundData = compoundGlyphData.GetLast();
			
			compoundData.Index = glyphIndex;
			
			while (true)
			{
				u16 flags = read16();
			
				mage_check((flags >> 1) & 1);
				mage_check(((flags >> 7) & 1) == 0);
			
				compoundData.Glyphs.Add(read16());
			
				glm::vec3 transformX(1.0f, 0.0f, f32((flags & 1) ? i16(read16()) : i8(read8())));
				glm::vec3 transformY(0.0f, 1.0f, f32((flags & 1) ? i16(read16()) : i8(read8())));
			
				if ((flags >> 3) & 1)
				{
					transformX.x = i16(read16()) / f32(1 << 14);
					transformY.y = transformX.x;
				}
				else if ((flags >> 6) & 1)
				{
					transformX.x = i16(read16()) / f32(1 << 14);
					transformY.y = i16(read16()) / f32(1 << 14);
				}

				compoundData.Transforms.AddConstruct(transformX, transformY);

				if (((flags >> 5) & 1) == 0)
					break;
			}
		}
		else if (contourCount > 0)
		{
			rawData.ContourEndIndices.ResizeUninitialized(contourCount);

			for (u16& contourEndIndex : rawData.ContourEndIndices)
				contourEndIndex = read16();

			u32 numPoints = rawData.ContourEndIndices.GetLast() + 1;

			mage::Array<u8> flags(numPoints, false);
			rawData.Points.ResizeUninitialized(numPoints);
			rawData.OnCurveFlags.ResizeUninitialized(numPoints);

			reader += read16();

			for (u32 pointIndex = 0; pointIndex < numPoints; ++pointIndex)
			{
				flags[pointIndex] = read8();
				rawData.OnCurveFlags[pointIndex] = flags[pointIndex] & 1;

				if ((flags[pointIndex] >> 3) & 1)
				{
					u8 flag = flags[pointIndex];
					u8 repeat = read8();
					for (u8 i = 0; i < repeat; ++i)
						flags[++pointIndex] = flag;
				}
			}

			for (u32 pointIndex = 0; pointIndex < numPoints; ++pointIndex)
			{
				u8 flag = flags[pointIndex];
				rawData.Points[pointIndex].x = pointIndex ? rawData.Points[pointIndex - 1].x : 0;

				if ((flag >> 1) & 1)
				{
					i16 offset = read8();
					rawData.Points[pointIndex].x += ((flag >> 4) & 1) ? offset : -offset;
				}
				else if (((flag >> 4) & 1) == 0)
					rawData.Points[pointIndex].x += i16(read16());
			}

			for (u32 pointIndex = 0; pointIndex < numPoints; ++pointIndex)
			{
				u8 flag = flags[pointIndex];
				rawData.Points[pointIndex].y = pointIndex ? rawData.Points[pointIndex - 1].y : 0;

				if ((flag >> 2) & 1)
				{
					i16 offset = read8();
					rawData.Points[pointIndex].y += ((flag >> 5) & 1) ? offset : -offset;
				}
				else if (((flag >> 5) & 1) == 0)
					rawData.Points[pointIndex].y += i16(read16());
			}
		}
	}

	while (compoundGlyphData.GetSize() > 0)
	{
		bool madeProgress = false;

		for (u32 compoundIndex = 0; compoundIndex < compoundGlyphData.GetSize(); ++compoundIndex)
		{
			CompoundGlyphData& compoundData = compoundGlyphData[compoundIndex];

			bool missingComponent = false;
			for (u16 componentIndex : compoundData.Glyphs)
				if (glyphRawData[componentIndex].Points.GetSize() == 0)
				{
					missingComponent = true;
					break;
				}

			if (missingComponent)
				continue;

			GlyphRawData& rawData = glyphRawData[compoundData.Index];

			for (u32 componentIndex = 0; componentIndex < compoundData.Glyphs.GetSize(); ++componentIndex)
			{
				GlyphRawData& componentRawData = glyphRawData[compoundData.Glyphs[componentIndex]];
				
				u32 existingPointCount = rawData.Points.GetSize();

				for (u16 contourEndIndex : componentRawData.ContourEndIndices)
					rawData.ContourEndIndices.Add(contourEndIndex + existingPointCount);

				glm::mat2x3 transform = compoundData.Transforms[componentIndex];

				for (glm::vec2 point : componentRawData.Points)
					rawData.Points.AddConstruct(glm::vec3(point, 1.0f) * transform);

				for (bool onCurveFlag : componentRawData.OnCurveFlags)
					rawData.OnCurveFlags.Add(onCurveFlag);
			}

			compoundGlyphData.RemoveAtSwap(compoundIndex--);
			madeProgress = true;
		}

		mage_check(madeProgress);
	}

	mage::Array<glm::i32vec2> unicodeMapping;

	{
		reader = file.GetData() + location("cmap");

		u16 version = read16();
		u16 numSubtables = read16();

		u32 subtableOffset = 0;
		i32 unicodeVersionId = -1;

		for (u16 i = 0; i < numSubtables; ++i)
		{
			u16 platformId = read16();
			u16 platformSpecificId = read16();

			u32 offset = read32();

			if (platformId != 0)
				continue;

			if (platformSpecificId > unicodeVersionId)
			{
				subtableOffset = offset;
				unicodeVersionId = platformSpecificId;
			}
		}

		mage_check(subtableOffset);

		reader = file.GetData() + location("cmap") + subtableOffset;

		u16 format = read16();

		mage_check(format == 4 || format == 12);

		if (format == 4)
		{
			u16 length = read16();
			u16 languageCode = read16();

			u16 segCount = read16() / 2;
			reader += 6;

			mage::Array<u16> endCodes(segCount);
			for (u16& endCode : endCodes)
				endCode = read16();

			reader += 2;

			mage::Array<u16> startCodes(segCount);
			for (u16& startCode : startCodes)
				startCode = read16();

			mage::Array<u16> idDeltas(segCount);
			for (u16& idDelta : idDeltas)
				idDelta = read16();

			mage::Array<u32> idRangeOffsets(segCount);
			for (u32& idRangeOffset : idRangeOffsets)
			{
				u16 offset = read16();
				idRangeOffset = offset ? u32(reader - file.GetData()) + offset - 2 : 0;
			}

			for (u32 i = 0; i < startCodes.GetSize(); ++i)
			{
				u16 startCode = startCodes[i];
				u16 codeCount = endCodes[i] - startCode + 1;

				for (u16 code = 0; code < codeCount; ++code)
				{
					if (idRangeOffsets[i] == 0)
					{
						unicodeMapping.AddConstruct(code + startCode, (code + startCode + idDeltas[i]) & 0xFFFF);
					}
					else
					{
						u32 glyphIndexArrayLocation = 2 * code + idRangeOffsets[i];

						u8* readerBackup = reader;
						reader = file.GetData() + glyphIndexArrayLocation;

						u16 glyphIndex = read16();
						if (glyphIndex != 0)
							glyphIndex = (glyphIndex + idDeltas[i]) & 0xFFFF;

						reader = readerBackup;

						unicodeMapping.AddConstruct(code + startCode, glyphIndex);
					}
				}
			}
		}
		else if (format == 12)
		{
			reader += 10;
			u32 numGroups = read32();

			for (u32 i = 0; i < numGroups; ++i)
			{
				u32 startCharCode = read32();
				u32 endCharCode = read32();
				u32 startGlyphIndex = read32();

				u32 codeCount = endCharCode - startCharCode + 1;

				for (u32 code = 0; code < codeCount; ++code)
				{
					unicodeMapping.AddConstruct(code + startCharCode, code + startGlyphIndex);
				}
			}
		}
	}

	for (glm::u32vec2 mapping : unicodeMapping)
	{
		GlyphData& glyph = Glyphs[mapping.x];
		GlyphRawData& rawData = glyphRawData[mapping.y];

		glyph.MinCoords = rawData.MinCoords;
		glyph.MaxCoords = rawData.MaxCoords;
		glyph.AdvanceWidth = advanceWidths[mapping.y];
		glyph.LeftSideBearing = leftSideBearings[mapping.y];

		glyph.Contours.ResizeDefault(rawData.ContourEndIndices.GetSize());

		u32 contourStart = 0;
		u32 contourIndex = 0;

		mage::Array<glm::vec2> contourPoints;

		for (u32 pointIndex = 0; pointIndex < rawData.Points.GetSize(); ++pointIndex)
		{
			glm::vec2 point = rawData.Points[pointIndex];
			contourPoints.AddConstruct(point);

			u32 nextIndex = pointIndex + 1;

			if (pointIndex == rawData.ContourEndIndices[contourIndex])
				std::swap(nextIndex, contourStart);

			glm::vec2 nextPoint = rawData.Points[pointIndex];

			if (rawData.OnCurveFlags[pointIndex] == rawData.OnCurveFlags[nextIndex])
				contourPoints.Add(0.5f * (glm::vec2(point) + glm::vec2(nextPoint)));

			if (nextIndex < pointIndex)
			{
				contourPoints.Add(contourPoints.GetLast());

				GlyphContourData& contour = glyph.Contours[contourIndex];

				for (u32 i = 1; i < contourPoints.GetSize(); i += 2)
				{
					glm::vec2 p0 = contourPoints[i - 1];
					glm::vec2 p1 = contourPoints[i];
					glm::vec2 p2 = contourPoints[i + 1];

					contour.Points.Add(p0);

					glm::vec2 d0 = p0 - p1;
					glm::vec2 d1 = p2 - p1;

					glm::vec2 a = d0 + d1;
					glm::vec2 b = -2.0f * d0;

					glm::vec2 t = d0 / a;

					glm::vec2 vspos = a * t.x * t.x + b * t.x + p0;
					glm::vec2 hspos = a * t.y * t.y + b * t.y + p0;

					glm::vec2 extv0 = (vspos - p0) / d0; extv0 = p0 + d0 * glm::vec2(extv0.y, extv0.x);
					glm::vec2 extv1 = (vspos - p2) / d1; extv1 = p2 + d1 * glm::vec2(extv1.y, extv1.x);
					glm::vec2 exth0 = (hspos - p0) / d0; exth0 = p0 + d0 * glm::vec2(exth0.y, exth0.x);
					glm::vec2 exth1 = (hspos - p2) / d1; exth1 = p2 + d1 * glm::vec2(exth1.y, exth1.x);

					bool v = (p1.x < p0.x && p1.x < p2.x) || (p1.x > p0.x && p1.x > p2.x);
					bool h = (p1.y < p0.y && p1.y < p2.y) || (p1.y > p0.y && p1.y > p2.y);

					if (v && h)
					{
						if (t.x < t.y)
						{
							contour.Points.AddConstruct(vspos.x, extv0.y);
							contour.Points.AddConstruct(vspos.x, vspos.y);
							contour.Points.AddConstruct(vspos.x, hspos.y);
							contour.Points.AddConstruct(hspos.x, hspos.y);
							contour.Points.AddConstruct(exth1.x, hspos.y);
						}
						else
						{
							contour.Points.AddConstruct(extv0.x, hspos.y);
							contour.Points.AddConstruct(hspos.x, hspos.y);
							contour.Points.AddConstruct(vspos.x, hspos.y);
							contour.Points.AddConstruct(vspos.x, vspos.y);
							contour.Points.AddConstruct(vspos.x, exth1.y);
						}
					}
					else if (v)
					{
						contour.Points.AddConstruct(vspos.x, extv0.y);
						contour.Points.AddConstruct(vspos.x, vspos.y);
						contour.Points.AddConstruct(vspos.x, extv1.y);
					}
					else if (h)
					{
						contour.Points.AddConstruct(exth0.x, hspos.y);
						contour.Points.AddConstruct(hspos.x, hspos.y);
						contour.Points.AddConstruct(exth1.x, hspos.y);
					}
					else
						contour.Points.Add(p1);
				}

				contour.Points.Add(contourPoints[0]);

				contourPoints.Empty();
				contourIndex++;
			}
		}
	}

	MissingCharacterGlyph = Glyphs[0xFFFF];
}
