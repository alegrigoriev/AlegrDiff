// FileListSupport.h
#if !defined(AFX_FILELISTSUPPORT_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_)
#define AFX_FILELISTSUPPORT_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_
#pragma once
#include <afxtempl.h>
#include <functional>
#include <vector>
#include "Md5HashCalculator.h"
#include "KListEntry.h"
#include "SmallAllocator.h"

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
	int pos;
	int scope; // 0 - combined file, 1 - left pane, 2 - right pane
	TextPosDisplay() {}
	TextPosDisplay(int l, int p, int s)
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
	static class CSmallAllocator m_Allocator;
};

enum FileCheckResult { FileDeleted, FileUnchanged, FileTimeChanged, };

class FileItem
{
public:
	FileItem(const WIN32_FIND_DATA * pWfd,
			const CString & BaseDir, const CString & Dir, FileItem * pParentDir);

	FileItem(LPCTSTR name);

	~FileItem();
	bool Load();
	void Unload();

	bool m_bMd5Calculated;
	bool m_bHasExtendedCharacters;
	bool m_bIsAlone;      // this item is inside directory existing on one side only

	enum FileEncoding
	{
		FileEncodingBinary = 0,
		FileEncodingASCII = 1, // all read characters are under 0x7F
		FileEncodingMBCS  = 2, // all read characters are under 0x7F
		FileEncodingUTF16BE = 3,    // big-endian UTF-16
		FileEncodingUTF16LE = 4,    // low-endian UTF-16
		FileEncodingUTF8 = 5,
		FileEncodingUnknown = 0xFF
	} m_FileEncoding;

	enum eFileType
	{
		FileTypeUnknown,
		FileTypeDirectory,
		FileTypeHashOnly,   // This file item represents a line from MD5 hash file
		FileTypeBinary,
		FileTypeText,
		FileTypeCCpp,
	} m_FileType;

	BOOL CalculateHashes(CMd5HashCalculator * pMd5Calc, class CProgressDialog * pProgressDialog);
	static size_t GetDigestLength() { return 16; }

	void SetMD5(BYTE const md5[16]);

	// add line from memory. Assuming the file created dynamically by the program
	void AddLine(LPCTSTR pLine);
	bool IsFolder() const
	{
		return m_FileType == FileTypeDirectory;
	}

	bool IsUnicode() const
	{
		return m_FileEncoding == FileEncodingUTF8 || m_FileEncoding == FileEncodingUTF16LE || m_FileEncoding == FileEncodingUTF16BE;
	}
	bool IsText() const
	{
		return m_FileType == FileTypeText || m_FileType == FileTypeCCpp;
	}
	void SetText()
	{
		m_FileType = FileTypeText;
	}

	bool IsCCpp() const
	{
		return m_FileType == FileTypeCCpp;
	}
	void SetCCpp()
	{
		m_FileType = FileTypeCCpp;
	}

	bool HasExtendedCharacters() const
	{
		return m_bHasExtendedCharacters;
	}

	bool IsBinary() const
	{
		return m_FileType == FileTypeBinary;
	}
	void SetBinary()
	{
		m_FileType = FileTypeBinary;
		m_FileEncoding = FileEncodingBinary;
	}

	bool IsPhantomFile() const
	{
		return m_FileType == FileTypeHashOnly;
	}

	bool IsAlone() const
	{
		return m_bIsAlone;
	}

	void SetAlone(bool alone)
	{
		m_bIsAlone = alone;
	}

	bool IsReparsePoint() const
	{
		return 0 != (m_Attributes & FILE_ATTRIBUTE_REPARSE_POINT);
	}

	bool IsSymbolicLink() const
	{//FIXME
		return 0 != (m_Attributes & FILE_ATTRIBUTE_REPARSE_POINT);
	}

	bool IsHardLink() const
	{//FIXME
		return 0 != (m_Attributes & FILE_ATTRIBUTE_REPARSE_POINT);
	}

	unsigned GetFileData(LONGLONG FileOffset, void * pBuf, unsigned bytes);
	void FreeReadBuffer();

	LPCTSTR GetLineString(int LineNum) const;
	const FileLine * GetLine(int LineNum) const
	{
		return m_Lines[LineNum];
	}
	unsigned GetNumLines() const
	{
		return (unsigned)m_Lines.size();
	}

	// get file name ONLY
	LPCTSTR GetName() const
	{
		return m_Name;
	}
	unsigned GetNameLength() const
	{
		return (unsigned)m_Name.GetLength();
	}

	// get sibdirectory ONLY, with trailing slash
	LPCTSTR GetSubdir() const
	{
		return m_Subdir;
	}
	int GetSubdirLength() const
	{
		return m_Subdir.GetLength();
	}

	LPCTSTR GetBasedir() const
	{
		return m_BaseDir;
	}
	int GetBasedirLength() const
	{
		return m_BaseDir.GetLength();
	}

