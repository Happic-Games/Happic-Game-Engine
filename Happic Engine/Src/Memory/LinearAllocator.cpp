#include "LinearAllocator.h"

namespace Happic {

	LinearAllocator::LinearAllocator(uint32 size) :
		m_size(size),
		m_offset(0)
	{
		m_pData = malloc(size);
	}

	LinearAllocator::LinearAllocator(void * pData, uint32 size):
		m_size(size),
		m_offset(0)
	{
		m_pData = pData;
	}

	LinearAllocator::~LinearAllocator()
	{
		free(m_pData);
	}

	void * LinearAllocator::Allocate(uint32 size)
	{
		if (m_offset + size > m_size)
			return NULL;

		void* pAllocation = (uint8*)m_pData + m_offset;
		m_offset += size;
		return pAllocation;
	}

	void LinearAllocator::Free(void * pData)
	{
		//Can't free memory
	}

	void LinearAllocator::Reset()
	{
		m_offset = 0;
	}

	void * LinearAllocator::GetData() const
	{
		return m_pData;
	}

	uint32 LinearAllocator::GetSize() const
	{
		return m_size;
	}


}