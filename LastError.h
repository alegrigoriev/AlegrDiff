#pragma once

class CLastError
{
public:

	CLastError(void)
		: LastError(-1)
	{
	}
	DWORD Get()
	{
		return LastError = ::GetLastError();
	}
	void Set(DWORD error)
	{
		LastError = error;
	}
	operator DWORD() const
	{
		return LastError;
	}
	// restore last error
	~CLastError(void)
	{
		if (-1 != LastError)
		{
			::SetLastError(LastError);
		}
	}
private:
	DWORD LastError;
};
