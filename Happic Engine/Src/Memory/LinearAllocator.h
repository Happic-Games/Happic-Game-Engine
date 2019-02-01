#pragma once

#include "Allocator.h"

namespace Happic {

	class LinearAllocator : public Allocator
	{
	public:
		LinearAllocator(uint32 size);
		LinearAllocator(void* pData, uint32 size);

		~LinearAllocator();

		void* Allocate(uint32 size) override;
		void Free(void* pData) override;
		void Reset() override;

		void* GetData() const override;
		uint32 GetSize() const override;
	private:
		void* m_pData;
		uint32 m_offset;
		uint32 m_size;
	};

}
