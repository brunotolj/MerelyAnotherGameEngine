#pragma once

template <typename T>
concept Enumeration = std::is_enum<T>::value;

template <Enumeration E>
class FlagSet
{
public:
	template <Enumeration Flag, Enumeration... Rest>
	void Set(Flag inFlag, Rest... inFlags)
	{
		Set(inFlag);
		Set(inFlags...);
	}

	template <Enumeration Flag>
	void Set(Flag inFlag)
	{
		mValue = E(u64(mValue) | u64(inFlag));
	}

	template <Enumeration Flag, Enumeration... Rest>
	void Reset(Flag inFlag, Rest... inFlags)
	{
		Reset(inFlag);
		Reset(inFlags...);
	}

	template <Enumeration Flag>
	void Reset(Flag inFlag)
	{
		mValue = E(u64(mValue) & ~u64(inFlag));
	}

	template <Enumeration... Flags>
	bool HasAny(Flags... inFlags) const
	{
		FlagSet test;
		test.Set(inFlags...);

		return (u64(mValue) & u64(test.mValue)) != 0;
	}

	template <Enumeration... Flags>
	bool HasAll(Flags... inFlags) const
	{
		FlagSet test;
		test.Set(inFlags...);

		return (u64(mValue) & u64(test.mValue)) == u64(test.mValue);
	}

private:
	E mValue = E(0);
};