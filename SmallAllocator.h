#pragma once
#include "SimpleCriticalSection.h"
#include "KlistEntry.h"
// this class allocates small items of fixed size
#pragma warning(disable:4355)
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
	struct BlockHeader : public KListEntry<BlockHeader>
	{
		BlockHeader(CSmallAllocator * alloc)
			: m_Signature(eSignature),
			pAllocator(alloc),
			pFreeItems(NULL),
			NumOfFreeItems(0)
		{
		}
		ULONG m_Signature;
		enum { eSignature = 'SmAl' };
		ItemHeader * pFreeItems;
		CSmallAllocator * pAllocator;
		size_t NumOfFreeItems;
	};
	BlockHeader * AllocateBlock();
	KListEntry<BlockHeader> m_Blocks;
	KListEntry<BlockHeader> m_BlocksWithFreeItems;
	CSimpleCriticalSection m_cs;
	size_t m_ItemSize;  // without header
	size_t m_BlockSize;
	unsigned m_ItemsInBlock;
	unsigned m_TotalFreeItems;
};

#pragma warning(default:4355)

