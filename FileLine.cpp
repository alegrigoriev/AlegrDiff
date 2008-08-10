// FileLine.cpp
#include "stdafx.h"
#include "FileLine.h"
#include "SmallAllocator.h"
#include "AlegrDiff.h"
#include "FileListSupport.h"    // FIXME: StringSection

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

// find difference in the strings, and build the array of inserts and
// and deleted chars
// returns number of different characters.
// if ppSections is not NULL, builds a list of sections
struct StringDiffSection : public ListItem<StringDiffSection>
{
public:
	static void * operator new(size_t size)
	{
		return m_Allocator.Allocate(size);
	}
	static void operator delete(void * ptr)
	{
		m_Allocator.Free(ptr);
	}
	StringDiffSection();

	LPCTSTR Str1;
	unsigned Len1;
	LPCTSTR Str2;
	unsigned Len2;

	LPCTSTR Str1ws;
	unsigned Len1ws;
	LPCTSTR Str2ws;
	unsigned Len2ws;

	bool Whitespace;
	bool Different;
private:
	static CSmallAllocator m_Allocator;
};

StringDiffSection::StringDiffSection()
{
	Whitespace = false;
	Different = false;
	Str1 = NULL;
	Str2 = NULL;
	Len1 = 0;
	Len2 = 0;
	Str1ws = NULL;
	Str2ws = NULL;
	Len1ws = 0;
	Len2ws = 0;
}
CSmallAllocator StringDiffSection::m_Allocator(sizeof (StringDiffSection));

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

void * LinePair::operator new(size_t size)
{
	return m_Allocator.Allocate(size);
}
void LinePair::operator delete(void * ptr)
{
	m_Allocator.Free(ptr);
}

LinePair::LinePair(const FileLine * pLine)
{
	pFirstLine = pLine;
	pSecondLine = pLine;

	StringSection * pSection = new StringSection;
	pSection->Attr = pSection->Identical;
	pSection->pDiffSection = NULL;
	pSection->pBegin = pLine->GetText();
	pSection->Length = (USHORT)pLine->GetLength();

	StrSections.InsertTail(pSection);
}

LinePair::LinePair(const FileLine * pLine1, const FileLine * pLine2, FileDiffSection * pDiffSection)
	: pFirstLine(pLine1)
	, pSecondLine(pLine2)
{
	if (NULL == pLine1 || NULL == pLine2 || pLine1 == pLine2)
	{
		StringSection * pSection = new StringSection;
		const FileLine * pLine = pLine1;

		if (pLine1 == pLine2)
		{
			pSection->Attr = StringSection::Identical;
		}
		else if (NULL == pLine1)
		{
			pSection->Attr = StringSection::Inserted | StringSection::Undefined;
			pLine = pLine2;
		}
		else
		{
			pSection->Attr = StringSection::Erased | StringSection::Undefined;
		}

		pSection->pBegin = pLine->GetText();
		pSection->Length = (USHORT)pLine->GetLength();
		pSection->pDiffSection = pDiffSection;

		if (pLine1 != pLine2 && pLine->IsBlank())
		{
			pSection->Attr |= StringSection::Whitespace;
		}
		else if (NULL != pDiffSection)
		{
			pDiffSection->m_Flags &= ~FileDiffSection::FlagWhitespace;
		}

		StrSections.InsertTail(pSection);
	}
	else
	{
	}
}

