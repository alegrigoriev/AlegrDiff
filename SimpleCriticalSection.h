#if !defined(AFX_SIMPLECRITICALSECTION_H__B7AA7401_4036_11D4_9ADD_00C0F0583C4B__INCLUDED_)
#define AFX_SIMPLECRITICALSECTION_H__B7AA7401_4036_11D4_9ADD_00C0F0583C4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSimpleCriticalSection
{
	CRITICAL_SECTION m_cs;
public:
	CSimpleCriticalSection() throw()
	{
		InitializeCriticalSection( & m_cs);
	}
	~CSimpleCriticalSection() throw()
	{
		DeleteCriticalSection( & m_cs);
	}
	void Lock() volatile throw()
	{
		EnterCriticalSection(const_cast<CRITICAL_SECTION *>( & m_cs));
	}
	void Unlock() volatile throw()
	{
		LeaveCriticalSection(const_cast<CRITICAL_SECTION *>( & m_cs));
	}
};
class CSimpleCriticalSectionLock
{
	CSimpleCriticalSection volatile & m_cs;
public:
	CSimpleCriticalSectionLock(CSimpleCriticalSection volatile & cs)
		: m_cs(cs)
	{
		cs.Lock();
	}
	~CSimpleCriticalSectionLock()
	{
		m_cs.Unlock();
	}
};

#endif //AFX_SIMPLECRITICALSECTION_H__B7AA7401_4036_11D4_9ADD_00C0F0583C4B__INCLUDED_
