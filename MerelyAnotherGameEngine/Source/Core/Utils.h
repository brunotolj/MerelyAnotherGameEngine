#pragma once

#include <fstream>
#include <functional>

namespace mage
{
	template<typename Type, typename... Rest>
	inline void HashCombine(u64& inSeed, Type const& inValue, Rest &&... inRest)
	{
		inSeed ^= std::hash<Type>{}(inValue) + 0x9e3779b9 + (inSeed << 6) + (inSeed >> 2);
		(HashCombine(inSeed, inRest), ...);
	};

	inline mage::Array<u8> ReadFile(mage::StringView inPath)
	{
		mage::Array<u8> data;

		std::ifstream file(inPath.GetCString(), std::ios::ate | std::ios::binary);
		mage_ensure(file.is_open());

		u32 fileSize = u32(file.tellg());
		data.ResizeUninitialized(fileSize + 1);

		file.seekg(0);
		file.read((char*)data.GetData(), fileSize);
		data.GetLast() = 0;

		return data;
	}

	enum BreakOrContinue : bool
	{
		Break,
		Continue
	};
}
