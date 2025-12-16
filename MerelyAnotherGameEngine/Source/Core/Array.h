#pragma once

#include <malloc.h>
#include <memory.h>

namespace mage
{
	template<typename T>
	class Array
	{
	public:
		Array(u32 initialSize = 0)
		{
			Realloc(initialSize);
		}

		~Array()
		{
			Empty();
			_aligned_free(elements);
		}

		T* GetData() const { return elements; }
		u32 GetSize() const { return size; }

		T& operator[](u32 index) { return elements[index]; }
		const T& operator[](u32 index) const { return elements[index]; }

		u32 Add(const T& element)
		{
			if (size == capacity)
				Realloc(size + 1);

			elements[size] = element;
			return size++;
		}

		bool Insert(const T& element, u32 index)
		{
			if (index > size)
				return false;

			if (size == capacity)
				Realloc(size + 1);

			for (u32 i = size; i > index; --i)
				elements[i] = elements[i - 1];

			elements[index] = element;
			++size;
			return true;
		}

		bool InsertSwap(const T& element, u32 index)
		{
			if (index > size)
				return false;

			if (index == size)
			{
				Add(element);
				return true;
			}

			if (size == capacity)
				Realloc(size + 1);

			elements[size] = elements[index];
			elements[index] = element;
			++size;
			return true;
		}

		bool Remove(const T& element)
		{
			for (u32 i = 0; i < size; ++i)
			{
				if (elements[i] == element)
				{
					--size;
					element.~T();

					for (u32 j = i; j < size; ++j)
						elements[j] = elements[j + 1];

					return true;
				}
			}

			return false;
		}

		bool RemoveSwap(const T& element)
		{
			for (u32 i = 0; i < size; ++i)
			{
				if (elements[i] == element)
				{
					--size;
					element.~T();
					elements[i] = elements[size];

					return true;
				}
			}

			return false;

		}

		bool RemoveAt(u32 index)
		{
			if (index >= size)
				return false;

			--size;
			elements[index].~T();

			for (u32 i = index; i < size; ++i)
				elements[i] = elements[i + 1];

			return true;
		}

		bool RemoveAtSwap(u32 index)
		{
			if (index >= size)
				return false;

			--size;
			elements[index].~T();
			elements[index] = elements[size];

			return true;
		}

		void Reserve(u32 newCapacity, bool allowShrinking = true)
		{
			if (newCapacity < capacity && !allowShrinking)
				return;

			Realloc(newCapacity);
		}

		void Empty()
		{
			for (u32 i = 0; i < size; ++i)
				elements[i].~T();

			size = 0;
		}

		void Reset(u32 expectedCapacity = 0, bool allowShrinking = true)
		{
			Empty();
			Reserve(expectedCapacity, allowShrinking);
		}

	private:
		static constexpr u32 minCapacity = u32(15);
		static constexpr u32 maxCapacity = u32(-1);

		T* elements = nullptr;
		u32 size = 0;
		u32 capacity = 0;

		void Realloc(u32 desiredCapacity)
		{
			if (desiredCapacity < size)
				desiredCapacity = size;

			u32 actualCapacity = minCapacity;
			while (actualCapacity < desiredCapacity)
				actualCapacity = actualCapacity << 1 | 1;

			if (elements != nullptr)
			{
				T* newElements = (T*)_aligned_malloc(actualCapacity * sizeof(T), alignof(T));
				memcpy(newElements, elements, size * sizeof(T));
				_aligned_free(elements);
				elements = newElements;
			}
			else
			{
				elements = (T*)_aligned_malloc(actualCapacity * sizeof(T), alignof(T));
			}
		}
	};
}
