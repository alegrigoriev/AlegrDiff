// FileListSupport.h
#if !defined(AFX_FILELISTSUPPORT_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_)
#define AFX_FILELISTSUPPORT_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_
#pragma once
#include <afxtempl.h>
#include "SmallAllocator.h"
#include <functional>
#include <vector>
#include <Wincrypt.h>

using namespace std;

struct TextPos
{
	int line;
	int pos;
	TextPos() {}
	TextPos(int l, int p)
	{
		line = l;
		pos = p;
	}
};

inline bool operator >(const TextPos & p1, const TextPos & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos > p2.pos);
}

inline bool operator >=(const TextPos & p1, const TextPos & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos >= p2.pos);
}

inline bool operator <(const TextPos & p1, const TextPos & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos < p2.pos);
}

inline bool operator <=(const TextPos & p1, const TextPos & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos <= p2.pos);
}

inline bool operator ==(const TextPos & p1, const TextPos & p2)
{
	return p1.line == p2.line && p1.pos == p2.pos;
}

inline bool operator !=(const TextPos & p1, const TextPos & p2)
{
	return p1.line != p2.line || p1.pos != p2.pos;
}

struct TextToken
{
	int m_Offset;
	int m_Len;
	DWORD m_Hash;
	class FileLine * m_pLine;
};

class FileDiffSection
{
public:
	FileDiffSection() { m_Flags = 0; }
	~FileDiffSection() {}
	TextPos m_Begin;
	TextPos m_End;
	ULONG m_Flags;
	enum {
		FlagWhitespace = 0x100,
	};

	static void * operator new(size_t size)
	{
		return m_Allocator.Allocate(size);
	}
	static void operator delete(void * ptr)
	{
		m_Allocator.Free(ptr);
	}
private:
	static CSmallAllocator m_Allocator;
};

bool less<FileDiffSection *>::operator()
	(FileDiffSection * const & pS1, FileDiffSection * const & pS2) const
{
	return pS1->m_Begin < pS2->m_Begin;
}

class FileLine
{
public:

	FileLine(LPCTSTR src, bool MakeNormalizedString, bool c_cpp_file);
	~FileLine();

	static void * operator new(size_t size)
	{
		return m_Allocator.Allocate(size);
	}
	static void operator delete(void * ptr)
	{
		m_Allocator.Free(ptr);
	}

public:
	DWORD GetHash() const { return m_HashCode; }
	DWORD GetNormalizedHash() const { return m_NormalizedHashCode; }

	DWORD GetGroupHash() const { return m_GroupHashCode; }
	DWORD GetNormalizedGroupHash() const { return m_NormalizedGroupHashCode; }

	void SetGroupHash(DWORD hash) { m_GroupHashCode = hash; }
	void SetNormalizedGroupHash(DWORD hash) { m_NormalizedGroupHashCode = hash; }

	bool IsEqual(const FileLine * pOtherLine) const;
	bool IsNormalizedEqual(const FileLine * pOtherLine) const;
	bool LooksLike(const FileLine * pOtherLine, int PercentsDifferent) const;
	bool IsBlank() const { return 0 == m_NormalizedStringLength; }

	bool IsExtraWhitespace(unsigned pos) const
	{
		return 0 != (m_pWhitespaceMask[pos / 8] & (1 << (pos & 7)));
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
	static int _cdecl NormalizedHashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2);

	static int _cdecl GroupHashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2);
	static int _cdecl NormalizedGroupHashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2);

private:
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
	char * m_pAllocatedBuf;
	const char * m_pWhitespaceMask;
	LPCTSTR m_pString;
	// points to the string with extra spaces removed
	LPCTSTR m_pNormalizedString;
	// String, normalized string and whitespace mask share common buffer.
	// you only need to delete m_pAllocatedBuf
	static CSmallAllocator m_Allocator;
};

struct StringSection
{
	static void * operator new(size_t size)
	{
		return m_Allocator.Allocate(size);
	}
	static void operator delete(void * ptr)
	{
		m_Allocator.Free(ptr);
	}
	StringSection * pNext;
	const class FileDiffSection * pDiffSection;
	LPCTSTR pBegin;
	USHORT Length;
	enum
	{
		Identical = 0,
		Inserted = 1,
		File2Only = 1,
		Erased = 2,
		File1Only = 2,
		Accepted = 4,
		UseFile2Only = 4,
		Declined = 8,
		UseFile1Only = 8,
		Undefined = 0x10,
		NoDifference = 0x20,
		Included = 0x40,
		Discarded = 0x80,
		Whitespace = 0x100,
	};
	USHORT Attr;
	bool IsAccepted() const { return 0 != (Attr & Accepted); }
	bool IsIncluded() const { return 0 != (Attr & Included); }
	bool IsFile1Only() const { return 0 != (Attr & File1Only); }
	bool IsDeclined() const { return 0 != (Attr & Declined); }
	bool IsDiscarded() const { return 0 != (Attr & Discarded); }
	bool IsWhitespace() const { return 0 != (Attr & Whitespace); }
private:
	static CSmallAllocator m_Allocator;
};

