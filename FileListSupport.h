// FileListSupport.h
#if !defined(AFX_FILELISTSUPPORT_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_)
#define AFX_FILELISTSUPPORT_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_
#pragma once
#include <afxtempl.h>
#include "SmallAllocator.h"
#include <functional>
#include <vector>
#include "Md5HashCalculator.h"
#include "KListEntry.h"

using namespace std;

#define FILE_OPEN_OVERHEAD 0x2000

struct TextPos
{
	int line;
	int pos;
	TextPos() {}
	TextPos(int l, int p)
		: line(l), pos(p)
	{
	}
};

struct TextPosLine
{
	int line;
	int pos;
	TextPosLine() {}
	TextPosLine(int l, int p)
		: line(l), pos(p)
	{
	}
};

struct TextPosDisplay
{
	int line;
	short pos;
	short scope;
	TextPosDisplay() {}
	TextPosDisplay(int l, short p, short s)
		: line(l), pos(p), scope(s)
	{
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

inline bool operator >(const TextPosLine & p1, const TextPosLine & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos > p2.pos);
}

inline bool operator >=(const TextPosLine & p1, const TextPosLine & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos >= p2.pos);
}

inline bool operator <(const TextPosLine & p1, const TextPosLine & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos < p2.pos);
}

inline bool operator <=(const TextPosLine & p1, const TextPosLine & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos <= p2.pos);
}

inline bool operator ==(const TextPosLine & p1, const TextPosLine & p2)
{
	return p1.line == p2.line && p1.pos == p2.pos;
}

inline bool operator !=(const TextPosLine & p1, const TextPosLine & p2)
{
	return p1.line != p2.line || p1.pos != p2.pos;
}

inline bool operator >(const TextPosDisplay & p1, const TextPosDisplay & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos > p2.pos);
}

inline bool operator >=(const TextPosDisplay & p1, const TextPosDisplay & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos >= p2.pos);
}

inline bool operator <(const TextPosDisplay & p1, const TextPosDisplay & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos < p2.pos);
}

inline bool operator <=(const TextPosDisplay & p1, const TextPosDisplay & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos <= p2.pos);
}

inline bool operator ==(const TextPosDisplay & p1, const TextPosDisplay & p2)
{
	return p1.line == p2.line && p1.pos == p2.pos;
}

