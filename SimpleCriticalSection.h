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
	void Lock() throw()
	{
		EnterCriticalSection( & m_cs);
	}
	void Unlock() throw()
	{
		LeaveCriticalSection( & m_cs);
	}
};
class CSimpleCriticalSectionLock
{
	CSimpleCriticalSection & m_cs;
public:
	CSimpleCriticalSectionLock(CSimpleCriticalSection & cs)
		: m_cs(cs)
	{
		cs.Lock();
	}
	~CSimpleCriticalSectionLock()
	{
		m_cs.Unlock();
	}
};

