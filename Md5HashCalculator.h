#pragma once
#include <Wincrypt.h>
#include <atlcrypt.h>

class CMd5HashCalculator : protected CCryptProv
{
public:
	CMd5HashCalculator();
	~CMd5HashCalculator();
	BYTE * m_HashBuf;

	BOOL CalculateFileMd5Hash(LPCTSTR Filename,
							BYTE MD5Hash[16],
							BOOL volatile & bStopOperation,
							LONGLONG volatile & BytesComplete,
							HWND volatile const & hNotifyWnd);

	static int const BufferSize = 0x10000;

};
