#include "stdafx.h"
#include "Md5HashCalculator.h"

CMd5HashCalculator::CMd5HashCalculator()
{
	m_CryptProvider = NULL;
	m_HashBuf = (PBYTE)VirtualAlloc(NULL, BufferSize, MEM_COMMIT, PAGE_READWRITE);
	CryptAcquireContext( & m_CryptProvider, NULL, MS_DEF_PROV, PROV_RSA_FULL,
						CRYPT_VERIFYCONTEXT);
}

CMd5HashCalculator::~CMd5HashCalculator()
{
	if (NULL != m_CryptProvider)
	{
		CryptReleaseContext(m_CryptProvider, 0);
	}
	if (NULL != m_HashBuf)
	{
		VirtualFree(m_HashBuf, 0, MEM_RELEASE);
	}
}

BOOL CMd5HashCalculator::CalculateFileMd5Hash(LPCTSTR Filename,
											BYTE MD5Hash[16], BOOL volatile & bStopOperation)
{
	if (NULL == m_HashBuf
		|| NULL == m_CryptProvider)
	{
		return FALSE;
	}
	HCRYPTHASH hash = NULL;
	HANDLE hFile = NULL;
	BOOL res = TRUE;
	hFile = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}
	DWORD LastError = 0;
	if (CryptCreateHash(m_CryptProvider, CALG_MD5, NULL, NULL, & hash))
	{
		while ( ! bStopOperation )
		{
			DWORD BytesRead;
			if ( ! ReadFile(hFile, m_HashBuf, BufferSize, & BytesRead, NULL))
			{
				LastError = GetLastError();
				res = FALSE;
				break;
			}
			if (0 == BytesRead)
			{
				break;
			}
			CryptHashData(hash, m_HashBuf, BytesRead, 0);
		}
		DWORD HashLen = sizeof MD5Hash;
		CryptGetHashParam(hash, HP_HASHVAL, MD5Hash, & HashLen, 0);
		CryptDestroyHash(hash);
	}
	CloseHandle(hFile);
	if (bStopOperation)
	{
		res = FALSE;
		LastError = ERROR_CANCELLED;
	}
	if (! res)
	{
		SetLastError(LastError);
	}
	return res;
}
