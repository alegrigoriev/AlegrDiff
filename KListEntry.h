// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#if !defined(AFX_KLISTENTRY_H__B7AA7401_4036_11D4_9ADD_00C0F0583C4B__INCLUDED_)
#define AFX_KLISTENTRY_H__B7AA7401_4036_11D4_9ADD_00C0F0583C4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SimpleCriticalSection.h"

template<class T>
struct KListEntry
{
	KListEntry<T> * pPrev;
	KListEntry<T> * pNext;
	// no destructor necessary
	void Init() volatile
	{
		pPrev = const_cast<KListEntry<T> *>(this);
		pNext = pPrev;
	}
	KListEntry()
	{
		Init();
	}
	~KListEntry()
	{
		ASSERT(IsEmpty());
	}
	void InsertHead(T * entry)
	{
		__assume(NULL != entry);
		KListEntry<T> * tmp = static_cast<KListEntry<T> *>(entry);
		ASSERT(tmp->pNext == tmp);
		ASSERT(tmp->pPrev == tmp);
		tmp->pPrev = this;
		tmp->pNext = pNext;
		pNext->pPrev = tmp;
		pNext = tmp;
	}
	void InsertHead(T volatile * entry) volatile
	{
		__assume(NULL != entry);
		KListEntry<T> volatile * tmp = static_cast<KListEntry<T> volatile *>(entry);
		ASSERT(tmp->pNext == tmp);
		ASSERT(tmp->pPrev == tmp);
		tmp->pPrev = const_cast<KListEntry<T> *>(this);
		tmp->pNext = pNext;
		pNext->pPrev = const_cast<KListEntry<T> *>(tmp);
		pNext = const_cast<KListEntry<T> *>(tmp);
	}
	void InsertTail(T * entry)
	{
		__assume(NULL != entry);
		KListEntry<T> * tmp = static_cast<KListEntry<T> *>(entry);
		ASSERT(tmp->pNext == tmp);
		ASSERT(tmp->pPrev == tmp);
		tmp->pNext = this;
		tmp->pPrev = pPrev;
		pPrev->pNext = tmp;
		pPrev = tmp;
	}
	void InsertTail(T volatile * entry) volatile
	{
		__assume(NULL != entry);
		KListEntry<T> volatile * tmp = static_cast<KListEntry<T> volatile *>(entry);
		ASSERT(tmp->pNext == tmp);
		ASSERT(tmp->pPrev == tmp);
		tmp->pNext = const_cast<KListEntry<T> *>(this);
		tmp->pPrev = pPrev;
		pPrev->pNext = const_cast<KListEntry<T> *>(tmp);
		pPrev = const_cast<KListEntry<T> *>(tmp);
	}

	bool IsEmpty() const volatile
	{
		ASSERT((pNext == this) == (pPrev == this));
		return pNext == this;
	}
	T * RemoveHead()
	{
		KListEntry<T> * tmp = pNext;
		tmp->pNext->pPrev = this;
		pNext = tmp->pNext;
		tmp->Init();
		__assume(NULL != tmp);
		return static_cast<T *>(tmp);
	}
	T * RemoveHead() volatile
	{
		KListEntry<T> * tmp = pNext;
		tmp->pNext->pPrev = const_cast<KListEntry<T> *>(this);
		pNext = tmp->pNext;
		tmp->Init();
		__assume(NULL != tmp);
		return static_cast<T *>(tmp);
	}
	T * RemoveTail()
	{
		KListEntry<T> * tmp = pPrev;
		tmp->pPrev->pNext = this;
		pPrev = tmp->pPrev;
		tmp->Init();
		__assume(NULL != tmp);
		return static_cast<T *>(tmp);
	}
	T * RemoveTail() volatile
	{
		KListEntry<T> * tmp = pPrev;
		tmp->pPrev->pNext = const_cast<KListEntry<T> *>(this);
		pPrev = tmp->pPrev;
		tmp->Init();
		__assume(NULL != tmp);
		return static_cast<T *>(tmp);
	}
	void RemoveFromList()
	{
		pNext->pPrev = pPrev;
		pPrev->pNext = pNext;
		Init();
	}
	void RemoveFromList() volatile
	{
		pNext->pPrev = pPrev;
		pPrev->pNext = pNext;
		Init();
	}
	static void RemoveEntry(T * entry)
	{
		__assume(NULL != entry);
		KListEntry<T> * tmp = static_cast<KListEntry<T> *>(entry);
		tmp->pNext->pPrev = tmp->pPrev;
		tmp->pPrev->pNext = tmp->pNext;
		tmp->Init();
	}