struct LinePair
{
	static void * operator new(size_t size)
	{
		return m_Allocator.Allocate(size);
	}
	static void operator delete(void * ptr)
	{
		m_Allocator.Free(ptr);
	}

	const FileLine * pFirstLine;
	const FileLine * pSecondLine;
	StringSection * pFirstSection;
private:
	static CSmallAllocator m_Allocator;
public:
	// recalculates offset in the raw line to offset in the line with or without whitespaces shown
	int LinePosToDisplayPos(int position, BOOL bIgnoreWhitespaces);
	// recalculates offset in the line with or without whitespaces shown to offset in the raw line
	int DisplayPosToLinePos(int position, BOOL bIgnoreWhitespaces);
	LPCTSTR GetText(LPTSTR buf, size_t nBufChars, int * pStrLen, BOOL IgnoreWhitespaces);
};

enum FileCheckResult { FileDeleted, FileUnchanged, FileTimeChanged, };

class FileItem
{
public:
	FileItem(const WIN32_FIND_DATA * pWfd,
			const CString & BaseDir, const CString & Dir);
	FileItem(LPCTSTR name);
	~FileItem();
	bool Load();
	void Unload();
	static BOOL InitHashCalculation();
	static void DeinitHashCalculation();
	BOOL CalculateHashes(BOOL volatile & bStopOperation);

	bool m_C_Cpp:1;
	bool m_IsBinary:1;
	bool m_IsUnicode:1;
	bool m_IsUnicodeBigEndian:1;
	bool m_bMd5Calculated:1;

	// add line from memory. Assuming the file created dynamically by the program
	void AddLine(LPCTSTR pLine);
	LPCTSTR GetLineString(int LineNum) const;
	const FileLine * GetLine(int LineNum) const { return m_Lines[LineNum]; }
	int GetNumLines() const { return m_Lines.size(); }

	LPCTSTR GetName() const { return m_Name; }
	int GetNameLength() const { return m_Name.GetLength(); }

	LPCTSTR GetSubdir() const { return m_Subdir; }
	int GetSubdirLength() const { return m_Subdir.GetLength(); }

	LPCTSTR GetBasedir() const { return m_BaseDir; }
	int GetBasedirLength() const { return m_BaseDir.GetLength(); }

	CString GetFullName() const { return m_BaseDir + m_Subdir + m_Name; }
	int GetFullNameLength() const { return m_BaseDir.GetLength() + m_Subdir.GetLength() + m_Name.GetLength(); }

	FILETIME GetLastWriteTime() const { return m_LastWriteTime; }
	const FileLine * FindMatchingLine(const FileLine * pLine,
									unsigned nStartLineNum, unsigned nEndLineNum);
	const FileLine * FindMatchingLineGroupLine(const FileLine * pLine,
												unsigned nStartLineNum, unsigned nEndLineNum);

	enum { MaxLineGroupSize = 50 };


	FileCheckResult CheckForFileChanged();
	FileCheckResult ReloadIfChanged();

	FileItem * m_pNext;
	static int _cdecl NameSortFunc(const void * p1, const void * p2);
	static int _cdecl DirNameSortFunc(const void * p1, const void * p2);
	static int _cdecl TimeSortFunc(const void * p1, const void * p2);
	static int _cdecl NameSortBackwardsFunc(const void * p1, const void * p2);
	static int _cdecl DirNameSortBackwardsFunc(const void * p1, const void * p2);
	static int _cdecl TimeSortBackwardsFunc(const void * p1, const void * p2);

	static int NameCompare(FileItem * Item1, FileItem * Item2);
	static int DirNameCompare(FileItem * Item1, FileItem * Item2);
	static int TimeCompare(FileItem * Item1, FileItem * Item2);
private:
	CString m_Name;
	CString m_Subdir;
	CString m_BaseDir;
	FILETIME m_LastWriteTime;
	LONGLONG m_Length;
	LONGLONG m_Crc64;   // use x64 + x4 + x3 + x + 1 polynomial
	BYTE m_Md5[20];

	vector<FileLine *> m_Lines;
	vector<FileLine *> m_NonBlankLines;
	vector<FileLine *> m_HashSortedLines;   // non-blank only
	vector<FileLine *> m_HashSortedLineGroups;   // non-blank only
	vector<FileLine *> m_NormalizedHashSortedLines;   // non-blank only
	vector<FileLine *> m_NormalizedHashSortedLineGroups;   // non-blank only
	//vector<TextToken> m_Tokens;
	friend class FilePair;
	//static CSimpleCriticalSection m_Cs;
	static BYTE * m_HashBuf;
	static HCRYPTPROV m_CryptProvider;
};

enum PairCheckResult { FilesDeleted, FilesUnchanged, FilesTimeChanged, };

