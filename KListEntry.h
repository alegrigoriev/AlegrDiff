// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#if !defined(AFX_KLISTENTRY_H__B7AA7401_4036_11D4_9ADD_00C0F0583C4B__INCLUDED_)
#define AFX_KLISTENTRY_H__B7AA7401_4036_11D4_9ADD_00C0F0583C4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SimpleCriticalSection.h"

#pragma warning(disable:4355)

template<class T> struct ListItem;

template<class T>
struct ListEntry
{
protected:
	typedef ListEntry<T> List;
	typedef ListItem<T> Item;

public:
	List * pPrev;
	List * pNext;
	ListEntry()
	{
		pPrev = this;
		pNext = this;
	}

	~ListEntry()
	{
		// make sure the item is not in any list
		ASSERT(pPrev == this && pNext == this);
	}
	void Init()
	{
		pPrev = this;
		pNext = this;
	}

private:
	// protection against assignment:
	ListEntry(ListEntry<T> const &);
	ListEntry<T> & operator =(ListEntry<T> const &);
};

template<class T>
struct ListItem : ListEntry<T>
{
	//typedef ListItem<T> Item;
	T * Next() const volatile { return static_cast<T *>(pNext); }
	T * Prev() const volatile { return static_cast<T *>(pPrev); }
	bool IsAlone() const volatile
	{
		return pPrev == static_cast<List const volatile *>(this);
	}
	void RemoveFromList()
	{
		pNext->pPrev = pPrev;
		pPrev->pNext = pNext;
		Init();
	}
	void InsertNextItem(Item * entry)
	{
		__assume(NULL != entry);
		// make sure the item is not in any list
		ASSERT(entry->IsAlone());

		pNext->pPrev = entry;
		entry->pNext = pNext;
		entry->pPrev = this;
		pNext = entry;
	}

	void InsertPrevItem(Item * entry)
	{
		__assume(NULL != entry);
		// make sure the item is not in any list
		ASSERT(entry->IsAlone());

		pPrev->pNext = entry;
		entry->pPrev = pPrev;
		entry->pNext = this;
		pPrev = entry;
	}
};

template<class T>
struct ListHead : ListEntry<T>
{
	//typename Item;
	typedef ListHead<T> Head;

	T * First() const volatile { return static_cast<T *>(pNext); }
	T * Last() const volatile { return static_cast<T *>(pPrev); }
	Item * FirstItem() const volatile { return static_cast<Item *>(pNext); }
	Item * LastItem() const volatile { return static_cast<Item *>(pPrev); }
	static T * Next(const volatile Item * pItem) { return pItem->Next(); }
	static T * Prev(const volatile Item * pItem) { return pItem->Prev(); }

	bool NotEnd(Item const volatile * entry) const volatile
	{
		return this != static_cast<List const volatile *>(entry);
	}
	bool IsEnd(Item const volatile * entry) const volatile
	{
		return this == static_cast<List const volatile *>(entry);
	}

	bool IsEmpty() const volatile
	{
		return pNext == static_cast<List const volatile *>(this);
	}

	static void RemoveEntry(Item * entry)
	{
		__assume(NULL != entry);
		entry->RemoveFromList();
	}

	T * RemoveHead()
	{
		if (IsEmpty())
		{
			return NULL;
		}
		Item * tmp = FirstItem();
		tmp->RemoveFromList();

		return static_cast<T *> (tmp);
	}
	T * RemoveTail()
	{
		if (IsEmpty())
		{
			return NULL;
		}
		Item * tmp = LastItem();
		tmp->RemoveFromList();

		return static_cast<T *>(tmp);
	}

	void InsertHead(Item * entry)
	{
		// make sure the item is not in any list
		ASSERT(entry->IsAlone());

		pNext->pPrev = entry;
		entry->pNext = pNext;
		entry->pPrev = this;
		pNext = entry;
	}

	void InsertTail(Item * entry)
	{
		// make sure the item is not in any list
		ASSERT(entry->IsAlone());

		pPrev->pNext = entry;
		entry->pPrev = pPrev;
		entry->pNext = this;
		pPrev = entry;
	}
	// move all the list to DstList. The list becomes empty
	void RemoveAll(Head & DstList)
	{
		ASSERT(DstList.IsEmpty());

		if ( ! IsEmpty())
		{
			DstList.pNext = pNext;
			DstList.pPrev = pPrev;
			pNext->pPrev = & DstList;
			pPrev->pNext = & DstList;
			pNext = this;
			pPrev = this;
		}
	}

