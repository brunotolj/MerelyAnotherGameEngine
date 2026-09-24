#pragma once

namespace mage
{
	constexpr u32 CalcLength(cstr inCstr) { return (*inCstr) ? (1 + CalcLength(inCstr + 1)) : 0; }

	class StringView
	{
	public:
		constexpr StringView() : StringView("") {}

		constexpr StringView(cstr inCstr, u32 inLength = mage::InvalidIndex)
		{
			mData = inCstr;
			mLength = (inLength == mage::InvalidIndex) ? CalcLength(inCstr) : inLength;
		}

		cstr GetCString() const { return mData; }
		u32 GetLength() const { return mLength; }

		bool operator==(mage::StringView inOther) const
		{
			if (inOther.mLength != mLength)
				return false;

			for (u32 i = 0; i < mLength; ++i)
				if (mData[i] != inOther.mData[i])
					return false;

			return true;
		}

	private:
		cstr mData = nullptr;
		u32 mLength = 0;
	};

	template <typename T>
	T ParseNumber(StringView inString, T inDefaultValue = T(0))
	{
		if (inString.GetLength() == 0) return inDefaultValue;
		char* end;
		f64 value = strtod(inString.GetCString(), &end);
		return *end ? inDefaultValue : T(value);
	}
}
