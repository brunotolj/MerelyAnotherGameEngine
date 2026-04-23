#pragma once

#include <fstream>
#include <functional>

namespace mage
{
	template<typename T, typename... Rest>
	inline void HashCombine(u64& inSeed, T const& inValue, Rest &&... inRest)
	{
		inSeed ^= std::hash<T>{}(inValue) + 0x9e3779b9 + (inSeed << 6) + (inSeed >> 2);
		(HashCombine(inSeed, inRest), ...);
	};

	inline String ReadFile(mage::StringView inPath)
	{
		String data;

		std::ifstream file(inPath.GetCString(), std::ios::ate | std::ios::binary);
		mage_ensure(file.is_open());

		u32 fileSize = u32(file.tellg());
		data.ResizeUninitialized(fileSize + 1);

		file.seekg(0);
		file.read(data.GetData(), fileSize);
		data[fileSize] = '\0';

		return data;
	}
}
