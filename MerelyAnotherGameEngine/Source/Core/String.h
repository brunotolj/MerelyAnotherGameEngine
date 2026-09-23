#pragma once

#include "Core/Array.h"

namespace mage
{
	class String : Array<char>
	{
		friend std::hash<String>;

	public:
		String(cstr inCstr = nullptr)
		{
			if (inCstr)
			{
				Reserve(CalcLength(inCstr) + 1);
				while (*inCstr)
				{
					Add(*(inCstr++));
				}
			}

			AppendNull();
		}

		String(StringView inStringView) : String(inStringView.GetCString())
		{
		}

		String& Append(char inChar)
		{
			GetLast() = inChar;
			AppendNull();
			return *this;
		}

		String& Append(StringView inStringView)
		{
			RemoveAt(GetLength());

			for (cstr str = inStringView.GetCString(); *str; ++str)
				Add(*str);

			AppendNull();
			return *this;
		}

		void Empty()
		{
			Array::Empty();
			AppendNull();
		}

		u32 GetLength() const { return GetSize() - 1; }

		operator StringView() const
		{
			return StringView(GetData(), GetLength());
		}

		String& operator+=(char inChar)
		{
			Append(inChar);
			return *this;
		}

		bool operator==(mage::String const& inOther) const
		{
			u32 length = GetLength();
			if (inOther.GetLength() != length)
				return false;

			for (u32 i = 0; i < length; ++i)
				if (mElements[i] != inOther[i])
					return false;

			return true;
		}

	private:
		void AppendNull() { Add('\0'); }
	};
}

namespace std
{
	template<>
	struct hash<mage::String>
	{
		u64 operator()(mage::String const& inValue) const
		{
			u64 seed = 0;

			for (u32 i = 0; i < inValue.GetLength(); ++i)
				mage::HashCombine(seed, inValue[i]);

			return seed;
		}
	};

}
