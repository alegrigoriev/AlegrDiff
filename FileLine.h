#ifndef FILE_LINE_H_INCLUDED
#define FILE_LINE_H_INCLUDED

class FileLine
{
public:
	enum { eContainsVersionInfo = 1, };

	FileLine(LPCTSTR src, int Length, bool MakeNormalizedString, bool c_cpp_file);
	~FileLine();

	static void * operator new(size_t size);
	static void operator delete(void * ptr);

public:
	DWORD GetHash() const { return m_HashCode; }
	DWORD GetNormalizedHash() const { return m_NormalizedHashCode; }

	DWORD GetGroupHash() const { return m_GroupHashCode; }
	DWORD GetNormalizedGroupHash() const { return m_NormalizedGroupHashCode; }

	void SetGroupHash(DWORD hash) { m_GroupHashCode = hash; }
	void SetNormalizedGroupHash(DWORD hash) { m_NormalizedGroupHashCode = hash; }

	void CalculateGroupHash(DWORD const src[], size_t size);
	void CalculateNormalizedGroupHash(DWORD const src[], size_t size);

	bool IsEqual(const FileLine * pOtherLine) const;
	bool IsNormalizedEqual(const FileLine * pOtherLine) const;
	bool IsBlank() const { return 0 == m_NormalizedStringLength; }

	bool IsExtraWhitespace(unsigned pos) const
	{
		return 0 != (m_pWhitespaceMask[pos / 8] & (1 << (pos & 7)));
	}

	bool ContainsVersionInfo() const
	{
		return 0 != (m_Flags & eContainsVersionInfo);
	}

	void SetNext(FileLine * pNext) { m_pNext = pNext; }
	FileLine * Next() const { return m_pNext; }

	unsigned GetLineNumber() const { return m_Number; }
	void SetLineNumber(unsigned num) { m_Number = num; }

	LPCTSTR GetText() const { return m_pString; }
	unsigned GetLength() const { return m_Length; }

	LPCTSTR GetNormalizedText() const { return m_pNormalizedString; }
	unsigned GetNormalizedLength() const { return m_NormalizedStringLength; }

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

#endif  // FILE_LINE_H_INCLUDED
