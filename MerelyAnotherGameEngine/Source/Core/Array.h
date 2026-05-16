#pragma once

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <malloc.h>
#include <vector>

namespace mage
{
	template<typename Type>
	class Array
	{
	public:
		Array()
		{
		}

		Array(u32 inInitialSize, bool inDefaultElements = true)
		{
			if (inInitialSize)
				if (inDefaultElements)
					ResizeDefault(inInitialSize);
				else
					ResizeUninitialized(inInitialSize);
		}

		Array(std::initializer_list<Type> inElements)
		{
			InitFrom_Copy(inElements.begin(), u32(inElements.size()));
		}

		Array(Array const& inOther) { *this = inOther; }
		Array(Array&& inOther) { *this = std::move(inOther); }
		Array(std::vector<Type> const& inOther) { *this = inOther; }
		Array(std::vector<Type>&& inOther) { *this = std::move(inOther); }

		Array& operator=(Array const& inOther)
		{
			InitFrom_Copy(inOther.GetData(), inOther.GetSize());
			return *this;
		}

		Array& operator=(Array&& inOther)
		{
			Empty();

			mElements = inOther.mElements;
			mCapacity = inOther.mCapacity;
			mSize = inOther.mSize;

			inOther.mElements = nullptr;
			inOther.mCapacity = 0;
			inOther.mSize = 0;

			return *this;
		}

		Array& operator=(std::vector<Type> const& inOther)
		{
			InitFrom_Copy(inOther.data(), u32(inOther.size()));
			return *this;
		}

		Array& operator=(std::vector<Type>&& inOther)
		{
			InitFrom_Move(inOther.data(), u32(inOther.size()));
			inOther.clear();
			return *this;
		}

		~Array()
		{
			Empty();
			_aligned_free(mElements);
		}

		Type* GetData() const { return mElements; }
		u32 GetSize() const { return mSize; }
		bool IsEmpty() const { return mSize == 0; }

		Type& operator[](u32 inIndex) { return mElements[inIndex]; }
		Type const& operator[](u32 inIndex) const { return mElements[inIndex]; }

		Type& GetFirst() { return mElements[0]; }
		Type const& GetFirst() const { return mElements[0]; }
		Type& GetLast() { return mElements[mSize - 1]; }
		Type const& GetLast() const { return mElements[mSize - 1]; }

		u32 Add(Type const& inElement)
		{
			if (mSize == mCapacity)
				Realloc(mSize + 1);

			return AddChecked(inElement);
		}

		u32 Add(Type&& inElement)
		{
			if (mSize == mCapacity)
				Realloc(mSize + 1);

			return AddChecked(std::move(inElement));
		}

		u32 AddDefault()
		{
			if (mSize == mCapacity)
				Realloc(mSize + 1);

			return AddChecked(Type());
		}

		u32 AddUninitialized()
		{
			if (mSize == mCapacity)
				Realloc(mSize + 1);

			return mSize++;
		}

		template <typename... Args>
		u32 AddConstruct(Args&&... args)
		{
			if (mSize == mCapacity)
				Realloc(mSize + 1);

			new (mElements + mSize) Type(args...);
			return mSize++;
		}

		bool Insert(Type const& inElement, u32 inIndex)
		{
			if (inIndex > mSize)
				return false;

			if (mSize == mCapacity)
				Realloc(mSize + 1);

			for (u32 i = mSize; i > inIndex; --i)
				mElements[i] = mElements[i - 1];

			mElements[inIndex] = inElement;
			++mSize;
			return true;
		}

		bool InsertSwap(Type const& inElement, u32 inIndex)
		{
			if (inIndex > mSize)
				return false;

			if (inIndex == mSize)
			{
				Add(inElement);
				return true;
			}

			if (mSize == mCapacity)
				Realloc(mSize + 1);

			mElements[mSize] = mElements[inIndex];
			mElements[inIndex] = inElement;
			++mSize;
			return true;
		}

		bool Remove(Type const& inElement)
		{
			for (u32 i = 0; i < mSize; ++i)
			{
				if (mElements[i] == inElement)
				{
					--mSize;

					for (u32 j = i; j < mSize; ++j)
						mElements[j] = std::move(mElements[j + 1]);

					mElements[mSize].~Type();
					return true;
				}
			}

			return false;
		}

		bool RemoveSwap(Type const& inElement)
		{
			for (u32 i = 0; i < mSize; ++i)
			{
				if (mElements[i] == inElement)
				{
					--mSize;

					mElements[i] = std::move(mElements[mSize]);

					mElements[mSize].~Type();
					return true;
				}
			}

			return false;

		}

