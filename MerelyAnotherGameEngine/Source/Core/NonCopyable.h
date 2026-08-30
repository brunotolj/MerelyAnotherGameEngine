#pragma once

class NonCopyable
{
protected:
	NonCopyable() {}
	NonCopyable(NonCopyable const&) = delete;
	NonCopyable& operator=(NonCopyable const&) = delete;
};

class NonMovable : public NonCopyable
{
protected:
	NonMovable() {}
	NonMovable(NonMovable&&) = delete;
	NonMovable& operator=(NonMovable&&) = delete;
};