class FilePair
{
public:
	FilePair();
	void Reference();
	void Dereference();
private:
	~FilePair();
	int m_RefCount;
	int m_LoadedCount;

public:
	FilePair * pNext;
	FileItem * pFirstFile;
	FileItem * pSecondFile;
	CString GetComparisionResult() const;
	void SetMemoryFile()
	{
		m_LoadedCount = 1;
		m_ComparisionResult = MemoryFile;
	}
	enum CompareSubitem
	{
		CompareSubitemName,
		CompareSubitemDir,
		CompareSubitemDate1,
		CompareSubitemDate2,
		CompareSubitemResult,
	};
	struct CompareParam
	{
		CompareSubitem PrimarySort;
		bool PrimaryBackward;
		CompareSubitem SecondarySort;
		bool SecondaryBackward;
	};

	static bool Compare(const FilePair * Pair1, const FilePair * Pair2, CompareParam comp);

	static int Time1SortFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int Time1SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int Time2SortFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int Time2SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int ComparisionSortFunc(const FilePair * Pair1, const FilePair * Pair2);

	static int NameCompare(const FilePair * Pair1, const FilePair * Pair2);
	static int DirNameCompare(const FilePair * Pair1, const FilePair * Pair2);

	bool LoadFiles();
	void UnloadFiles(bool ForceUnload = false);
	void FreeLinePairData();


	PairCheckResult CheckForFilesChanged();
	PairCheckResult ReloadIfChanged();

	bool NextDifference(TextPos PosFrom, BOOL IgnoreWhitespaces,
						TextPos * DiffPos, TextPos * EndPos);
	bool PrevDifference(TextPos PosFrom, BOOL IgnoreWhitespaces,
						TextPos * DiffPos, TextPos * EndPos);

	int GetAcceptDeclineFlags(TextPos PosFrom, TextPos PosTo, bool bIgnoreWhitespaces);
	BOOL ModifyAcceptDeclineFlags(TextPos & PosFrom, TextPos & PosTo, int Set, int Reset);

	BOOL EnumStringDiffSections(TextPos & PosFrom, TextPos & PosTo,
								void (* Func)(StringSection * pSection, void * Param), void * pParam);
	static void GetAcceptDeclineFlagsFunc(StringSection * pSection, void * Param);
	static void ModifyAcceptDeclineFlagsFunc(StringSection * pSection, void * Param);

	enum eFileComparisionResult
	{
		ResultUnknown,
		FileUnaccessible,
		FilesIdentical,
		DifferentInSpaces,
		VersionInfoDifferent,
		FilesDifferent,
		OnlyFirstFile,
		OnlySecondFile,
		OnlyFirstDirectory,
		OnlySecondDirectory,
		FirstFileLonger,
		SecondFileLonger,
		ErrorReadingFirstFile,
		ErrorReadingSecondFile,
		ReadingFirstFile,
		ReadingSecondFile,
		CalculatingFirstFingerprint,
		CalculatingSecondFingerprint,
		ComparingFiles,
		MemoryFile,
	};

	int ComparisionResultPriority() const;

	eFileComparisionResult CompareFiles(BOOL volatile & bStopOperation);
	eFileComparisionResult CompareTextFiles(BOOL volatile & bStopOperation);
	eFileComparisionResult CompareBinaryFiles(BOOL volatile & bStopOperation);
	struct FileSection
	{
		FileSection * pNext;
		int File1LineBegin;
		int File1LineEnd;

		int File2LineBegin;
		int File2LineEnd;
	};

	FileSection * BuildSectionList(int NumLine1Begin, int NumLine1AfterEnd,
									int NumLine2Begin, int NumLine2AfterEnd, bool UseLineGroups);

	eFileComparisionResult PreCompareFiles(BOOL volatile & bStopOperation);
	eFileComparisionResult PreCompareTextFiles(BOOL volatile & bStopOperation);
	eFileComparisionResult PreCompareBinaryFiles(BOOL volatile & bStopOperation);

	int m_CompletedPercent;
	eFileComparisionResult m_ComparisionResult;
	bool m_bChanged;
	bool m_bHideFromListView;
	bool m_bSelected;

	vector<LinePair *> m_LinePairs;
	vector<FileDiffSection *> m_DiffSections;
};

class FileList
{
public:
	FileList();
	~FileList() { FreeFileList(); }
	void Detach()
	{
		m_pList = NULL;
		m_NumFiles = 0;
	}
	bool LoadFolder(const CString & BaseDir, bool bRecurseSubdirs,
					LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
					LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask);
	bool LoadSubFolder(const CString & Subdir, bool bRecurseSubdirs,
						LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
						LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask);

	void FreeFileList();
	enum { SortNameFirst = 1, SortDirFirst = 2, SortDataModified = 4, SortBackwards = 8};
	void GetSortedList(CArray<FileItem *, FileItem *> & ItemArray, DWORD SortFlags);
	FileItem * m_pList;
	CString m_BaseDir;
	int m_NumFiles;
};

bool MatchWildcard(LPCTSTR name, LPCTSTR pattern);
bool MultiPatternMatches(LPCTSTR name, LPCTSTR sPattern);
CString MiltiSzToCString(LPCTSTR pMsz);
CString PatternToMultiCString(LPCTSTR src);
int MatchStrings(LPCTSTR pStr1, LPCTSTR pStr2, StringSection ** ppSections, int nMinMatchingChars);
#endif