inline bool operator !=(const TextPosDisplay & p1, const TextPosDisplay & p2)
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
	TextPosLine m_Begin;
	TextPosLine m_End;
	ULONG m_Flags;
	enum {
		FlagWhitespace = 0x100,
		FlagVersionInfoDifferent = 0x200,
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

struct StringSection : public ListItem<StringSection>
{
	static void * operator new(size_t size)
	{
		return m_Allocator.Allocate(size);
	}
	static void operator delete(void * ptr)
	{
		m_Allocator.Free(ptr);
	}

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
		VersionInfo = 0x200,
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
	ListHead<StringSection> StrSections;
private:
	static CSmallAllocator m_Allocator;
public:
	// recalculates offset in the raw line to offset in the line with or without whitespaces shown
	int LinePosToDisplayPos(int position, BOOL bIgnoreWhitespaces, int FileScope);
	// recalculates offset in the line with or without whitespaces shown to offset in the raw line
	int DisplayPosToLinePos(int position, BOOL bIgnoreWhitespaces, int FileScope);
	LPCTSTR GetText(LPTSTR buf, size_t nBufChars, int * pStrLen, BOOL IgnoreWhitespaces, int SelectFile);
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

	bool m_C_Cpp:1;
	bool m_IsBinary:1;
	bool m_IsUnicode:1;
	bool m_IsUnicodeBigEndian:1;
	bool m_bMd5Calculated:1;
	bool m_bIsFolder:1;
	bool m_bIsPhantomFile:1;
	bool m_bUnicodeName:1;

	BOOL CalculateHashes(CMd5HashCalculator * pMd5Calc,
						class CProgressDialog * pProgressDialog);
	static size_t GetDigestLength() { return 16; }

	void SetMD5(BYTE md5[16]);

	// add line from memory. Assuming the file created dynamically by the program
	void AddLine(LPCTSTR pLine);
	bool IsFolder() const { return m_bIsFolder; }

	size_t GetFileData(LONGLONG FileOffset, void * pBuf, size_t bytes);
	void FreeReadBuffer();

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

	LONGLONG GetFileLength() const { return m_Length; }
	UINT GetDigest(int idx) const { return m_Md5[idx]; }
	BYTE const * GetDigest() const { return m_Md5; }

	const FileLine * FindMatchingLine(const FileLine * pLine,
									unsigned nStartLineNum, unsigned nEndLineNum);
	const FileLine * FindMatchingLineGroupLine(const FileLine * pLine,
												unsigned nStartLineNum, unsigned nEndLineNum);

	enum { MaxLineGroupSize = 50 };


	FileCheckResult CheckForFileChanged();
	FileCheckResult ReloadIfChanged();

	FileItem * m_pNext;
	static bool NameSortFunc(FileItem const * Item1, FileItem const * Item2);
	static bool DirNameSortFunc(FileItem const * Item1, FileItem const * Item2);
	static bool TimeSortFunc(FileItem const * Item1, FileItem const * Item2);
	static bool NameSortBackwardsFunc(FileItem const * Item1, FileItem const * Item2);
	static bool DirNameSortBackwardsFunc(FileItem const * Item1, FileItem const * Item2);
	static bool TimeSortBackwardsFunc(FileItem const * Item1, FileItem const * Item2);

	static int NameCompare(FileItem const * Item1, FileItem const * Item2);
	static int DirNameCompare(FileItem const * Item1, FileItem const * Item2);
	static int TimeCompare(FileItem const * Item1, FileItem const * Item2);
	static int LengthCompare(FileItem const * Item1, FileItem const * Item2);

private:
	CString m_Name;
	CString m_Subdir;
	CString m_BaseDir;

	FILETIME m_LastWriteTime;
	LONGLONG m_Length;
	LONGLONG m_Crc64;   // use x64 + x4 + x3 + x + 1 polynomial
	BYTE m_Md5[16];

	BYTE * m_pFileReadBuf;
	size_t m_FileReadBufSize;
	LONGLONG m_FileReadPos;
	DWORD m_FileReadFilled;
	HANDLE m_hFile;

	vector<FileLine *> m_Lines;
	vector<FileLine *> m_NonBlankLines;
	vector<FileLine *> m_NormalizedHashSortedLines;   // non-blank only
	vector<FileLine *> m_NormalizedHashSortedLineGroups;   // non-blank only
	//vector<TextToken> m_Tokens;
	friend class FilePair;
	static CSimpleCriticalSection m_Cs;
};

enum PairCheckResult { FilesDeleted, FilesUnchanged, FilesTimeChanged, };

class FilePair : public ListItem<FilePair>
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
	FileItem * pFirstFile;
	FileItem * pSecondFile;
	CString GetComparisonResult() const;
	void SetMemoryFile()
	{
		m_LoadedCount = 1;
		m_ComparisonResult = MemoryFile;
	}

	bool LoadFiles();
	void UnloadFiles(bool ForceUnload = false);
	void FreeLinePairData();

	bool NeedBinaryComparison() const
	{
		return (pFirstFile != NULL && pFirstFile->m_IsBinary)
			|| (pSecondFile != NULL && pSecondFile->m_IsBinary);
	}

	PairCheckResult CheckForFilesChanged();
	PairCheckResult ReloadIfChanged();

	bool NextDifference(TextPosDisplay PosFrom, BOOL IgnoreWhitespaces,
						TextPosDisplay * DiffPos, TextPosDisplay * EndPos);
	bool PrevDifference(TextPosDisplay PosFrom, BOOL IgnoreWhitespaces,
						TextPosDisplay * DiffPos, TextPosDisplay * EndPos);
	TextPosLine DisplayPosToLinePos(TextPosDisplay position, BOOL IgnoreWhitespaces);
	TextPosDisplay LinePosToDisplayPos(TextPosLine position, BOOL IgnoreWhitespaces, int FileScope);

	int GetAcceptDeclineFlags(TextPosLine PosFrom, TextPosLine PosTo, bool bIgnoreWhitespaces);
	BOOL ModifyAcceptDeclineFlags(TextPosLine & PosFrom, TextPosLine & PosTo, int Set, int Reset);

	BOOL EnumStringDiffSections(TextPosLine & PosFrom, TextPosLine & PosTo,
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
		FileInFingerprintFileOnly,
		OnlySecondFile,
		OnlyFirstDirectory,
		DirectoryInFingerprintFileOnly,
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

	eFileComparisionResult CompareFiles(class CProgressDialog * pProgressDialog);
	eFileComparisionResult CompareTextFiles(class CProgressDialog * pProgressDialog);
	eFileComparisionResult CompareBinaryFiles(class CProgressDialog * pProgressDialog);
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

	eFileComparisionResult PreCompareFiles(CMd5HashCalculator * pMd5Calc, class CProgressDialog * pProgressDialog);
	eFileComparisionResult PreCompareTextFiles(class CProgressDialog * pProgressDialog);
	eFileComparisionResult PreCompareBinaryFiles(CMd5HashCalculator * pMd5Calc, class CProgressDialog * pProgressDialog);

	eFileComparisionResult m_ComparisonResult;
	bool m_bChanged;
	bool m_bHideFromListView;
	bool m_bSelected;
	bool m_bDeleted;
	bool m_bFocused;

	// used to speed up file list sort
	ULONG m_FilenameSortOrder;
	ULONG m_DirectorySortOrder;

	vector<LinePair *> m_LinePairs;
	vector<FileDiffSection *> m_DiffSections;
};

struct FilePairComparePredicate
{
	bool operator ()(const FilePair * Pair1, const FilePair * Pair2);
	FilePairComparePredicate(enum eColumns Sort[], bool Ascending[], int SortNumber);
private:
	typedef int (*CompareFunc)(const FilePair * Pair1, const FilePair * Pair2);
	CompareFunc Functions[6];

	static int Time1SortFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int Time1SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int Time2SortFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int Time2SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int Length1SortFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int Length1SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int Length2SortFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int Length2SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int ComparisionSortFunc(const FilePair * Pair1, const FilePair * Pair2);
	static int ComparisionSortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2);

	static int NameCompare(const FilePair * Pair1, const FilePair * Pair2);
	static int DirNameCompare(const FilePair * Pair1, const FilePair * Pair2);
	static int NameCompareBackward(const FilePair * Pair1, const FilePair * Pair2);
	static int DirNameCompareBackward(const FilePair * Pair1, const FilePair * Pair2);

	static int NoOp(const FilePair * , const FilePair * )
	{
		return 0;
	}
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
					LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask, LPCTSTR sIgnoreDirs);
	bool LoadSubFolder(const CString & Subdir, bool bRecurseSubdirs,
						LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
						LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask, LPCTSTR sIgnoreDirs);

	void FreeFileList();
	enum { SortNameFirst = 1, SortDirFirst = 2, SortDataModified = 4, SortBackwards = 8};
	void GetSortedList(vector<FileItem *> & ItemArray, DWORD SortFlags);
	FileItem * m_pList;
	CString m_BaseDir;
	int m_NumFiles;
};

bool MatchWildcard(LPCTSTR name, LPCTSTR pattern);
bool MultiPatternMatches(LPCTSTR name, LPCTSTR sPattern);
CString MiltiSzToCString(LPCTSTR pMsz);
CString PatternToMultiCString(LPCTSTR src);
int MatchStrings(LPCTSTR pStr1, LPCTSTR pStr2, ListHead<StringSection> * ppSections, int nMinMatchingChars);
#endif
