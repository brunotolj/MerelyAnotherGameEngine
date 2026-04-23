#pragma once

class NonCopyableClass
{
protected:
	NonCopyableClass() {}
	NonCopyableClass(NonCopyableClass const&) = delete;
	NonCopyableClass& operator=(NonCopyableClass const&) = delete;
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
	NonCopyableStruct(NonCopyableStruct const&) = delete;
	NonCopyableStruct& operator=(NonCopyableStruct const&) = delete;
};

class NonMovableStruct : public NonCopyableStruct
{
protected:
	NonMovableStruct() {}
	NonMovableStruct(NonMovableStruct&&) = delete;
	NonMovableStruct& operator=(NonMovableStruct&&) = delete;
};
