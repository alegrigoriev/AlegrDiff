#pragma once
#include "SimpleCriticalSection.h"
// this class allocates small items of fixed size
class CSmallAllocator
{
public:
	CSmallAllocator(size_t ItemSize, size_t BlockSize = 4096);
	~CSmallAllocator();

	void * Allocate(size_t size);
	static void Free(void * ptr);
private:
	struct BlockHeader;
	union ItemHeader
	{
		ItemHeader * pNext;
		BlockHeader * pContainingBlock;
	};
	struct BlockHeader
	{
		BlockHeader(CSmallAllocator * alloc)
			: m_Signature(eSignature),
			pAllocator(alloc),
			pNext(this),
			pPrev(this),
			pFreeItems(NULL),
			NumOfFreeItems(0)
		{
		}
		ULONG m_Signature;
		enum { eSignature = 'SmAl' };
		BlockHeader * pNext;
		BlockHeader * pPrev;
		ItemHeader * pFreeItems;
		CSmallAllocator * pAllocator;
		size_t NumOfFreeItems;
	};
	BlockHeader * AllocateBlock();
	BlockHeader m_Blocks;
	BlockHeader m_BlocksWithFreeItems;
	CSimpleCriticalSection m_cs;
	size_t m_ItemSize;  // without header
	size_t m_BlockSize;
	size_t m_ItemsInBlock;
};