	static void RemoveEntry(T volatile * entry)
	{
		__assume(NULL != entry);
		KListEntry<T> volatile * tmp = static_cast<KListEntry<T> volatile *>(entry);
		tmp->pNext->pPrev = tmp->pPrev;
		tmp->pPrev->pNext = tmp->pNext;
		tmp->Init();
	}

	// move all the list to DstList. The list becomes empty
	void RemoveAll(KListEntry<T> & DstList)
	{
		if ( ! IsEmpty())
		{
			KListEntry<T> * pListEntry = pNext;
			RemoveFromList();
			Init();
			pListEntry->InsertTailList( & DstList);
		}
		else
		{
			DstList.Init();
		}
	}

	void RemoveAll(KListEntry<T> & DstList) volatile
	{
		if ( ! IsEmpty())
		{
			KListEntry<T> * pListEntry = pNext;
			RemoveFromList();
			Init();
			pListEntry->InsertTailList( & DstList);
		}
		else
		{
			DstList.Init();
		}
	}

	T * Next() const volatile
	{
		__assume(NULL != pNext);
		return static_cast<T *>(pNext);
	}
	T * Prev() const volatile
	{
		__assume(NULL != pPrev);
		return static_cast<T *>(pPrev);
	}

	// same as Next, Prev
	T * First() const volatile
	{
		__assume(NULL != pNext);
		return static_cast<T *>(pNext);
	}

	T * Last() const volatile
	{
		__assume(NULL != pPrev);
		return static_cast<T *>(pPrev);
	}

	T * Head()
	{
		return static_cast<T *>(this);
	}
	T const * Head() const
	{
		return static_cast<T const *>(this);
	}
	T volatile * Head() volatile
	{
		return static_cast<T volatile *>(this);
	}
	T const volatile * Head() const volatile
	{
		return static_cast<T const volatile *>(this);
	}

