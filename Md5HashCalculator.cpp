#include "stdafx.h"
#include "Md5HashCalculator.h"
#include <afxpriv.h>

CMd5HashCalculator::CMd5HashCalculator()
{
	m_HashBuf = (PBYTE)VirtualAlloc(NULL, BufferSize, MEM_COMMIT, PAGE_READWRITE);

	CryptAcquireContextA( & m_hProv, NULL, MS_DEF_PROV_A, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
}

CMd5HashCalculator::~CMd5HashCalculator()
{
	if (NULL != m_HashBuf)
	{
		VirtualFree(m_HashBuf, 0, MEM_RELEASE);
	}
}

BOOL CMd5HashCalculator::CalculateFileMd5Hash(LPCTSTR Filename,
											BYTE MD5Hash[16], BOOL volatile & bStopOperation,
											LONGLONG volatile & BytesComplete,
											HWND volatile const & hNotifyWnd)
{
	if (NULL == m_HashBuf
		|| NULL == m_hProv)
	{
		return FALSE;
	}

	CCryptMD5Hash hash;

	HANDLE hFile = NULL;
	BOOL res = TRUE;
	hFile = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}
	DWORD LastError = 0;
	if (S_OK == hash.Initialize(* this))
	{
		DWORD LastTime = GetTickCount();
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
			hash.AddData(m_HashBuf, BytesRead, 0);
			BytesComplete += BytesRead;

			if (NULL != hNotifyWnd
				&& GetTickCount() - LastTime >= 500)
			{
				::PostMessage(hNotifyWnd, WM_KICKIDLE, 0, 0);
				LastTime = GetTickCount();
			}
		}

		DWORD HashLen = 16;
		hash.GetValue(MD5Hash, & HashLen);
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
