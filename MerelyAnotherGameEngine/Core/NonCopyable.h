#pragma once

class NonCopyableClass
{
protected:
	NonCopyableClass() {}
	NonCopyableClass(const NonCopyableClass&) = delete;
	NonCopyableClass& operator=(const NonCopyableClass&) = delete;
};

class NonMovableClass : public NonCopyableClass
{
protected:
	NonMovableClass() {}
	NonMovableClass(NonMovableClass&&) = delete;
	NonMovableClass& operator=(NonMovableClass&&) = delete;
};

struct NonCopyableStruct
{
protected:
	NonCopyableStruct() {}
	NonCopyableStruct(const NonCopyableStruct&) = delete;
	NonCopyableStruct& operator=(const NonCopyableStruct&) = delete;
};

class NonMovableStruct : public NonCopyableStruct
{
protected:
	NonMovableStruct() {}
	NonMovableStruct(NonMovableStruct&&) = delete;
	NonMovableStruct& operator=(NonMovableStruct&&) = delete;
};
