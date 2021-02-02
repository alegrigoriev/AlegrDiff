#ifndef FILE_LINE_H_INCLUDED
#define FILE_LINE_H_INCLUDED
#include "KListEntry.h"
#include <vector>

class FileLine
{
public:
	enum { eContainsVersionInfo = 1, };

	FileLine(LPCTSTR src, size_t Length, bool MakeNormalizedString, bool c_cpp_file);
	~FileLine();

	static void * operator new(size_t size);
	static void operator delete(void * ptr);

public:
	DWORD GetHash() const noexcept { return m_HashCode; }
	DWORD GetNormalizedHash() const noexcept { return m_NormalizedHashCode; }

	DWORD GetGroupHash() const noexcept { return m_GroupHashCode; }
	DWORD GetNormalizedGroupHash() const noexcept { return m_NormalizedGroupHashCode; }

	void SetGroupHash(DWORD hash) noexcept { m_GroupHashCode = hash; }
	void SetNormalizedGroupHash(DWORD hash) noexcept { m_NormalizedGroupHashCode = hash; }

	void CalculateGroupHash(DWORD const src[], size_t size);
	void CalculateNormalizedGroupHash(DWORD const src[], size_t size);

	bool IsEqual(const FileLine * pOtherLine) const;
	bool IsNormalizedEqual(const FileLine * pOtherLine) const;
	bool IsBlank() const noexcept { return 0 == m_NormalizedStringLength; }

	bool IsExtraWhitespace(unsigned pos) const noexcept
	{
		return 0 != (m_pWhitespaceMask[pos / 8] & (1 << (pos & 7)));
	}

	bool ContainsVersionInfo() const noexcept
	{
		return 0 != (m_Flags & eContainsVersionInfo);
	}

	void SetNext(FileLine * pNext) noexcept { m_pNext = pNext; }
	FileLine * Next() const noexcept { return m_pNext; }

	unsigned GetLineNumber() const noexcept { return m_Number; }
	void SetLineNumber(unsigned num) noexcept { m_Number = num; }

	LPCTSTR GetText() const noexcept { return m_pString; }
	unsigned GetLength() const noexcept { return m_Length; }

	LPCTSTR GetNormalizedText() const noexcept { return m_pNormalizedString; }
	unsigned GetNormalizedLength() const noexcept { return m_NormalizedStringLength; }

	static int _cdecl HashCompareFunc(FileLine const * pLine1, FileLine const * pLine2);
	static int _cdecl HashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2);

	static int _cdecl GroupHashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2);

protected:
	int RemoveExtraWhitespaces(LPTSTR pDst, LPCTSTR Src, unsigned DstCount,
								char * pWhitespaceMask, int WhitespaceMaskSize,
								unsigned Flags);
private:

	DWORD m_Flags;
	DWORD m_HashCode;
	DWORD m_GroupHashCode;
	DWORD m_NormalizedHashCode;
	DWORD m_NormalizedGroupHashCode;
	unsigned m_Number; // line ordinal number in the file
	FileLine * m_pNext;
	// length of the source string
	unsigned m_Length;
	unsigned m_NormalizedStringLength;
	//int m_FirstTokenIndex;
	//FileLine * m_Link;
	TCHAR * m_pAllocatedBuf;
	const char * m_pWhitespaceMask;
	LPCTSTR m_pString;
	// points to the string with extra spaces removed
	LPCTSTR m_pNormalizedString;
	// String, normalized string and whitespace mask share common buffer.
	// you only need to delete m_pAllocatedBuf
	static class CSmallAllocator m_Allocator;
};

enum class eFileScope
{
	Both = 0,
	Left = 1,
	Right = 2,
};

struct LinePair
{
	static void * operator new(size_t size);
	static void operator delete(void * ptr);

	const FileLine * pFirstLine;
	const FileLine * pSecondLine;
	ListHead<struct StringSection> StrSections;
private:
	static class CSmallAllocator m_Allocator;
public:
	LinePair(const FileLine * pLine);   // from the same line
	LinePair(const FileLine * pLine1, const FileLine * pLine2, class FileDiffSection * pDiffSection = NULL);
	void BuildDiffSectionsList(int nLineIndex, std::vector<class FileDiffSection*> & DiffSections, int MinMatchingChars);
	// recalculates offset in the raw line to offset in the line with or without whitespaces shown
	int LinePosToDisplayPos(int position, BOOL bIgnoreWhitespaces, eFileScope FileScope);
	// recalculates offset in the line with or without whitespaces shown to offset in the raw line
	int DisplayPosToLinePos(int position, BOOL bIgnoreWhitespaces, eFileScope FileScope);
	LPCTSTR GetText(LPTSTR buf, size_t nBufChars, int * pStrLen, BOOL IgnoreWhitespaces, eFileScope SelectFile);
};

int MatchStrings(const FileLine * pStr1, const FileLine * pStr2, ListHead<StringSection> * ppSections, unsigned nMinMatchingChars);
bool LooksLike(const FileLine * pLine1, const FileLine * pLine2, int PercentsDifferent);

#endif  // FILE_LINE_H_INCLUDED
