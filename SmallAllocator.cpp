#include "StdAfx.h"
#include "SmallAllocator.h"

static void * operator new(size_t, void * ptr)
{
	return ptr;
}

static void operator delete(void * ptr, void *)
{
	//::delete(ptr);
}

CSmallAllocator::CSmallAllocator(size_t ItemSize, size_t BlockSize)
	: m_ItemSize(ItemSize),
	m_Blocks(NULL), m_BlocksWithFreeItems(NULL),
	m_BlockSize(BlockSize)
{
	size_t RoundedItemSize = (ItemSize + 3) & ~3;
	m_ItemsInBlock = (BlockSize - sizeof BlockHeader) / (RoundedItemSize + sizeof ItemHeader);
}
CSmallAllocator::~CSmallAllocator()
{
	// check that all the blocks are freed
	if (m_Blocks.pNext != & m_Blocks
		|| m_BlocksWithFreeItems.pNext != & m_BlocksWithFreeItems)
	{
		TRACE("CSmallAllocator::~CSmallAllocator ItemSize= %d, Some blocks are not freed\n",
			m_ItemSize);
	}
}

CSmallAllocator::BlockHeader * CSmallAllocator::AllocateBlock()
{
	char * pBuf = new char[m_BlockSize];
	if (NULL == pBuf)
	{
		return NULL;
	}
	BlockHeader * pBlock = new (pBuf) BlockHeader(this);
	ItemHeader * pItem = reinterpret_cast<ItemHeader *>(pBuf + sizeof BlockHeader);
	for (unsigned i = 0; i < m_ItemsInBlock; i++)
	{
		ItemHeader * pItem = reinterpret_cast<ItemHeader *>(
								pBuf + sizeof BlockHeader + i * (sizeof ItemHeader + ((m_ItemSize + 3) & ~3)));
		pItem->pNext = pBlock->pFreeItems;
		pBlock->pFreeItems = pItem;
	}
	pBlock->NumOfFreeItems = m_ItemsInBlock;
	TRACE("CSmallAllocator::AllocateBlock: %X allocated\n", pBlock);
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

	BlockHeader * pBlock = m_BlocksWithFreeItems.pNext;
	if (pBlock == & m_BlocksWithFreeItems)
	{
		// allocate another block
		pBlock = AllocateBlock();
		if (NULL == pBlock)
		{
			return NULL;
		}
		pBlock->pNext = m_BlocksWithFreeItems.pNext;
		m_BlocksWithFreeItems.pNext = pBlock;
		pBlock->pNext->pPrev = pBlock;
		pBlock->pPrev = & m_BlocksWithFreeItems;
	}
	ItemHeader * pItem = pBlock->pFreeItems;
	pBlock->pFreeItems = pItem->pNext;
	pItem->pContainingBlock = pBlock;
	pBlock->NumOfFreeItems--;
	if (0 == pBlock->NumOfFreeItems)
	{
		// remove from m_BlocksWithFreeItems
		pBlock->pNext->pPrev = pBlock->pPrev;
		pBlock->pPrev->pNext = pBlock->pNext;
		// put to m_Blocks
		pBlock->pNext = m_Blocks.pNext;
		m_Blocks.pNext = pBlock;
		pBlock->pNext->pPrev = pBlock;
		pBlock->pPrev = & m_Blocks;
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

	pBlock->NumOfFreeItems++;
	if (1 == pBlock->NumOfFreeItems)
	{
		// remove from m_Blocks
		pBlock->pNext->pPrev = pBlock->pPrev;
		pBlock->pPrev->pNext = pBlock->pNext;
		// put to m_BlocksWithFreeItems
		pBlock->pNext = pAlloc->m_BlocksWithFreeItems.pNext;
		pAlloc->m_BlocksWithFreeItems.pNext = pBlock;
		pBlock->pNext->pPrev = pBlock;
		pBlock->pPrev = & pAlloc->m_BlocksWithFreeItems;
	}
	if (pAlloc->m_ItemsInBlock == pBlock->NumOfFreeItems)
	{
		// remove from m_Blocks
		pBlock->pNext->pPrev = pBlock->pPrev;
		pBlock->pPrev->pNext = pBlock->pNext;
		TRACE("CSmallAllocator::Free Block %X freed\n", pBlock);
		::delete[] (char *) pBlock;
	}
}

