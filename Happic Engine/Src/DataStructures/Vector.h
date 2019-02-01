#pragma once

#include "Types.h"
#include <cstdlib>

namespace Happic {

	template<typename T>
	class Vector
	{
	public:
		inline Vector(uint32 initialCapacity = 0, uint32 sizeIncrease = 10) :
			m_sizeIncrease(sizeIncrease),
			m_capacity(initialCapacity),
			m_numElements(initialCapacity),
			m_pMemory(NULL)

		inline Vector(const Vector<T>& vec)
		{
			m_capacity = vec.m_capacity;
			m_numElements = vec.m_numElements;
			m_pMemory = (uint8*)malloc(m_capacity * sizeof(T));
			memcpy(m_pMemory, vec.m_pMemory, vec.m_capacity * sizeof(T));
			m_sizeIncrease = vec.m_sizeIncrease;
		}

		~Vector()
		{
			free(m_pMemory);
		}

		inline uint32 Size() const { return m_numElements; }

		bool IsEmpty() const { return m_numElements == 0; }

		inline void Push(const T& obj)
		{
			if (m_numElements == m_capacity)
				Grow();

			memcpy(m_pMemory + m_numElements * sizeof(T), &obj, sizeof(T));
			m_numElements++;
		}

		inline void Pop()
		{
			
		}

		inline void Insert(const T& obj, uint32 index)
		{

		}

		inline void Remove(uint32 index)
		{

		}

		const T* begin() const { return (T*)m_pMemory; }
		T* begin() { return (T*)m_pMemory; }

		const T* end() const { return (T*) ( m_pMemory + m_capacity * sizeof(T) ); }
		T* end() { return (T*) (m_pMemory + m_capacity * sizeof(T) ); }

		inline const T& operator[](uint32 index) const { return *(T*)(m_pMemory + index * sizeof(T)); }
		inline T& operator[](uint32 index) { return *(T*)(m_pMemory + index * sizeof(T)); }
	private:
		inline void Grow()
		{
			m_capacity += m_sizeIncrease;
			uint8* pNewMemory = (uint8*)malloc(m_capacity * sizeof(T));
			memcpy(pNewMemory, m_pMemory, (m_capacity - m_sizeIncrease) * sizeof(T));
			if(m_pMemory)
				free(m_pMemory);
			m_pMemory = pNewMemory;
		}
	private:
		uint8* m_pMemory;
		uint32 m_numElements;
		uint32 m_capacity;
		uint32 m_sizeIncrease;
	};

}