		bool RemoveAt(u32 inIndex)
		{
			if (inIndex >= mSize)
				return false;

			--mSize;

			for (u32 i = inIndex; i < mSize; ++i)
				mElements[i] = std::move(mElements[i + 1]);

			mElements[mSize].~Type();

			return true;
		}

		bool RemoveAtSwap(u32 inIndex)
		{
			if (inIndex >= mSize)
				return false;

			--mSize;

			mElements[inIndex] = std::move(mElements[mSize]);

			mElements[mSize].~Type();
			return true;
		}

		void Reserve(u32 inNewCapacity, bool inAllowShrinking = true)
		{
			if (inNewCapacity < mCapacity && !inAllowShrinking)
				return;

			Realloc(inNewCapacity);
		}

		void ResizeDefault(u32 inNewSize)
		{
			Reserve(inNewSize, false);

			for (u32 i = inNewSize; i < mSize; ++i)
				mElements[i].~Type();

			for (u32 i = mSize; i < inNewSize; ++i)
				new (mElements + i) Type();

			mSize = inNewSize;
		}

		void ResizeUninitialized(u32 inNewSize)
		{
			Reserve(inNewSize, false);

			for (u32 i = inNewSize; i < mSize; ++i)
				mElements[i].~Type();

			mSize = inNewSize;
		}

		void Empty()
		{
			for (u32 i = 0; i < mSize; ++i)
				mElements[i].~Type();

			mSize = 0;
		}

		void Reset(u32 inExpectedCapacity = 0, bool inAllowShrinking = true)
		{
			Empty();
			Reserve(inExpectedCapacity, inAllowShrinking);
		}

		Type* Find(Type const& inElement)
		{
			for (Type& element : *this)
				if (element == inElement)
					return &element;

			return nullptr;
		}

		Type const* Find(Type const& inElement) const
		{
			for (Type const& element : *this)
				if (element == inElement)
					return &element;

			return nullptr;
		}

		Type* Find(std::function<bool(Type const&)>&& inPredicate)
		{
			for (Type& element : *this)
				if (inPredicate(element))
					return &element;

			return nullptr;
		}

		Type const* Find(std::function<bool(Type const&)>&& inPredicate) const
		{
			for (Type const& element : *this)
				if (inPredicate(element))
					return &element;

			return nullptr;
		}

		bool Contains(Type const& inElement) const
		{
			return Find(inElement) != nullptr;
		}

		bool Contains(std::function<bool(Type const&)>&& inPredicate) const
		{
			return Find(std::move(inPredicate)) != nullptr;
		}

		void Sort()
		{
			std::sort(begin(), end());
		}

		Type* begin() const { return mElements; }
		Type* end() const { return mElements + mSize; }

	private:
		void Realloc(u32 inDesiredCapacity)
		{
			if (inDesiredCapacity < mSize)
				inDesiredCapacity = mSize;

			u32 newCapacity = cMinCapacity;
			while (newCapacity < inDesiredCapacity)
				newCapacity = newCapacity << 1 | 1;

			if (newCapacity == mCapacity)
				return;

			mCapacity = newCapacity;

			if (mSize)
			{
				Type* newElements = (Type*)_aligned_malloc(mCapacity * sizeof(Type), alignof(Type));
				mage_check(newElements);
				memcpy(newElements, mElements, mSize * sizeof(Type));
				_aligned_free(mElements);
				mElements = newElements;
			}
			else
			{
				_aligned_free(mElements);
				mElements = (Type*)_aligned_malloc(mCapacity * sizeof(Type), alignof(Type));
				mage_check(mElements);
			}
		}

		void InitFrom_Copy(Type const* inFirst, u32 inSize)
		{
			Empty();
			Reserve(inSize);

			for (u32 i = 0; i < inSize; ++i)
				AddChecked(inFirst[i]);
		}

		void InitFrom_Move(Type* inFirst, u32 inSize)
		{
			Empty();
			Reserve(inSize);

			for (u32 i = 0; i < inSize; ++i)
				AddChecked(std::move(inFirst[i]));
		}

		u32 AddChecked(Type const& inElement)
		{
			new (mElements + mSize) Type(inElement);
			return mSize++;
		}

		u32 AddChecked(Type&& inElement)
		{
			new (mElements + mSize) Type(std::move(inElement));
			return mSize++;
		}

		Type* mElements = nullptr;
		u32 mCapacity = 0;
		u32 mSize = 0;

		static constexpr u32 cMinCapacity = u32(7);
	};
}
