#pragma once

namespace mage
{
	class String : public Array<char>
	{
	};

	class StringView
	{
	public:
		StringView(const char* cstr)
		{
			data = cstr;
			length = (u32)strlen(cstr);
		}

		StringView(const String& string)
		{
			data = string.GetData();
			length = string.GetSize();
		}

		const char* GetCString() const { return data; }
		u32 GetLength() const { return length; }

	private:
		const char* data = nullptr;
		u32 length = 0;
	};
}
