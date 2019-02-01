#include "StackAllocator.h"

namespace Happic {

	StackAllocator::StackAllocator(uint32 size) :
		m_size(size),
		m_offset(0)
	{
		m_pData = malloc(size);
	}

	StackAllocator::StackAllocator(void * pData, uint32 size) :
		m_size(size),
		m_offset(0),
		m_pData(pData)
	{}

	StackAllocator::StackAllocator() :
		m_size(0),
		m_pData(NULL),
		m_offset(0)
	{}

	StackAllocator::~StackAllocator()
	{
		free(m_pData);
	}

	void * Happic::StackAllocator::Allocate(uint32 size)
	{
		uint8* pAllocation = (uint8*)m_pData + m_offset;
		*(stack_offset_data_type*)pAllocation = m_offset;
		pAllocation += sizeof(stack_offset_data_type);
		return pAllocation;
	}

	void StackAllocator::Free(void * pData)
	{
		m_offset = *(stack_offset_data_type*)((uint8*)pData - sizeof(stack_offset_data_type));
	}

	void StackAllocator::Reset()
	{
		m_offset = 0;
	}

	void * StackAllocator::GetData() const
	{
		return m_pData;
	}

	uint32 StackAllocator::GetSize() const
	{
		return m_size;
	}

}
