#include "PoolAllocator.h"
namespace Happic {

	PoolAllocator::PoolAllocator(uint32 numElements, uint32 elementSize) :
		m_numElements(numElements),
		m_elementSize(elementSize)
	{
		m_pElement = (PoolElement*)malloc(numElements * elementSize);
		InitPoolElementDataStructure();
	}

	PoolAllocator::PoolAllocator(void * pData, uint32 numElements, uint32 elementSize) :
		m_numElements(numElements),
		m_elementSize(elementSize)
	{
		m_pElement = (PoolElement*)pData;
		InitPoolElementDataStructure();
	}

	PoolAllocator::PoolAllocator()
	{
		m_pElement = NULL;
		m_numElements = 0;
		m_elementSize = 0;
	}

	void * PoolAllocator::Allocate(uint32 size)
	{
		if (!m_pElement)
			return NULL;

		PoolElement* pHead = m_pElement;
		m_pElement = pHead->pNext;
		return pHead;
	}

	void PoolAllocator::Free(void * pData)
	{
		PoolElement* pHead = (PoolElement*)pData;
		pHead->pNext = m_pElement;
		m_pElement = pHead;
	}

	void PoolAllocator::Reset()
	{

	}

	void * PoolAllocator::GetData() const
	{
		return m_pElement;
	}

	uint32 PoolAllocator::GetSize() const
	{
		return m_numElements * m_elementSize;
	}

	void PoolAllocator::InitPoolElementDataStructure()
	{
		union
		{
			PoolElement* as_self;
			unsigned char* as_char;
			void* as_void;
		};

		as_self = m_pElement;

		PoolElement* pRunner = m_pElement;
		for (uint32 i = 0; i < m_numElements; i++)
		{
			pRunner->pNext = as_self;
			pRunner = as_self;
			as_char += m_elementSize;
		}

		pRunner->pNext = NULL;
	}

}
