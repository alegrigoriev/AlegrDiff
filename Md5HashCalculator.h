#pragma once
#include <Wincrypt.h>

class CMd5HashCalculator
{
public:
	CMd5HashCalculator();
	~CMd5HashCalculator();
	BYTE * m_HashBuf;
	HCRYPTPROV m_CryptProvider;

	BOOL CalculateFileMd5Hash(LPCTSTR Filename,
							BYTE MD5Hash[16],
							BOOL volatile & bStopOperation);

	static int const BufferSize = 0x10000;

};
