#pragma once

#include "Core/Array.h"

namespace mage
{
	class StringView
	{
	public:
		constexpr StringView() : StringView("") {}

		constexpr StringView(cstr inCstr)
		{
			mData = inCstr;
			mLength = CalcLength(inCstr);
		}

		cstr GetCString() const { return mData; }
		u32 GetLength() const { return mLength; }

	private:
		constexpr u32 CalcLength(cstr inCstr) { return (*inCstr) ? (1 + CalcLength(inCstr + 1)) : 0; }

		cstr mData = nullptr;
		u32 mLength = 0;
	};
}
