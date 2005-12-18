// FileLine.cpp
#include "stdafx.h"
#include "FileLine.h"
#include "SmallAllocator.h"
#include "AlegrDiff.h"

#define C_CPP_FILE 1
#define REMOVE_VERSION_INFO 2

static DWORD CalculateHash(void const * pData, int len);

void * FileLine::operator new(size_t size)
{
	return m_Allocator.Allocate(size);
}

void FileLine::operator delete(void * ptr)
{
	m_Allocator.Free(ptr);
}

FileLine::FileLine(LPCTSTR src, int Length, bool /*MakeNormalizedString*/, bool c_cpp_file)
	: m_Flags(0),
	m_Length(Length),
	m_pNext(NULL),
	m_Number((unsigned)-1),
//m_Link(NULL),
//m_FirstTokenIndex(-1),
	m_HashCode(0),
	m_GroupHashCode(0),
	m_NormalizedHashCode(0),
	m_NormalizedGroupHashCode(0)
{
	TCHAR TmpBuf[4096];
	char WhitespaceMask[4096 / 8];

	unsigned Flags = 0; REMOVE_VERSION_INFO;
	if (c_cpp_file)
	{
		Flags |= C_CPP_FILE;
	}

	m_NormalizedStringLength = RemoveExtraWhitespaces(TmpBuf, src, countof(TmpBuf),
													WhitespaceMask, sizeof WhitespaceMask,
													Flags);

	int WhitespaceMaskLength = (m_Length + 7) / 8;

	m_pAllocatedBuf = ::new TCHAR[m_Length + m_NormalizedStringLength + 2 + (WhitespaceMaskLength + (sizeof(TCHAR) - 1)) / sizeof(TCHAR)];

	if (NULL != m_pAllocatedBuf)
	{
		TCHAR * pTmp = m_pAllocatedBuf;
		m_pString = pTmp;
		memcpy(pTmp, src, sizeof(TCHAR) * (m_Length + 1));
		m_HashCode = CalculateHash(m_pString, sizeof(TCHAR) * m_Length);

		pTmp += m_Length + 1;
		m_pNormalizedString = pTmp;
		memcpy(pTmp, TmpBuf, sizeof(TCHAR) * (m_NormalizedStringLength + 1));
		m_NormalizedHashCode = CalculateHash(m_pNormalizedString, sizeof(TCHAR) * m_NormalizedStringLength);

		pTmp += m_NormalizedStringLength + 1;
		m_pWhitespaceMask = (char*)(pTmp);
		memcpy(pTmp, WhitespaceMask, WhitespaceMaskLength);
	}
	else
	{
		m_pString = NULL;
		m_pNormalizedString = NULL;
		m_Length = 0;
		m_NormalizedStringLength = 0;
		m_pWhitespaceMask = NULL;

		m_HashCode = 0xFFFFFFFF;
		m_NormalizedHashCode = 0xFFFFFFFF;
	}
}

FileLine::~FileLine()
{
	delete[] m_pAllocatedBuf;
}

int _cdecl FileLine::HashCompareFunc(FileLine const * pLine1, FileLine const * pLine2)
{
	if (pLine1->GetHash() < pLine2->GetHash())
	{
		return 1;
	}
	return 0;
}

int _cdecl FileLine::HashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2)
{
	// if hash is the same, compare line numbers
	if (pLine1->GetHash() < pLine2->GetHash())
	{
		return 1;
	}
	if (pLine1->GetHash() == pLine2->GetHash()
		&& pLine1->GetLineNumber() < pLine2->GetLineNumber())
	{
		return 1;
	}
	return 0;
}

int _cdecl FileLine::GroupHashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2)
{
	if (pLine1->GetGroupHash() < pLine2->GetGroupHash())
	{
		return 1;
	}
	// if hash is the same, compare line numbers
	if (pLine1->GetGroupHash() == pLine2->GetGroupHash()
		&& pLine1->GetLineNumber() < pLine2->GetLineNumber())
	{
		return 1;
	}
	return 0;
}

