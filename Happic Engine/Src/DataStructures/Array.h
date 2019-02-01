#pragma once

#include "Types.h"
#include <cstring>
#include <cstdlib>

namespace Happic {

	template<typename T, uint32 D>
	class Array
	{
	public:
		inline Array() {}
		inline Array(const T* pElements) { memcpy(m_elements, pElements, sizeof(T) * D); }
		inline ~Array() {}

		inline const T& operator[](uint32 index) const { return m_elements[index]; }
		inline T& operator[](uint32 index) { return m_elements[index]; }
	private:
		T m_elements[D];
	};
}
