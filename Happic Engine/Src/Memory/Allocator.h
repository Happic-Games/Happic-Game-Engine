#pragma once

#include "../DataStructures/Types.h"
#include <cstdlib>

#define MEMORY_1KB 1000
#define MEMORY_HALF_KB MEMORY_1KB / 2

#define MEMORY_1MB 1000000
#define MEMORY_HALF_MB MEMORY_1MB / 2

#define MEMORY_1GB 1000000000
#define MEMORY_HALF_GB MEMORY_1GB / 2

namespace Happic {

	class Allocator
	{
	public:
		virtual void* Allocate(uint32 size) = 0;
		virtual void Free(void* pData) = 0;
		virtual void Reset() = 0;

		virtual void* GetData() const = 0;
		virtual uint32 GetSize() const = 0;
	};

}
