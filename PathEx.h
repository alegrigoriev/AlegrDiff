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

public:
	CPathExT(PCXSTR pszPath)
		: BasePath(pszPath)
	{
	}
	CPathExT( ) throw( ) {}

	bool MakeFullPath();
	bool GetCurrentDirectory();

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
bool CPathExT<StringType>::GetCurrentDirectory()
{
	StringType tmp;
	typename StringType::XCHAR tmpchar[2];

	DWORD Length = GetCurrentDirectoryT(2, tmpchar);
	if (0 == Length)
	{
		return false;
	}

	typename StringType::XCHAR * pBuf = tmp.GetBuffer(Length + 1);

	if (NULL != pBuf)
	{
		DWORD ExpandedLength = GetCurrentDirectoryT(Length + 1, pBuf);
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

typedef CPathExT<CString> CPathEx;
typedef CPathExT<CStringW> CPathExW;
typedef CPathExT<CStringA> CPathExA;