LPCTSTR LinePair::GetText(LPTSTR buf, const size_t nBufChars, int * pStrLen,
						BOOL IgnoreWhitespaces, int SelectFile)
{
	// SelectFile: 1 - first file, 2 - second file, any other: both files
	if (1 == SelectFile)
	{
		if (NULL != pFirstLine)
		{
			*pStrLen = pFirstLine->GetLength();
			return pFirstLine->GetText();
		}
		else
		{
			*pStrLen = 0;
			buf[0] = 0;
			return buf;
		}
	}
	if (2 == SelectFile)
	{
		if (NULL != pSecondLine)
		{
			*pStrLen = pSecondLine->GetLength();
			return pSecondLine->GetText();
		}
		else
		{
			*pStrLen = 0;
			buf[0] = 0;
			return buf;
		}
	}

	if (NULL == pFirstLine)
	{
		*pStrLen = pSecondLine->GetLength();
		return pSecondLine->GetText();
	}

	if (NULL == pSecondLine)
	{
		*pStrLen = pFirstLine->GetLength();
		return pFirstLine->GetText();
	}
	// make a string of string sections
	int StrLen = 0;
	for (StringSection * pSection = StrSections.First();
		StrSections.NotEnd(pSection) && StrLen + 1u < nBufChars; pSection = pSection->Next())
	{
		if ((pSection->Attr & pSection->Whitespace)
			&& (pSection->Attr & pSection->Erased)
			&& IgnoreWhitespaces)
		{
			continue;   // don't show the section
		}
		int len = pSection->Length;
		if (StrLen + len + 1u > nBufChars)
		{
			len = nBufChars - StrLen - 1;
		}
		_tcsncpy_s(buf + StrLen, nBufChars-StrLen, pSection->pBegin, len);
		StrLen += len;
	}
	buf[StrLen] = 0;
	*pStrLen = StrLen;
	return buf;
}

int LinePair::LinePosToDisplayPos(int position, BOOL bIgnoreWhitespaces, int FileScope)
{
	if ( ! bIgnoreWhitespaces && 0 == FileScope)
	{
		return position;
	}
	int pos = 0;
	int adj = 0;

	for (StringSection * pSection = StrSections.First();
		StrSections.NotEnd(pSection); pSection = pSection->Next())
	{
		pos += pSection->Length;
		if ((pSection->Attr & pSection->Erased)
			&& (((pSection->Attr & pSection->Whitespace)
					&& bIgnoreWhitespaces && 0 == FileScope)
				|| 2 == FileScope))
		{
			adj += pSection->Length;
			if (pos >= position)
			{
				// return begin of the segment
				return pos - adj;
			}
		}
		else if (1 == FileScope
				&& (pSection->Attr & pSection->File2Only))
		{
			adj += pSection->Length;
			if (pos >= position)
			{
				// return begin of the segment
				return pos - adj;
			}
		}
		else
		{
			if (pos >= position)
			{
				return position - adj;
			}
		}
	}
	return position - adj;
}

int LinePair::DisplayPosToLinePos(int position, BOOL bIgnoreWhitespaces, int FileScope)
{
	if ( ! bIgnoreWhitespaces && 0 == FileScope)
	{
		return position;
	}
	int pos = 0;
	int adj = 0;

	for (StringSection * pSection = StrSections.First();
		StrSections.NotEnd(pSection); pSection = pSection->Next())
	{
		if ((pSection->Attr & pSection->Erased)
			&& (((pSection->Attr & pSection->Whitespace)
					&& bIgnoreWhitespaces && 0 == FileScope)
				|| 2 == FileScope))
		{
			adj += pSection->Length;
		}
		else if ((pSection->Attr & pSection->File2Only)
				&& 1 == FileScope)
		{
			adj += pSection->Length;
		}
		else
		{
			pos += pSection->Length;
			if (pos >= position)
			{
				return position + adj;
			}
		}
	}
	return position + adj;
}