// CRC32 Lookup Table generated from Charles Michael
//  Heard's CRC-32 code
static DWORD CRC32_Table[256] =
{
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
	0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
	0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
	0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
	0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
	0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
	0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
	0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
	0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
	0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
	0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
	0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
	0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
	0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
	0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
	0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
	0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
	0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
	0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
	0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
	0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
	0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
	0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
	0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
	0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
	0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
	0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
	0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
	0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
	0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
	0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
	0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
	0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
	0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
	0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
	0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
	0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
	0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
	0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
	0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
	0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
	0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
	0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

static DWORD CalculateHash(void const * pData, int len)
{
	// CRC32
	DWORD	crc32_val = 0xFFFFFFFF;
	const unsigned char * data = (const unsigned char *) pData;

	// Calculate a CRC32 value
	for (int i = 0 ; i < len; i++)
	{
		char c = data[i];
		crc32_val = ( crc32_val << 8 ) ^ CRC32_Table[(( crc32_val >> 24) ^ c) & 0xff];
	}
	return crc32_val;
}
// remove the unnecessary whitespaces from the line (based on C, C++ syntax)
// return string length
int FileLine::RemoveExtraWhitespaces(LPTSTR pDst, LPCTSTR Src, unsigned DstCount,
									char * pWhitespaceMask, int WhitespaceMaskSize,
									unsigned Flags)
{
	// pDst buffer size must be greater or equal strlen(pSrc)
	bool const c_cpp_file = 0 != (Flags & C_CPP_FILE);

	unsigned int SrcIdx = 0;
	unsigned int DstIdx = 0;
	unsigned int FirstWhitespaceIndex = 0;
	bool RemovedWhitespaces = false;
	bool LeaveOneExtraSpace = false;
	TCHAR cPrevChar = 0;
	bool PrevCharAlpha = false;
	unsigned int WhitespaceMaskBits = WhitespaceMaskSize * 8;
	if (NULL != pWhitespaceMask)
	{
		memset(pWhitespaceMask, 0, WhitespaceMaskSize);
	}
	else
	{
		WhitespaceMaskBits = 0;
	}

	TCHAR c;
	// remove whitespaces at the begin
	while ((c = Src[SrcIdx]) == ' '
			|| '\t' == c)
	{
		if (WhitespaceMaskBits > SrcIdx)
		{
			pWhitespaceMask[SrcIdx / 8] |= (1 << (SrcIdx & 7));
		}
		SrcIdx++;
	}

	// check if it is preprocessor line
	LPCTSTR VersionBegin = NULL;
	LPCTSTR VersionEnd = NULL;

	if (c_cpp_file && '#' == Src[SrcIdx])
	{
		// consider all characters alpha, that is don't remove all but one spaces
		LeaveOneExtraSpace = true;
	}
	else if (Flags & REMOVE_VERSION_INFO)
	{
		VersionBegin = _tcschr(Src, '$');

		if (NULL != VersionBegin)
		{
			VersionEnd = _tcsrchr(VersionBegin + 1, '$');

			static LPCTSTR const keywords[] =
			{
				_T("Archive:"),
				_T("Author:"),
				_T("Date:"),
				_T("Header:"),
				_T("History:"),
				_T("JustDate:"),
				_T("Log:"),
				_T("Logfile:"),
				_T("Modtime:"),
				_T("Revision:"),
				_T("Workfile:"),
			};

			unsigned i;

			if (NULL != VersionEnd)
				for (i = 0; i < countof(keywords); i++)
				{
					int len = _tcslen(keywords[i]);

					if (0 == _tcsncmp(VersionBegin + 1, keywords[i], len))
					{
						// revision info
						// mark everything in between as white spaces

						for (VersionBegin += len + 1 ; VersionBegin < VersionEnd;
							VersionBegin++)
						{
							unsigned index = VersionBegin - Src;
							if (index < WhitespaceMaskBits)
							{
								pWhitespaceMask[index / 8] |= 1 << (index & 7);
							}
						}
						m_Flags |= eContainsVersionInfo;
						break;
					}
				}
		}
	}

	while (Src[SrcIdx] && DstIdx + 1 < DstCount)
	{
		// it's OK to remove extra spaces between non-alpha bytes,
		// unless these are the following pairs:
		// /*, */, //, ++, --, &&, ||, ##, ->*, ->, >>, << >=, <=, ==, ::
		// that is most two-char operators
		// strings also must be kept intact.
		// Can remove extra spaces between alpha and non-alpha, unless LeaveOneExtraSpace = true
		if (SrcIdx < WhitespaceMaskBits
			&& pWhitespaceMask[SrcIdx / 8] & (1 << (SrcIdx & 7)))
		{
			// this was a removed version info
			SrcIdx++;
			continue;
		}

		c = Src[SrcIdx];
		bool c_IsAlpha = (_istalnum(TCHAR_MASK & c)  || '_' == c);
		if ((RemovedWhitespaces && LeaveOneExtraSpace) || (PrevCharAlpha && c_IsAlpha))
		{
			if(DstIdx + 1 >= DstCount)
			{
				break;
			}
			// insert one whitespace to the output string
			pDst[DstIdx] = ' ';
			DstIdx++;
			RemovedWhitespaces = false;
			// mark the previous space as non extra whitespace
			if (FirstWhitespaceIndex != 0 && WhitespaceMaskBits > FirstWhitespaceIndex)
			{
				pWhitespaceMask[FirstWhitespaceIndex / 8] &= ~(1 << (FirstWhitespaceIndex & 7));
			}
		}
		if(c_IsAlpha)
		{
			// move all alpha chars till non-alpha
			do
			{
				if(DstIdx + 1 >= DstCount)
				{
					break;
				}
				pDst[DstIdx] = c;
				cPrevChar = c;
				SrcIdx++;
				DstIdx++;
				c = Src[SrcIdx];
			} while (_istalnum(TCHAR_MASK & c) || '_' == c);

			PrevCharAlpha = true;
		}
		else
		{
			if(c_cpp_file && RemovedWhitespaces)
			{

				// check if we need to insert a whitespace
				static int ReservedPairs[] =
				{
					'//', '/*', '*/', '++', '--', '!=', '##', '%=',
					'^=', '&=', '&&', '-=', '+=', '==', '::', '<<',
					'>>', '||', '|=', '<=', '>=', '/=', '\'\'', '""',
					'L"', 'L\'',
				};
				// may be non-portable to big-endian
				int pair = ((cPrevChar & TCHAR_MASK) << 16) | (c & TCHAR_MASK);
				for (int i = 0; i < countof(ReservedPairs); i++)
				{
					int ReservedPair = (ReservedPairs[i] & 0xFF) | ((ReservedPairs[i] & 0xFF00) << 8);
					if (pair == ReservedPair)
					{
						if(DstIdx + 1 < DstCount)
						{
							pDst[DstIdx] = ' ';
							DstIdx++;
							// mark the previous space as non extra whitespace
							if (FirstWhitespaceIndex != 0 && WhitespaceMaskBits > FirstWhitespaceIndex)
							{
								pWhitespaceMask[FirstWhitespaceIndex / 8] &= ~(1 << (FirstWhitespaceIndex & 7));
							}
						}
						break;
					}
				}
			}
			// move all non-alpha non whitespace chars
			// check for a string or character constant
			if (c_cpp_file && '\'' == c)
			{
				// character constant
				// skip everything till the next '. Process \'
				cPrevChar = c;
				do
				{
					if(DstIdx + 1 >= DstCount)
					{
						break;
					}
					pDst[DstIdx] = c;
					SrcIdx++;
					DstIdx++;
					c = Src[SrcIdx];
					if ('\'' == c)
					{
						// if the next char is double quote,
						// skip both
						if(DstIdx + 1 >= DstCount)
						{
							break;
						}
						pDst[DstIdx] = c;
						SrcIdx++;
						DstIdx++;
						c = Src[SrcIdx];
						if ('\'' == c)
						{
							continue;
						}
						break;
					}
					else if ('\\' ==c)
					{
						// skip the next char
						if(DstIdx + 1 >= DstCount)
						{
							break;
						}
						pDst[DstIdx] = c;
						SrcIdx++;
						DstIdx++;
						c = Src[SrcIdx];
					}
				} while (c != 0);
			}
			else if (c_cpp_file && '"' == c)
			{
				// char string
				cPrevChar = c;
				do
				{
					if(DstIdx + 1 >= DstCount)
					{
						break;
					}
					pDst[DstIdx] = c;
					SrcIdx++;
					DstIdx++;
					c = Src[SrcIdx];
					if ('"' == c)
					{
						// if the next char is double quote,
						// skip both
						if(DstIdx + 1 >= DstCount)
						{
							break;
						}
						pDst[DstIdx] = c;
						SrcIdx++;
						DstIdx++;
						c = Src[SrcIdx];
						if ('"' == c)
						{
							continue;
						}
						break;
					}
					else if ('\\' ==c)
					{
						// skip the next char
						if(DstIdx + 1 >= DstCount)
						{
							break;
						}
						pDst[DstIdx] = c;
						SrcIdx++;
						DstIdx++;
						c = Src[SrcIdx];
					}
				} while (c != 0);
			}
			else do
			{
				// move all non-alphanumeric
				if(DstIdx + 1 >= DstCount)
				{
					break;
				}
				cPrevChar = c;
				pDst[DstIdx] = c;
				SrcIdx++;
				DstIdx++;
				c = Src[SrcIdx];
			} while (c != 0
					&& c != ' '
					&& c != '\t'
					&& ! (_istalnum(TCHAR_MASK & c) || '_' == c));
			PrevCharAlpha = false;
		}
		// remove whitespaces
		RemovedWhitespaces = false;
		FirstWhitespaceIndex = SrcIdx;
		while ((c = Src[SrcIdx]) == ' '
				|| '\t' == c)
		{
			RemovedWhitespaces = true;
			if (WhitespaceMaskBits > SrcIdx)
			{
				pWhitespaceMask[SrcIdx / 8] |= (1 << (SrcIdx & 7));
			}
			SrcIdx++;
		}
	}

	pDst[DstIdx] = 0;

	return DstIdx;
}

bool FileLine::IsEqual(const FileLine * pOtherLine) const
{
	if (m_HashCode != pOtherLine->m_HashCode
		|| m_Length != pOtherLine->m_Length)
	{
		return false;
	}
	if (0 == m_Length)
	{
		return true;
	}
	return 0 == memcmp(m_pString, pOtherLine->m_pString, m_Length);
}

bool FileLine::IsNormalizedEqual(const FileLine * pOtherLine) const
{
	if (m_NormalizedHashCode != pOtherLine->m_NormalizedHashCode
		|| m_NormalizedStringLength != pOtherLine->m_NormalizedStringLength)
	{
		return false;
	}
	if (0 == m_NormalizedStringLength)
	{
		return true;
	}
	return 0 == memcmp(m_pNormalizedString, pOtherLine->m_pNormalizedString, m_NormalizedStringLength);
}

void FileLine::CalculateGroupHash(DWORD const src[], size_t size)
{
	SetGroupHash(CalculateHash(src, size));
}

void FileLine::CalculateNormalizedGroupHash(DWORD const src[], size_t size)
{
	SetNormalizedGroupHash(CalculateHash(src, size));
}

CSmallAllocator FileLine::m_Allocator(sizeof FileLine);
