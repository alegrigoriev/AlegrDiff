// CPathEx Extends CPathT
#include <shlwapi.h>
#include <atlpath.h>
#pragma once

template<typename StringType>
class CPathExT : public ATL::CPathT<StringType>
{
	typedef ATL::CPathT<StringType> BasePath;
	typedef typename BasePath::PCXSTR PCXSTR;

	static DWORD GetFullPathNameT(
								LPCWSTR lpFileName, DWORD nBufferLength,
								LPWSTR lpBuffer, LPWSTR* lpFilePart)
	{
		return GetFullPathNameW(lpFileName, nBufferLength, lpBuffer, lpFilePart);
	}

	static DWORD GetFullPathNameT(
								LPCSTR lpFileName, DWORD nBufferLength,
								LPSTR lpBuffer, LPSTR* lpFilePart)
	{
		return GetFullPathNameA(lpFileName, nBufferLength, lpBuffer, lpFilePart);
	}

	static DWORD GetCurrentDirectoryT(DWORD nBufferLength,
									LPWSTR lpBuffer)
	{
		return ::GetCurrentDirectoryW(nBufferLength, lpBuffer);
	}

	static DWORD GetCurrentDirectoryT(DWORD nBufferLength,
									LPSTR lpBuffer)
	{
		return ::GetCurrentDirectoryA(nBufferLength, lpBuffer);
	}

	static DWORD GetTempPathT(DWORD nBufferLength,
							LPSTR lpBuffer)
	{
		return ::GetTempPathA(nBufferLength, lpBuffer);
	}

	static DWORD GetTempPathT(DWORD nBufferLength,
							LPWSTR lpBuffer)
	{
		return ::GetTempPathW(nBufferLength, lpBuffer);
	}

	static DWORD GetModuleFileNameT(HMODULE hModule, LPSTR lpBuffer,
									DWORD nBufferLength)
	{
		return ::GetModuleFileNameA(hModule, lpBuffer, nBufferLength);
	}

	static DWORD GetModuleFileNameT(HMODULE hModule, LPWSTR lpBuffer,
									DWORD nBufferLength)
	{
		return ::GetModuleFileNameW(hModule, lpBuffer, nBufferLength);
	}

public:
	CPathExT(PCXSTR pszPath)
		: BasePath(pszPath)
	{
	}
	CPathExT( ) throw( ) {}

	bool MakeFullPath();
	bool MakeFullPath(PCXSTR pszPath);

	bool GetTempPath();
	bool GetModuleFileName(HMODULE hModule);  // NULL - running EXE

	bool GetCurrentDirectory();
	bool IsEmpty() const
	{
		return m_strPath.IsEmpty();
	}

	CPathExT & operator =(StringType const & src)
	{
		m_strPath = src;
		return *this;
	}
};

template<typename StringType>
bool CPathExT<StringType>::MakeFullPath()
{
	StringType tmp;
	typename StringType::XCHAR tmpchar[2];
	typename StringType::XCHAR * pFilePart;

	DWORD Length = GetFullPathNameT(*this, 2, tmpchar, & pFilePart);
	if (0 == Length)
	{
		return false;
	}

	typename StringType::XCHAR * pBuf = tmp.GetBuffer(Length + 1);
	if (NULL != pBuf)
	{
		DWORD ExpandedLength = GetFullPathNameT(*this, Length + 1, pBuf, & pFilePart);
		if (ExpandedLength <= Length)
		{
			tmp.ReleaseBuffer(ExpandedLength);
			static_cast<StringType &>(*this) = tmp;
			return true;
		}
		else
		{
			tmp.ReleaseBuffer(0);
		}
	}
	return false;
}

template<typename StringType>
bool CPathExT<StringType>::MakeFullPath(PCXSTR pszPath)
{
	typename StringType::XCHAR tmpchar[2];
	typename StringType::XCHAR * pFilePart;

	if (NULL == pszPath
		|| 0 == pszPath[0])
	{
		return false;
	}

	DWORD Length = GetFullPathNameT(pszPath, 2, tmpchar, & pFilePart);
	if (0 == Length)
	{
		return false;
	}

	typename StringType::XCHAR * pBuf = m_strPath.GetBuffer(Length + 1);

	if (NULL != pBuf)
	{
		DWORD ExpandedLength = GetFullPathNameT(pszPath, Length + 1, pBuf, & pFilePart);

		if (ExpandedLength <= Length)
		{
			m_strPath.ReleaseBuffer(ExpandedLength);
			return true;
		}
		else
		{
			m_strPath.ReleaseBuffer(0);
		}
	}
	return false;
}

template<typename StringType>
bool CPathExT<StringType>::GetCurrentDirectory()
{
	typename StringType::XCHAR tmpchar[2];

	DWORD Length = GetCurrentDirectoryT(2, tmpchar);
	if (0 == Length)
	{
		return false;
	}

	typename StringType::XCHAR * pBuf = m_strPath.GetBuffer(Length + 1);

	if (NULL != pBuf)
	{
		DWORD ExpandedLength = GetCurrentDirectoryT(Length + 1, pBuf);
		if (ExpandedLength <= Length)
		{
			m_strPath.ReleaseBuffer(ExpandedLength);
			return true;
		}
		else
		{
			m_strPath.ReleaseBuffer(0);
		}
	}
	return false;
}

template<typename StringType>
bool CPathExT<StringType>::GetModuleFileName(HMODULE hModule)
{
	typename StringType::XCHAR tmpchar[2];

	DWORD Length = GetModuleFileNameT(hModule, tmpchar, 2);

	if (0 == Length)
	{
		return false;
	}

	typename StringType::XCHAR * pBuf = m_strPath.GetBuffer(Length + 1);

	if (NULL != pBuf)
	{
		DWORD ExpandedLength = GetModuleFileNameT(hModule, pBuf, Length + 1);

		if (ExpandedLength <= Length)
		{
			m_strPath.ReleaseBuffer(ExpandedLength);
			return true;
		}
		else
		{
			m_strPath.ReleaseBuffer(0);
		}
	}
	return false;
}

template<typename StringType>
bool CPathExT<StringType>::GetTempPath()
{
	typename StringType::XCHAR tmpchar[2];

	DWORD Length = GetTempPathT(2, tmpchar);
	if (0 == Length)
	{
		return false;
	}

	typename StringType::XCHAR * pBuf = m_strPath.GetBuffer(Length + 1);

	if (NULL != pBuf)
	{
		DWORD ExpandedLength = GetTempPathT(Length + 1, pBuf);
		if (ExpandedLength <= Length)
		{
			m_strPath.ReleaseBuffer(ExpandedLength);
			return true;
		}
		else
		{
			m_strPath.ReleaseBuffer(0);
		}
	}
	return false;
}

typedef CPathExT<CString> CPathEx;
typedef CPathExT<CStringW> CPathExW;
typedef CPathExT<CStringA> CPathExA;
