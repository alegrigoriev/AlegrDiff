#include "StdAfx.h"
#include "SmallAllocator.h"

#ifndef __PLACEMENT_NEW_INLINE
static void * operator new(size_t, void * ptr)
{
	return ptr;
}

static void operator delete(void * ptr, void *)
{
}
#endif

CSmallAllocator::CSmallAllocator(size_t ItemSize, size_t BlockSize)
	: m_ItemSize(ItemSize)
	, m_TotalFreeItems(0)
	, m_BlockSize(BlockSize)
{
	size_t RoundedItemSize = (ItemSize + 3) & ~3;
	m_ItemsInBlock = (BlockSize - sizeof BlockHeader) / (RoundedItemSize + sizeof ItemHeader);
}

CSmallAllocator::~CSmallAllocator()
{
	// check that all the blocks are freed
	CSimpleCriticalSectionLock lock(m_cs);

	while ( ! m_BlocksWithFreeItems.IsEmpty())
	{
		BlockHeader * pBlock = m_BlocksWithFreeItems.RemoveTail();
		if (pBlock->NumOfFreeItems < m_ItemsInBlock)
		{
			TRACE("CSmallAllocator::~CSmallAllocator ItemSize= %d, %d blocks are not freed\n",
				m_ItemSize, m_ItemsInBlock - pBlock->NumOfFreeItems);
		}
		else
		{
			pBlock->~BlockHeader();
			::delete[] (char*) pBlock;
		}
	}

	while ( ! m_Blocks.IsEmpty())
	{
		//BlockHeader * pBlock =
		m_Blocks.RemoveTail();
		TRACE("CSmallAllocator::~CSmallAllocator ItemSize= %d, %d blocks are not freed\n",
			m_ItemSize, m_ItemsInBlock);
	}
}

CSmallAllocator::BlockHeader * CSmallAllocator::AllocateBlock()
{
	char * pBuf = ::new char[m_BlockSize];
	if (NULL == pBuf)
	{
		return NULL;
	}
	BlockHeader * pBlock = new (pBuf) BlockHeader(this);

	//ItemHeader * pItem = reinterpret_cast<ItemHeader *>(pBuf + sizeof BlockHeader);
	for (unsigned i = 0; i < m_ItemsInBlock; i++)
	{
		ItemHeader * pItem = reinterpret_cast<ItemHeader *>(
								pBuf + sizeof BlockHeader + i * (sizeof ItemHeader + ((m_ItemSize + 3) & ~3)));
		pItem->pNext = pBlock->pFreeItems;
		pBlock->pFreeItems = pItem;
	}
	pBlock->NumOfFreeItems = m_ItemsInBlock;
	if (0) TRACE("CSmallAllocator::AllocateBlock: %X allocated\n", pBlock);
	return pBlock;
}

void * CSmallAllocator::Allocate(size_t size)
{
	if (size != m_ItemSize)
	{
		TRACE("Allocating item of size %d, m_ItemSize=%d\n",
			size, m_ItemSize);
		return NULL;
	}
	CSimpleCriticalSectionLock lock(m_cs);

	BlockHeader * pBlock;
	if (m_BlocksWithFreeItems.IsEmpty())
	{
		// allocate another block
		pBlock = AllocateBlock();
		if (NULL == pBlock)
		{
			return NULL;
		}
		m_BlocksWithFreeItems.InsertHead(pBlock);
		m_TotalFreeItems += m_ItemsInBlock;
	}
	else
	{
		pBlock = m_BlocksWithFreeItems.First();
	}

	ItemHeader * pItem = pBlock->pFreeItems;
	pBlock->pFreeItems = pItem->pNext;
	pItem->pContainingBlock = pBlock;

	pBlock->NumOfFreeItems--;
	m_TotalFreeItems--;

	if (0 == pBlock->NumOfFreeItems)
	{
		pBlock->RemoveFromList();
		m_Blocks.InsertTail(pBlock);
	}
	return pItem + 1;
}

void CSmallAllocator::Free(void * ptr)
{
	if (NULL == ptr)
	{
		return;
	}
	ItemHeader * pItem = static_cast<ItemHeader *>(ptr) - 1;

	BlockHeader * pBlock = pItem->pContainingBlock;
	if (pBlock->m_Signature != pBlock->eSignature)
	{
		TRACE("Wrong item %X freed, block adress=%X, signature = %X\n",
			pItem, pBlock, pBlock->m_Signature);
		return;
	}
	CSmallAllocator * pAlloc = pBlock->pAllocator;

	CSimpleCriticalSectionLock lock(pAlloc->m_cs);

	pItem->pNext = pBlock->pFreeItems;
	pBlock->pFreeItems = pItem;

	pAlloc->m_TotalFreeItems++;
	pBlock->NumOfFreeItems++;

	if (1 == pBlock->NumOfFreeItems)
	{
		// remove from m_Blocks
		pBlock->RemoveFromList();
		// put to m_BlocksWithFreeItems
		pAlloc->m_BlocksWithFreeItems.InsertTail(pBlock);
	}

	// leave the block for the future allocations
	if (pAlloc->m_ItemsInBlock == pBlock->NumOfFreeItems
		&& pAlloc->m_TotalFreeItems >= pAlloc->m_ItemsInBlock * 2)
	{
		// remove from m_Blocks
		pBlock->RemoveFromList();
		pAlloc->m_TotalFreeItems -= pAlloc->m_ItemsInBlock;
		if (0) TRACE("CSmallAllocator::Free Block %X freed\n", pBlock);

		pBlock->~BlockHeader();
		::delete[] (char*) pBlock;
	}
}