void LinePair::BuildDiffSectionsList(int nLineIndex, std::vector<class FileDiffSection*> & DiffSections, int MinMatchingChars)
{
	MatchStrings(pFirstLine, pSecondLine, & StrSections, MinMatchingChars);

	int pos = 0;
	for (StringSection * pSection = StrSections.First();
		StrSections.NotEnd(pSection); pSection = pSection->Next())
	{
		if (pSection->Identical != pSection->Attr)
		{
			FileDiffSection * pDiffSection = new FileDiffSection;
			pSection->pDiffSection = pDiffSection;
			if (NULL != pDiffSection)
			{
				pDiffSection->m_Begin.line = nLineIndex;
				pDiffSection->m_Begin.pos = pos;
				pDiffSection->m_End.line = nLineIndex;
				pos += pSection->Length;
				pDiffSection->m_End.pos = pos;

				if (pSection->Attr & pSection->Whitespace)
				{
					pDiffSection->m_Flags |= FileDiffSection::FlagWhitespace;
				}
				else if (pSection->Attr & pSection->VersionInfo)
				{
					pDiffSection->m_Flags |= FileDiffSection::FlagVersionInfoDifferent;
				}

				if (StrSections.Last() != pSection
					&& pSection->Next()->Attr != pSection->Identical)
				{
					pSection = pSection->Next();

					if (0 == (pSection->Attr & pSection->Whitespace))
					{
						pDiffSection->m_Flags &= ~FileDiffSection::FlagWhitespace;
					}
					if (0 == (pSection->Attr & pSection->VersionInfo))
					{
						pDiffSection->m_Flags &= ~FileDiffSection::FlagVersionInfoDifferent;
					}

					pSection->pDiffSection = pDiffSection;
					pos += pSection->Length;
					pDiffSection->m_End.pos = pos;
				}
				DiffSections.push_back(pDiffSection);
			}
		}
		else
		{
			pSection->pDiffSection = NULL;
			pos += pSection->Length;
		}
	}
}