	template <typename K> T * FindByKey(const K & key)
	{
		// operator == (const T&, const K&) must exist
		for (T * p = First(); NotEnd(p); p = Next(p))
		{
			if (*p == key)
			{
				return p;
			}
		}
		return NULL;
	}

	// call a function with any return type
	template <typename F> void CallForEach(F function)
	{
		for (T * p = First(); NotEnd(p); )
		{
			// the item can be removed from the list during F
			T * next = Next(p);
			(p->*function)();
			p = next;
		}
	}
	// call a function with any return and argument type
	template <typename F, typename A> void CallForEach(F function, A a)
	{
		for (T * p = First(); NotEnd(p); )
		{
			// the item can be removed from the list during F
			// CANNOT use 'List' typedef with 'p', because of possible ambiguity
			T * next = Next(p);
			(p->*function)(a);
			p = next;
		}
	}
	template <typename F> bool CallForEachWhileTrue(F function)
	{
		for (T * p = First(); NotEnd(p); )
		{
			// the item can be removed from the list during F
			// CANNOT use 'List' typedef with 'p', because of possible ambiguity
			T * next = Next(p);
			if ( ! (p->*function)())
			{
				return false;
			}
			p = next;
		}
		return true;
	}
	template <typename F, typename A> bool CallForEachWhileTrue(F function, A a)
	{
		for (T * p = First(); NotEnd(p); )
		{
			// the item can be removed from the list during F
			T * next = Next(p);
			if ( ! (p->*function)(a))
			{
				return false;
			}
			p = next;
		}
		return true;
	}
	template <typename F> bool CallForEachWhileNotTrue(F function)
	{
		for (T * p = First(); NotEnd(p); )
		{
			// the item can be removed from the list during F
			T * next = Next(p);
			if ((p->*function)())
			{
				return true;
			}
			p = next;
		}
		return false;
	}
	template <typename F, typename A> bool CallForEachWhileNotTrue(F function, A a)
	{
		for (T * p = First(); NotEnd(p); )
		{
			// the item can be removed from the list during F
			T * next = Next(p);
			if ((p->*function)(a))
			{
				return true;
			}
			p = next;
		}
		return false;
	}

};

// the list may use dispatch (thread) lock or more strong interrupt lock
// the lock could also be a mutex
template<class T, class L = CSimpleCriticalSection>
struct LockedListHead : ListHead<T>, public L
{
	// no destructor necessary
	void InsertHead(Item * entry)
	{
		Lock();
		Head::InsertHead(entry);
		Unlock();
	}
	void InsertTail(Item * entry)
	{
		Lock();
		Head::InsertTail(entry);
		Unlock();
	}
	T * RemoveHead()
	{
		Lock();
		T * tmp = Head::RemoveHead();
		Unlock();
		return tmp;
	}

	T * RemoveTail()
	{
		Lock();
		T * tmp = Head::RemoveTail();
		Unlock();
		return tmp;
	}

	void InsertHeadUnsafe(Item * entry)
	{
		Head::InsertHead(entry);
	}
	void InsertTailUnsafe(Item * entry)
	{
		Head::InsertTail(entry);
	}

	T * RemoveHeadUnsafe()
	{
		return Head::RemoveHead();
	}
	T * RemoveTailUnsafe()
	{
		return Head::RemoveTail();
	}
	void RemoveEntry(Item * entry)
	{
		Lock();
		Head::RemoveEntry(entry);
		Unlock();
	}
	void RemoveEntryUnsafe(Item * entry)
	{
		Head::RemoveEntry(entry);
	}

	// move all the list to DstList. The list becomes empty
	void RemoveAll(Head & DstList)
	{
		Lock();
		Head::RemoveAll(DstList);
		Unlock();
	}

};

#endif // AFX_KLISTENTRY_H__B7AA7401_4036_11D4_9ADD_00C0F0583C4B__INCLUDED_
