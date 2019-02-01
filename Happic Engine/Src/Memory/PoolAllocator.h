#pragma once

#include "Allocator.h"

namespace Happic {

	class PoolAllocator : public Allocator
	{
	public:
		PoolAllocator(uint32 numElements, uint32 elementSize);
		PoolAllocator(void* pData, uint32 numElements, uint32 elementSize);
		PoolAllocator();

		void* Allocate(uint32 size = 0) override;
		void Free(void* pData) override;
		void Reset() override;

		void* GetData() const override;
		uint32 GetSize() const override;
	private:
		void InitPoolElementDataStructure();
	private:
		struct PoolElement
		{
			PoolElement* pNext;
		};

		PoolElement* m_pElement;
		uint32 m_numElements;
		uint32 m_elementSize;
	};

}
