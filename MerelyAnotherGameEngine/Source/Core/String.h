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

		String(String const& inOther) { *this = inOther; }
		String(String&& inOther) { *this = std::move(inOther); }

		String& operator=(String const& inOther)
		{
			InitFrom_Copy(inOther.GetData(), inOther.GetSize());
			return *this;
		}

		String& operator=(String&& inOther)
		{
			Empty();

			mElements = inOther.mElements;
			mCapacity = inOther.mCapacity;
			mSize = inOther.mSize;

			inOther.mElements = nullptr;
			inOther.mCapacity = 0;
			inOther.mSize = 0;

			inOther.AppendNull();

			return *this;
		}

		template <NumericType T>
		String& FromNumber(T inValue, u32 inPrecision = 0)
		{
			i32 size = snprintf(nullptr, 0, "%.*f", inPrecision, f64(inValue));
			ResizeUninitialized(size + 1);
			snprintf(GetData(), size + 1, "%.*f", inPrecision, f64(inValue));
			return *this;
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