int MatchStrings(const FileLine * pStr1, const FileLine * pStr2, ListHead<StringSection> * ppSections, int nMinMatchingChars)
{
	if (NULL == pStr2 && NULL == pStr1)
	{
		return 0;
	}

	if (NULL == pStr2)
	{
		if (NULL == ppSections)
		{
			return pStr1->GetLength();
		}
		StringSection * pSection = new StringSection;
		if (NULL != pSection)
		{
			pSection->pBegin = pStr1->GetText();
			pSection->Length = (USHORT)pStr1->GetLength();
			pSection->Attr = StringSection::Erased | StringSection::Undefined;

			// check if it is whitespace difference
			if (pStr1->IsBlank())
			{
				pSection->Attr |= pSection->Whitespace;
			}
			ppSections->InsertTail(pSection);
		}
		return pStr1->GetLength();
	}

	if (NULL == pStr1)
	{
		if (NULL == ppSections)
		{
			return pStr2->GetLength();
		}
		StringSection * pSection = new StringSection;
		if (NULL != pSection)
		{
			pSection->pBegin = pStr2->GetText();
			pSection->Length = (USHORT)pStr2->GetLength();
			pSection->Attr = StringSection::Inserted | StringSection::Undefined;

			// check if it is whitespace difference
			if (pStr2->IsBlank())
			{
				pSection->Attr |= pSection->Whitespace;
			}
			ppSections->InsertTail(pSection);
		}
		return pStr2->GetLength();
	}

	// text without whitespaces
	LPCTSTR str1 = pStr1->GetNormalizedText();
	LPCTSTR str2 = pStr2->GetNormalizedText();

	ASSERT(_tcslen(str1) == pStr1->GetNormalizedLength());
	ASSERT(_tcslen(str2) == pStr2->GetNormalizedLength());

	ListHead<StringDiffSection> DiffSections;

	// allocate an equal section of zero length and add to the list
	StringDiffSection * pDiffSection;

	LPCTSTR pEqualStrBegin1 = str1;
	LPCTSTR pEqualStrBegin2 = str2;

	while (1)
	{
		if (str1[0] == str2[0]
			&& str1[0] != 0)
		{
			// next non-blank char is the same
			// check if we can skip whitespace characters
			str1++;
			str2++;
			continue;
		}

		if (str1 != pEqualStrBegin1)
		{
			StringDiffSection * pDiffSection = new StringDiffSection;
			if (NULL != pDiffSection)
			{
				pDiffSection->Str1 = pEqualStrBegin1;
				pDiffSection->Len1 = str1 - pEqualStrBegin1;
				pDiffSection->Str2 = pEqualStrBegin2;
				pDiffSection->Len2 = str2 - pEqualStrBegin2;

				DiffSections.InsertTail(pDiffSection);
			}
		}
		if (str1[0] == 0
			&& str2[0] == 0)
		{
			break;
		}
		// difference found, starting from str1, str2

		bool found = false;
		unsigned idx1, idx2;
		for (idx1 = 0, idx2 = 0; (str1[idx1] != 0 || str2[idx2] != 0) && ! found; )
		{
			// check if str1+i ( i < idx1) equal str2+idx2
			for (unsigned i = 0; i <= idx1; i++)
			{
				LPCTSTR tmp1 = str1 + i;
				LPCTSTR tmp2 = str2 + idx2;
				int j;
				for (j = 0; j < nMinMatchingChars; j++)
				{
					if (tmp1[j] != tmp2[j]
						//|| tmp1[j] == 0 // no need to test both, because they are the same
						|| tmp2[j] == 0)
					{
						break;
					}
				}
				if (j == nMinMatchingChars
					//|| (tmp2[j] == 0 && tmp1[j] == 0)
					)
				{
					// create new section
					idx1 = i;
					found = true;
					break;
				}
			}
			if ( ! found)
			{
				// check if str2+i ( i < idx2) equal str1+idx1
				for (int i = 0; i <= idx2; i++)
				{
					LPCTSTR tmp1 = str1 + idx1;
					LPCTSTR tmp2 = str2 + i;
					int j;
					for (j = 0; j < nMinMatchingChars; j++)
					{
						if (tmp1[j] != tmp2[j]
							//|| tmp1[j] == 0 // no need to test both, because they are the same
							|| tmp2[j] == 0)
						{
							break;
						}
					}
					if (j == nMinMatchingChars
						//|| (tmp2[j] == 0 && tmp1[j] == 0)
						)
					{
						// create new section
						idx2 = i;
						found = true;
						break;
					}
				}
			}
			if (found)
			{
				break;
			}
			// only increment the distance index, if they don't go beyound the string
			if (str1[idx1] != 0)
			{
				idx1++;
			}
			if (str2[idx2] != 0)
			{
				idx2++;
			}
		}
		// create new section
		// if end reached, check if there are identical characters in the end of the line
		if (0 == str1[idx1]
			&& 0 == str2[idx2])
		{
			for (int i = 0; i < nMinMatchingChars && idx1 > 0 && idx2 > 0; i++)
			{
				if (str1[idx1 - 1] == str2[idx2 - 1])
				{
					idx1--;
					idx2--;
				}
			}
		}

//        int nDiffChars1 = 0;
//        int nDiffChars2 = 0;

		if (idx1 != 0
			|| idx2 != 0)
		{
			StringDiffSection * pDiffSection = new StringDiffSection;
			if (NULL != pDiffSection)
			{
				pDiffSection->Str1 = str1;
				pDiffSection->Len1 = idx1;
				if (0 == idx1)
				{
					pDiffSection->Str1 = NULL;
				}

				pDiffSection->Str2 = str2;
				pDiffSection->Len2 = idx2;
				if (0 == idx2)
				{
					pDiffSection->Str2 = NULL;
				}

				pDiffSection->Different = true;
				DiffSections.InsertTail(pDiffSection);
			}
		}

		str1 += idx1;
		str2 += idx2;
		pEqualStrBegin1 = str1;
		pEqualStrBegin2 = str2;
	}
	// convert a string offsets with whitespaces removed to offsets in a source string

	// text WITH whitespaces
	LPCTSTR str1ws = pStr1->GetText();
	LPCTSTR str1VersionBegin = _tcschr(str1ws, '$');
	LPCTSTR str1VersionEnd = _tcsrchr(str1ws, '$');

	LPCTSTR str2ws = pStr2->GetText();

	LPCTSTR str2VersionBegin = _tcschr(str2ws, '$');
	LPCTSTR str2VersionEnd = _tcsrchr(str2ws, '$');

	ASSERT(_tcslen(str1ws) == pStr1->GetLength());
	ASSERT(_tcslen(str2ws) == pStr2->GetLength());

	if (NULL != str1VersionBegin
		&& NULL != str2VersionBegin)
	{
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
		for (i = 0; i < countof(keywords); i++)
		{
			int len = _tcslen(keywords[i]);
			if (0 == _tcsncmp(str1VersionBegin + 1, keywords[i], len)
				&& 0 == _tcsncmp(str2VersionBegin + 1, keywords[i], len))
			{
				break;
			}
		}
		if (countof(keywords) == i)
		{
			// no revision info
			str1VersionBegin = NULL;
			str2VersionBegin = NULL;
			str1VersionEnd = NULL;
			str2VersionEnd = NULL;
		}
	}

	str1 = pStr1->GetNormalizedText();
	str2 = pStr2->GetNormalizedText();

	for (pDiffSection = DiffSections.First();
		DiffSections.NotEnd(pDiffSection); pDiffSection = DiffSections.Next(pDiffSection))
	{
		if (NULL != pDiffSection->Str1)
		{
			while (str1 < pDiffSection->Str1)
			{
				while(*str1ws != *str1)
				{
					str1ws++;
				}
				str1++;
				str1ws++;
			}
			while(*str1ws != *str1)
			{
				str1ws++;
			}
			pDiffSection->Str1ws = str1ws;
			for (unsigned i = 0; i < pDiffSection->Len1; i++, str1++, str1ws++)
			{
				while(*str1ws != *str1)
				{
					str1ws++;
				}
			}
			pDiffSection->Len1ws = str1ws - pDiffSection->Str1ws;
		}

		if (NULL != pDiffSection->Str2)
		{
			while (str2 < pDiffSection->Str2)
			{
				while(*str2ws != *str2)
				{
					str2ws++;
				}
				str2++;
				str2ws++;
			}
			while(*str2ws != *str2)
			{
				str2ws++;
			}
			pDiffSection->Str2ws = str2ws;
			for (unsigned i = 0; i < pDiffSection->Len2; i++, str2++, str2ws++)
			{
				while(*str2ws != *str2)
				{
					str2ws++;
				}
			}
			pDiffSection->Len2ws = str2ws - pDiffSection->Str2ws;
		}
	}

	if (0)
	{
		for (pDiffSection = DiffSections.First();
			DiffSections.NotEnd(pDiffSection); pDiffSection = pDiffSection->Next())
		{
			// print the diff sections
			if (0 != pDiffSection->Len1ws)
			{
				TRACE(_T("Str1: len=%d, str=\"%.*s\"\n"),
					pDiffSection->Len1, pDiffSection->Len1, pDiffSection->Str1);
				TRACE(_T("Str1ws: len=%d, str=\"%.*s\"\n"),
					pDiffSection->Len1ws, pDiffSection->Len1ws, pDiffSection->Str1ws);
			}
			if (0 != pDiffSection->Len2ws)
			{
				TRACE(_T("Str2: len=%d, str=\"%.*s\"\n"),
					pDiffSection->Len2, pDiffSection->Len2, pDiffSection->Str2);
				TRACE(_T("Str2ws: len=%d, str=\"%.*s\"\n"),
					pDiffSection->Len2ws, pDiffSection->Len2ws, pDiffSection->Str2ws);
			}
		}
	}

	// allocate an equal section of zero length as the first section and add to the list
	str1ws = pStr1->GetText();
	str2ws = pStr2->GetText();
	pDiffSection = new StringDiffSection;

	pDiffSection->Str1ws = str1ws;
	pDiffSection->Str2ws = str2ws;

	DiffSections.InsertHead(pDiffSection);

	// allocate an equal section of zero length as the last section and add to the list
	pDiffSection = new StringDiffSection;

	pDiffSection->Str1ws = str1ws + pStr1->GetLength();
	pDiffSection->Str2ws = str2ws + pStr2->GetLength();

	DiffSections.InsertTail(pDiffSection);

	// Now expand all "equal" sections to include preceding and trailing blanks

	// first expand them to make number of file1/file2 blanks equal
	for (pDiffSection = DiffSections.First();
		DiffSections.NotEnd(pDiffSection); pDiffSection = DiffSections.Next(pDiffSection))
	{
		if (! pDiffSection->Different)
		{
			// find number of heading blanks
			unsigned blanks1, blanks2;
			for (blanks1 = 0;
				blanks1 < pDiffSection->Len1ws && ' ' == pDiffSection->Str1ws[blanks1];
				blanks1++)
			{
			}
			for (blanks2 = 0;
				blanks2 < pDiffSection->Len2ws && ' ' == pDiffSection->Str2ws[blanks2];
				blanks2++)
			{
			}

			while (blanks1 < blanks2
					&& pDiffSection->Str1ws > str1ws
					&& ' ' == pDiffSection->Str1ws[-1])
			{
				blanks1++;
				pDiffSection->Str1ws--;
				pDiffSection->Len1ws++;
			}

			while (blanks2 < blanks1
					&& pDiffSection->Str2ws > str2ws
					&& ' ' == pDiffSection->Str2ws[-1])
			{
				blanks2++;
				pDiffSection->Str2ws--;
				pDiffSection->Len2ws++;
			}

			while (pDiffSection->Str1ws > str1ws
					&& ' ' == pDiffSection->Str1ws[-1]
					&& pDiffSection->Str2ws > str2ws
					&& ' ' == pDiffSection->Str2ws[-1])
			{
				pDiffSection->Str1ws--;
				pDiffSection->Len1ws++;
				pDiffSection->Str2ws--;
				pDiffSection->Len2ws++;
			}
		}
		if (NULL != pDiffSection->Str1ws)
		{
			str1ws = pDiffSection->Str1ws + pDiffSection->Len1ws;
		}
		if (NULL != pDiffSection->Str2ws)
		{
			str2ws = pDiffSection->Str2ws + pDiffSection->Len2ws;
		}
	}

	str1ws = pStr1->GetText() + pStr1->GetLength();
	str2ws = pStr2->GetText() + pStr2->GetLength();

	for (pDiffSection = DiffSections.Last();
		DiffSections.NotEnd(pDiffSection); pDiffSection = pDiffSection->Prev())
	{
		if (! pDiffSection->Different)
		{
			// find number of trailing blanks
			unsigned blanks1, blanks2;
			for (blanks1 = 0;
				blanks1 < pDiffSection->Len1ws
				&& ' ' == pDiffSection->Str1ws[pDiffSection->Len1ws - 1 - blanks1];
				blanks1++)
			{
			}
			for (blanks2 = 0;
				blanks2 < pDiffSection->Len2ws
				&& ' ' == pDiffSection->Str2ws[pDiffSection->Len2ws - 1 - blanks2];
				blanks2++)
			{
			}

			while (blanks1 < blanks2
					&& pDiffSection->Str1ws + pDiffSection->Len1ws < str1ws
					&& ' ' == pDiffSection->Str1ws[pDiffSection->Len1ws])
			{
				blanks1++;
				pDiffSection->Len1ws++;
			}
			while (blanks2 < blanks1
					&& pDiffSection->Str2ws + pDiffSection->Len2ws < str2ws
					&& ' ' == pDiffSection->Str2ws[pDiffSection->Len2ws])
			{
				blanks2++;
				pDiffSection->Len2ws++;
			}

			while (pDiffSection->Str1ws + pDiffSection->Len1ws < str1ws
					&& ' ' == pDiffSection->Str1ws[pDiffSection->Len1ws]
					&& pDiffSection->Str2ws + pDiffSection->Len2ws < str2ws
					&& ' ' == pDiffSection->Str2ws[pDiffSection->Len2ws])
			{
				pDiffSection->Len1ws++;
				pDiffSection->Len2ws++;
			}
		}

		if (NULL != pDiffSection->Str1ws)
		{
			str1ws = pDiffSection->Str1ws;
		}
		if (NULL != pDiffSection->Str2ws)
		{
			str2ws = pDiffSection->Str2ws;
		}
	}

	// then add the remaining blanks
	for (pDiffSection = DiffSections.First();
		DiffSections.NotEnd(pDiffSection); pDiffSection = DiffSections.Next(pDiffSection))
	{
		if (! pDiffSection->Different)
		{
			while (pDiffSection->Str1ws > str1ws
					&& ' ' == pDiffSection->Str1ws[-1])
			{
				pDiffSection->Str1ws--;
				pDiffSection->Len1ws++;
			}
			while (pDiffSection->Str2ws > str2ws
					&& ' ' == pDiffSection->Str2ws[-1])
			{
				pDiffSection->Str2ws--;
				pDiffSection->Len2ws++;
			}
		}
		if (NULL != pDiffSection->Str1ws)
		{
			str1ws = pDiffSection->Str1ws + pDiffSection->Len1ws;
		}
		if (NULL != pDiffSection->Str2ws)
		{
			str2ws = pDiffSection->Str2ws + pDiffSection->Len2ws;
		}
	}

	str1ws = pStr1->GetText() + pStr1->GetLength();
	str2ws = pStr2->GetText() + pStr2->GetLength();

	for (pDiffSection = DiffSections.Last();
		DiffSections.NotEnd(pDiffSection); pDiffSection = pDiffSection->Prev())
	{
		if (! pDiffSection->Different)
		{
			while (pDiffSection->Str1ws + pDiffSection->Len1ws < str1ws
					&& ' ' == pDiffSection->Str1ws[pDiffSection->Len1ws])
			{
				pDiffSection->Len1ws++;
			}
			while (pDiffSection->Str2ws + pDiffSection->Len2ws < str2ws
					&& ' ' == pDiffSection->Str2ws[pDiffSection->Len2ws])
			{
				pDiffSection->Len2ws++;
			}
		}
		if (NULL != pDiffSection->Str1ws)
		{
			str1ws = pDiffSection->Str1ws;
		}
		if (NULL != pDiffSection->Str2ws)
		{
			str2ws = pDiffSection->Str2ws;
		}
	}

	// now we have all identical sections expanded to fill all the whitespaces around them
	// merge all adjacent identical sections
	for (pDiffSection = DiffSections.First();
		DiffSections.NotEnd(pDiffSection); pDiffSection = pDiffSection->Next())
	{
		StringDiffSection * pNextDiff = pDiffSection->Next();
		if (! pDiffSection->Different
			&& DiffSections.NotEnd(pNextDiff)
			&& ! pNextDiff->Different
			&& pDiffSection->Str1ws + pDiffSection->Len1ws == pNextDiff->Str1ws
			&& pDiffSection->Str2ws + pDiffSection->Len2ws == pNextDiff->Str2ws)
		{
			pDiffSection->Len1ws += pNextDiff->Len1ws;
			pDiffSection->Len2ws += pNextDiff->Len2ws;
			pNextDiff->RemoveFromList();
			delete pNextDiff;
		}
	}

	int nDifferentChars = 0;

	// now generate the resulting difference sections
	for (pDiffSection = DiffSections.First();
		DiffSections.NotEnd(pDiffSection); pDiffSection = pDiffSection->Next())
	{
		StringSection * pNewSection;
		if (! pDiffSection->Different)
		{
			// any difference could only be whitespaces
			str1 = pDiffSection->Str1ws;
			str2 = pDiffSection->Str2ws;
			unsigned len1 = pDiffSection->Len1ws;
			unsigned len2 = pDiffSection->Len2ws;
			while (0 != len1 || 0 != len2)
			{
				unsigned idx = 0;
				while (idx < len1
						&& idx < len2
						&& str1[idx] == str2[idx])
				{
					idx++;
				}
				if (0 != idx)
				{
					// equal section found
					if (NULL != ppSections)
					{
						pNewSection = new StringSection;
						if (NULL != pNewSection)
						{
							pNewSection->Attr = StringSection::Identical;
							pNewSection->Length = (USHORT)idx;
							pNewSection->pBegin = str1;
							ppSections->InsertTail(pNewSection);
						}
					}
					str1 += idx;
					str2 += idx;
					len1 -= idx;
					len2 -= idx;
				}
				// skip the spaces
				ASSERT(0 == len1 || ' ' != *str1 || 0 == len2 || ' ' != *str2);

				unsigned idx1, idx2;
				for (idx1 = 0; idx1 < len1 && ' ' == str1[idx1]; idx1++)
				{
				}
				for (idx2 = 0; idx2 < len2 && ' ' == str2[idx2]; idx2++)
				{
				}

				ASSERT(0 == idx1 || 0 == idx2);
				if (0 != idx1)
				{
					// insert erased whitespace section
					if (NULL != ppSections)
					{
						pNewSection = new StringSection;
						if (NULL != pNewSection)
						{
							pNewSection->Attr = StringSection::File1Only
												| StringSection::Undefined
												| StringSection::Whitespace;
							pNewSection->Length = (USHORT)idx1;
							pNewSection->pBegin = str1;
							ppSections->InsertTail(pNewSection);
						}
					}
					str1 += idx1;
					len1 -= idx1;
				}
				else if (0 != idx2)
				{
					// insert added whitespace section
					if (NULL != ppSections)
					{
						pNewSection = new StringSection;
						if (NULL != pNewSection)
						{
							pNewSection->Attr = StringSection::File2Only
												| StringSection::Undefined
												| StringSection::Whitespace;
							pNewSection->Length = (USHORT)idx2;
							pNewSection->pBegin = str2;
							ppSections->InsertTail(pNewSection);
						}
					}
					str2 += idx2;
					len2 -= idx2;
				}
			}

		}
		else
		{
			// difference section
			if (0 != pDiffSection->Len1ws)
			{
				if (NULL != ppSections)
				{
					// insert erased section
					pNewSection = new StringSection;
					if (NULL != pNewSection)
					{
						pNewSection->Attr = StringSection::File1Only
											| StringSection::Undefined;
						pNewSection->Length = (USHORT)pDiffSection->Len1ws;
						pNewSection->pBegin = pDiffSection->Str1ws;

						if (pDiffSection->Str1ws > str1VersionBegin
							&& pDiffSection->Str1ws + pDiffSection->Len1ws <= str1VersionEnd)
						{
							pNewSection->Attr |= StringSection::VersionInfo;
						}
						ppSections->InsertTail(pNewSection);
					}
				}
			}
			if (0 != pDiffSection->Len2ws)
			{
				if (NULL != ppSections)
				{
					// insert added section
					pNewSection = new StringSection;
					if (NULL != pNewSection)
					{
						pNewSection->Attr = StringSection::File2Only
											| StringSection::Undefined;
						pNewSection->Length = (USHORT)pDiffSection->Len2ws;
						pNewSection->pBegin = pDiffSection->Str2ws;
						if (pDiffSection->Str2ws > str2VersionBegin
							&& pDiffSection->Str2ws + pDiffSection->Len2ws <= str2VersionEnd)
						{
							pNewSection->Attr |= StringSection::VersionInfo;
						}
						ppSections->InsertTail(pNewSection);
					}
				}
			}
			if (pDiffSection->Len1 > pDiffSection->Len2)
			{
				nDifferentChars += pDiffSection->Len1;
			}
			else
			{
				nDifferentChars += pDiffSection->Len2;
			}
		}
	}

	while ( ! DiffSections.IsEmpty())
	{
		delete DiffSections.RemoveHead();
	}
	return nDifferentChars;
}

bool LooksLike(const FileLine * pLine1, const FileLine * pLine2, int PercentsDifferent)
{
	unsigned nCharsDifferent = MatchStrings(pLine1, pLine2, NULL, GetApp()->m_MinMatchingChars);
	// nCharsDifferent won't count whitespace difference

	unsigned nLength = pLine1->GetNormalizedLength();
	if (nLength < pLine2->GetNormalizedLength())
	{
		nLength = pLine2->GetNormalizedLength();
	}
	return (nLength * PercentsDifferent / 100) >= nCharsDifferent;
}

CSmallAllocator FileLine::m_Allocator(sizeof FileLine);
CSmallAllocator LinePair::m_Allocator(sizeof LinePair);