	bool NotEnd(KListEntry<T> const volatile * entry) const volatile
	{
		return this != entry;
	}
	bool IsEnd(KListEntry<T> const volatile * entry) const volatile
	{
		return this == entry;
	}
	// call a function with any return type
	template <class F> void CallForEach(F function) const volatile
	{
		for (T * p = Next(); p != this; p = p->KListEntry<T>::Next())
		{
			(p->*function)();
		}
	}
	// call a function with any return and argument type
	template <class F, class A> void CallForEach(F function, A a) const volatile
	{
		for (T * p = Next(); p != this; p = p->KListEntry<T>::Next())
		{
			(p->*function)(a);
		}
	}
	template <class F> BOOL CallForEachWhileNotSuccess(F function) const volatile
	{
		for (T * p = Next(); p != this; p = p->KListEntry<T>::Next())
		{
			BOOL status = (p->*function)();
			if (status)
			{
				return status;
			}
		}
		return FALSE;
	}
	template <class F, class A> BOOL CallForEachWhileNotSuccess(F function, A a) const volatile
	{
		for (T * p = Next(); p != this; p = p->KListEntry<T>::Next())
		{
			BOOL status = (p->*function)(a);
			if (status)
			{
				return status;
			}
		}
		return FALSE;
	}
	template <class F> BOOL CallForEachWhileSuccess(F function) const volatile
	{
		for (T * p = Next(); p != this; p = p->KListEntry<T>::Next())
		{
			BOOL status = (p->*function)();
			if ( ! status)
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	template <class F, class A> BOOL CallForEachWhileSuccess(F function, A a) const volatile
	{
		for (T * p = Next(); p != this; p = p->KListEntry<T>::Next())
		{
			BOOL status = (p->*function)(a);
			if ( ! status)
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	template <class F> bool CallForEachWhileTrue(F function) const volatile
	{
		for (T * p = Next(); p != this; p = p->KListEntry<T>::Next())
		{
			if ( ! (p->*function)())
			{
				return false;
			}
		}
		return true;
	}
	template <class F, class A> bool CallForEachWhileTrue(F function, A a) const volatile
	{
		for (T * p = Next(); p != this; p = p->KListEntry<T>::Next())
		{
			if ( ! (p->*function)(a))
			{
				return false;
			}
		}
		return true;
	}
	template <class F> bool CallForEachWhileNotTrue(F function) const volatile
	{
		for (T * p = Next(); p != this; p = p->KListEntry<T>::Next())
		{
			if ((p->*function)())
			{
				return true;
			}
		}
		return false;
	}
	template <class F, class A> bool CallForEachWhileNotTrue(F function, A a) const volatile
	{
		for (T * p = Next(); p != this; p = p->KListEntry<T>::Next())
		{
			if ((p->*function)(a))
			{
				return true;
			}
		}
		return false;
	}

};

template<class T >
struct LockedListHead: public KListEntry<T>, public CSimpleCriticalSection
{
	void InsertHead(T * entry)
	{
		Lock();
		KListEntry::InsertHead(entry);
		Unlock();
	}
	void InsertHead(T * entry) volatile
	{
		Lock();
		KListEntry<T>::InsertHead(entry);
		Unlock();
	}
	void InsertHeadUnsafe(T * entry)
	{
		KListEntry<T>::InsertHead(entry);
	}
	void InsertHeadUnsafe(T * entry) volatile
	{
		KListEntry<T>::InsertHead(entry);
	}
	void InsertTail(T * entry)
	{
		Lock();
		KListEntry<T>::InsertTail(entry);
		Unlock();
	}
	void InsertTail(T * entry) volatile
	{
		Lock();
		KListEntry<T>::InsertTail(entry);
		Unlock();
	}
	void InsertTailUnsafe(T * entry)
	{
		KListEntry<T>::InsertTail(entry);
	}
	void InsertTailUnsafe(T * entry) volatile
	{
		KListEntry<T>::InsertTail(entry);
	}
	T * RemoveHead()
	{
		CSimpleCriticalSectionLock lock(*this);
		if (IsEmpty())
		{
			return NULL;
		}
		return KListEntry<T>::RemoveHead();
	}
	T * RemoveHead() volatile
	{
		CSimpleCriticalSectionLock lock(*this);
		if (IsEmpty())
		{
			return NULL;
		}
		return KListEntry<T>::RemoveHead();
	}
	T * RemoveHeadUnsafe()
	{
		return KListEntry<T>::RemoveHead();
	}
	T * RemoveHeadUnsafe() volatile
	{
		return KListEntry<T>::RemoveHead();
	}
	void RemoveEntry(T * Entry)
	{
		Lock();
		KListEntry<T>::RemoveEntry(Entry);
		Unlock();
	}
	void RemoveEntry(T * Entry) volatile
	{
		Lock();
		KListEntry<T>::RemoveEntry(Entry);
		Unlock();
	}
	static void RemoveEntryUnsafe(T * entry)
	{
		KListEntry<T>::RemoveEntry(Entry);
	}

	// move all the list to DstList. The list becomes empty
	void RemoveAll(KListEntry<T> & DstList)
	{
		Lock();
		KListEntry<T>::RemoveAll(DstList);
		Unlock();
	}
	void RemoveAll(KListEntry<T> & DstList) volatile
	{
		Lock();
		KListEntry<T>::RemoveAll(DstList);
		Unlock();
	}
};

#endif // AFX_KLISTENTRY_H__B7AA7401_4036_11D4_9ADD_00C0F0583C4B__INCLUDED_
