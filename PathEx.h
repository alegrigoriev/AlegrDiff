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

public:
	CPathExT(PCXSTR pszPath)
		: BasePath(pszPath)
	{
	}
	CPathExT( ) throw( ) {}

	bool MakeFullPath()
	{
		StringType tmp;
		typename StringType::XCHAR tmpchar[2];
		typename StringType::XCHAR * pFilePart;

		DWORD Length = GetFullPathNameT(*this, 2, tmpchar, & pFilePart);
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
	CPathExT & operator =(StringType const & src)
	{
		m_strPath = src;
		return *this;
	}
};

typedef CPathExT<CString> CPathEx;
typedef CPathExT<CStringW> CPathExW;
typedef CPathExT<CStringA> CPathExA;
