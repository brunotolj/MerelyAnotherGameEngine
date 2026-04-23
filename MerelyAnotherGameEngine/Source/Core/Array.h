#pragma once

#include <functional>
#include <initializer_list>
#include <malloc.h>
#include <vector>

namespace mage
{
	template<typename T>
	class Array
	{
	public:
		Array(u32 inInitialSize = 0)
		{
			Empty();
			
			if (inInitialSize)
				ResizeUninitialized(inInitialSize);
		}

		Array(std::initializer_list<T> inElements)
		{
			InitFrom_Copy(inElements.begin(), u32(inElements.size()));
		}

		Array(Array const& inOther) { *this = inOther; }
		Array(Array&& inOther) { *this = std::move(inOther); }
		Array(std::vector<T> const& inOther) { *this = inOther; }
		Array(std::vector<T>&& inOther) { *this = std::move(inOther); }

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

		Array& operator=(std::vector<T> const& inOther)
		{
			InitFrom_Copy(inOther.data(), u32(inOther.size()));
			return *this;
		}

		Array& operator=(std::vector<T>&& inOther)
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

		T* GetData() const { return mElements; }
		u32 GetSize() const { return mSize; }
		bool IsEmpty() const { return mSize == 0; }

		T& operator[](u32 inIndex) { return mElements[inIndex]; }
		T const& operator[](u32 inIndex) const { return mElements[inIndex]; }

		u32 Add(T const& inElement)
		{
			if (mSize == mCapacity)
				Realloc(mSize + 1);

			return AddChecked(inElement);
		}

		u32 Add(T&& inElement)
		{
			if (mSize == mCapacity)
				Realloc(mSize + 1);

			return AddChecked(std::move(inElement));
		}

		u32 AddDefault()
		{
			if (mSize == mCapacity)
				Realloc(mSize + 1);

			return AddChecked(T());
		}

		u32 AddUninitialized()
		{
			if (mSize == mCapacity)
				Realloc(mSize + 1);

			return mSize++;
		}

		template <typename... Args>
		u32 AddConstruct(Args... args)
		{
			if (mSize == mCapacity)
				Realloc(mSize + 1);

			new (mElements + mSize) T(args...);
			return mSize++;
		}

		bool Insert(T const& inElement, u32 inIndex)
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

		bool InsertSwap(T const& inElement, u32 inIndex)
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

		bool Remove(T const& inElement)
		{
			for (u32 i = 0; i < mSize; ++i)
			{
				if (mElements[i] == inElement)
				{
					--mSize;

					for (u32 j = i; j < mSize; ++j)
						mElements[j] = std::move(mElements[j + 1]);

					mElements[mSize].~T();
					return true;
				}
			}

			return false;
		}

		bool RemoveSwap(T const& inElement)
		{
			for (u32 i = 0; i < mSize; ++i)
			{
				if (mElements[i] == inElement)
				{
					--mSize;

					mElements[i] = std::move(mElements[mSize]);

					mElements[mSize].~T();
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

			mElements[mSize].~T();

			return true;
		}

		bool RemoveAtSwap(u32 inIndex)
		{
			if (inIndex >= mSize)
				return false;

			--mSize;

			mElements[inIndex] = std::move(mElements[mSize]);

			mElements[mSize].~T();
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
				mElements[i].~T();

			for (u32 i = mSize; i < inNewSize; ++i)
				mElements[i] = T();

			mSize = inNewSize;
		}

		void ResizeUninitialized(u32 inNewSize)
		{
			Reserve(inNewSize, false);

			for (u32 i = inNewSize; i < mSize; ++i)
				mElements[i].~T();

			mSize = inNewSize;
		}

		void Empty()
		{
			for (u32 i = 0; i < mSize; ++i)
				mElements[i].~T();

			mSize = 0;
		}

		void Reset(u32 inExpectedCapacity = 0, bool inAllowShrinking = true)
		{
			Empty();
			Reserve(inExpectedCapacity, inAllowShrinking);
		}

		T* Find(T const& inElement)
		{
			for (T& element : *this)
				if (element == inElement)
					return &element;

			return nullptr;
		}

		T const* Find(T const& inElement) const
		{
			for (T const& element : *this)
				if (element == inElement)
					return &element;

			return nullptr;
		}

		T* Find(std::function<bool(T const&)>&& inPredicate)
		{
			for (T& element : *this)
				if (inPredicate(element))
					return &element;

			return nullptr;
		}

		T const* Find(std::function<bool(T const&)>&& inPredicate) const
		{
			for (T const& element : *this)
				if (inPredicate(element))
					return &element;

			return nullptr;
		}

		bool Contains(T const& inElement) const
		{
			return Find(inElement) != nullptr;
		}

		bool Contains(std::function<bool(T const&)>&& inPredicate) const
		{
			return Find(std::move(inPredicate)) != nullptr;
		}

		T* begin() const { return mElements; }
		T* end() const { return mElements + mSize; }

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
				T* newElements = (T*)_aligned_malloc(mCapacity * sizeof(T), alignof(T));
				mage_check(newElements);
				memcpy(newElements, mElements, mSize * sizeof(T));
				_aligned_free(mElements);
				mElements = newElements;
			}
			else
			{
				_aligned_free(mElements);
				mElements = (T*)_aligned_malloc(mCapacity * sizeof(T), alignof(T));
				mage_check(mElements);
			}
		}

		void InitFrom_Copy(const T* inFirst, u32 inSize)
		{
			Empty();
			Reserve(inSize);

			for (u32 i = 0; i < inSize; ++i)
				AddChecked(inFirst[i]);
		}

		void InitFrom_Move(T* inFirst, u32 inSize)
		{
			Empty();
			Reserve(inSize);

			for (u32 i = 0; i < inSize; ++i)
				AddChecked(std::move(inFirst[i]));
		}

		u32 AddChecked(T const& inElement)
		{
			new (mElements + mSize) T(inElement);
			return mSize++;
		}

		u32 AddChecked(T&& inElement)
		{
			new (mElements + mSize) T(std::move(inElement));
			return mSize++;
		}

		T* mElements = nullptr;
		u32 mCapacity = 0;
		u32 mSize = 0;

		static constexpr u32 cMinCapacity = u32(7);
	};
}
