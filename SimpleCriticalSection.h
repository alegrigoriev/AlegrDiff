#pragma once

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

