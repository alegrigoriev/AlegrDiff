#pragma once

class CLastError
{
public:

	CLastError(void)
		: LastError(~0UL)
	{
	}
	DWORD Get()
	{
		return LastError = ::GetLastError();
	}
	void Set(DWORD error)
	{
		SetLastError(error);
		LastError = error;
	}
	operator DWORD() const
	{
		return LastError;
	}
	// restore last error
	~CLastError(void)
	{
		if (~0 != LastError)
		{
			::SetLastError(LastError);
		}
	}
private:
	DWORD LastError;
};
