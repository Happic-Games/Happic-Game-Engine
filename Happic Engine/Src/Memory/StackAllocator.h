#pragma once

#include "Allocator.h"

namespace Happic {

	typedef uint8 stack_offset_data_type;

	class StackAllocator : public Allocator
	{
	public:
		StackAllocator(uint32 size);
		StackAllocator(void* pData, uint32 size);
		StackAllocator();

		~StackAllocator();

		void* Allocate(uint32 size) override;
		void Free(void* pData) override;
		void Reset();

		void* GetData() const override;
		uint32 GetSize() const override;
	private:
		void*					m_pData;
		stack_offset_data_type	m_offset;
		uint32					m_size;
	};

}
