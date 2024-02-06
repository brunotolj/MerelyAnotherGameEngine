#pragma once

#include <functional>

namespace mage
{
	template<typename T, typename... Rest>
	inline void HashCombine(std::size_t& seed, T const& v, Rest &&... rest)
	{
		seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(HashCombine(seed, rest), ...);
	};
}