	CString GetFullName() const
	{
		return m_BaseDir + m_Subdir + m_Name;
	}
	int GetFullNameLength() const
	{
		return m_BaseDir.GetLength() + m_Subdir.GetLength() + m_Name.GetLength();
	}

	FILETIME GetLastWriteTime() const
	{
		return m_LastWriteTime;
	}

	LONGLONG GetFileLength() const
	{
		return m_Length;
	}
	UINT GetDigest(int idx) const
	{
		return m_Md5[idx];
	}
	BYTE const * GetDigest() const
	{
		return m_Md5;
	}

	const FileLine * FindMatchingLine(const FileLine * pLine,
									unsigned nStartLineNum, unsigned nEndLineNum);
	const FileLine * FindMatchingLineGroupLine(const FileLine * pLine,
												unsigned nStartLineNum, unsigned nEndLineNum);

	enum { MaxLineGroupSize = 50 };


	FileCheckResult CheckForFileChanged();
	FileCheckResult ReloadIfChanged();

	FileItem * m_pNext;
	FileItem* m_pParentDir;
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

	static void Lock()
	{
		m_Cs.Lock();
	}

	static void Unlock()
	{
		m_Cs.Unlock();
	}

private:
	CString m_Name;
	CString m_Subdir;
	CString m_BaseDir;

	FILETIME m_LastWriteTime;
	LONGLONG m_Length;
	LONGLONG m_Crc64;   // use x64 + x4 + x3 + x + 1 polynomial
	BYTE m_Md5[16];

	BYTE * m_pFileReadBuf;
	ULONG m_FileReadBufSize;
	LONGLONG m_FileReadPos;
	DWORD m_FileReadFilled;
	DWORD m_Attributes;
	HANDLE m_hFile;

	vector<FileLine *> m_Lines;
	vector<FileLine *> m_NonBlankLines;
	vector<FileLine *> m_NormalizedHashSortedLines;   // non-blank only
	vector<FileLine *> m_NormalizedHashSortedLineGroups;   // non-blank only

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
		return (pFirstFile != NULL && pFirstFile->IsBinary())
			|| (pSecondFile != NULL && pSecondFile->IsBinary());
	}

	bool FilesAreDifferent() const
	{
		return m_ComparisonResult == FilesDifferent
				|| m_ComparisonResult == DifferentInSpaces
				|| m_ComparisonResult == VersionInfoDifferent
				|| m_ComparisonResult == FirstFileLonger
				|| m_ComparisonResult == SecondFileLonger;
	}
	bool FilesAreIdentical() const
	{
		return m_ComparisonResult == FilesIdentical;
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
		FileFromSubdirInFirstDirOnly,   // file from a subdirectory that exists only in first directory
		FileFromSubdirInSecondDirOnly,   // file from a subdirectory that exists only in first directory
		SubdirsParentInFirstDirOnly,   // dir from a subdirectory that exists only in first directory
		SubdirsParentInSecondDirOnly,   // dir from a subdirectory that exists only in first directory
		FilesDirectoryInFingerprintFileOnly,
	};

	int ComparisionResultPriority() const;

	eFileComparisionResult CompareFiles(class CProgressDialog * pProgressDialog);
	eFileComparisionResult CompareTextFiles(class CProgressDialog * pProgressDialog);
	eFileComparisionResult CompareBinaryFiles(class CProgressDialog * pProgressDialog);
	struct FileSection
	{
		FileSection * pNext;
		unsigned File1LineBegin;
		unsigned File1LineEnd;

		unsigned File2LineBegin;
		unsigned File2LineEnd;
	};

	void SetComparisonResult(eFileComparisionResult result)
	{
		m_ComparisonResult = result;
	}
	CString GetComparisonResultStr() const;
	eFileComparisionResult GetComparisonResult() const
	{
		return m_ComparisonResult;
	}
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

	std::vector<struct LinePair *> m_LinePairs;
	std::vector<FileDiffSection *> m_DiffSections;
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
	~FileList();
	void Detach()
	{
		m_pList = NULL;
		m_NumFiles = 0;
	}

	bool LoadFolder(const CString & BaseDir, bool bRecurseSubdirs,
					LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
					LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask, LPCTSTR sIgnoreDirs);
	bool LoadSubFolder(LPCTSTR Subdir,
						LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
						LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask, LPCTSTR sIgnoreDirs, FileItem * pParentDir);

	void FreeFileList();

	enum { SortNameFirst = 1, SortDirFirst = 2, SortDataModified = 4, SortBackwards = 8};
	void GetSortedList(vector<FileItem *> & ItemArray, DWORD SortFlags);

	FileItem * m_pList;
	CString m_BaseDir;
	int m_NumFiles;
};

bool MatchWildcard(LPCTSTR name, LPCTSTR pattern);
// Empty pattern never matches
bool MultiPatternMatches(LPCTSTR name, LPCTSTR sPattern);
CString MiltiSzToCString(LPCTSTR pMsz);
CString PatternToMultiCString(LPCTSTR src);



#endif
