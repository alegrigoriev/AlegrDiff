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
#include "AdelsonVelskyLandisTree.h"
#include "PathEx.h"
#include "FileLine.h"

#define FILE_OPEN_OVERHEAD 0x2000

struct TextPos
{
	int line;
	int pos;
	TextPos() noexcept
		: line(0), pos(0) {}
	TextPos(int l, int p) noexcept
		: line(l), pos(p)
	{
	}
};

struct TextPosLine
{
	int line;
	int pos;
	TextPosLine() noexcept
		: line(0), pos(0) {}
	TextPosLine(int l, int p) noexcept
		: line(l), pos(p)
	{
	}
};

struct TextPosDisplay
{
	int line;
	int pos;
	eFileScope scope; // 0 - combined file, 1 - left pane, 2 - right pane
	TextPosDisplay() noexcept
		: line(0), pos(0), scope(eFileScope::Both) {}
	TextPosDisplay(int l, int p, eFileScope s) noexcept
		: line(l), pos(p), scope(s)
	{
	}
};

constexpr inline bool operator >(const TextPos & p1, const TextPos & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos > p2.pos);
}

constexpr inline bool operator >=(const TextPos & p1, const TextPos & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos >= p2.pos);
}

constexpr inline bool operator <(const TextPos & p1, const TextPos & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos < p2.pos);
}

constexpr inline bool operator <=(const TextPos & p1, const TextPos & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos <= p2.pos);
}

constexpr inline bool operator ==(const TextPos & p1, const TextPos & p2)
{
	return p1.line == p2.line && p1.pos == p2.pos;
}

constexpr inline bool operator !=(const TextPos & p1, const TextPos & p2)
{
	return p1.line != p2.line || p1.pos != p2.pos;
}

constexpr inline bool operator >(const TextPosLine & p1, const TextPosLine & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos > p2.pos);
}

constexpr inline bool operator >=(const TextPosLine & p1, const TextPosLine & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos >= p2.pos);
}

constexpr inline bool operator <(const TextPosLine & p1, const TextPosLine & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos < p2.pos);
}

constexpr inline bool operator <=(const TextPosLine & p1, const TextPosLine & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos <= p2.pos);
}

constexpr inline bool operator ==(const TextPosLine & p1, const TextPosLine & p2)
{
	return p1.line == p2.line && p1.pos == p2.pos;
}

constexpr inline bool operator !=(const TextPosLine & p1, const TextPosLine & p2)
{
	return p1.line != p2.line || p1.pos != p2.pos;
}

constexpr inline bool operator >(const TextPosDisplay & p1, const TextPosDisplay & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos > p2.pos);
}

constexpr inline bool operator >=(const TextPosDisplay & p1, const TextPosDisplay & p2)
{
	return (p1.line > p2.line)
		|| (p1.line == p2.line && p1.pos >= p2.pos);
}

constexpr inline bool operator <(const TextPosDisplay & p1, const TextPosDisplay & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos < p2.pos);
}

constexpr inline bool operator <=(const TextPosDisplay & p1, const TextPosDisplay & p2)
{
	return (p1.line < p2.line)
		|| (p1.line == p2.line && p1.pos <= p2.pos);
}

constexpr inline bool operator ==(const TextPosDisplay & p1, const TextPosDisplay & p2)
{
	return p1.line == p2.line && p1.pos == p2.pos;
}

constexpr inline bool operator !=(const TextPosDisplay & p1, const TextPosDisplay & p2)
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
	bool IsAccepted() const noexcept { return 0 != (Attr & Accepted); }
	bool IsIncluded() const noexcept { return 0 != (Attr & Included); }
	bool IsFile1Only() const noexcept { return 0 != (Attr & File1Only); }
	bool IsDeclined() const noexcept { return 0 != (Attr & Declined); }
	bool IsDiscarded() const noexcept { return 0 != (Attr & Discarded); }
	bool IsWhitespace() const noexcept { return 0 != (Attr & Whitespace); }
private:
	static class CSmallAllocator m_Allocator;
};

class FileItem;
enum FileCheckResult { FileDeleted, FileUnchanged, FileTimeChanged, };
struct KeyDirectoryEntry
{
	ULONG RefCount;     // number of entries referring to this key
	ULONG SortSequence; // index in sorted sequence
	KeyDirectoryEntry() noexcept
		: RefCount(0), SortSequence(0xFFFFFFFF)
	{}
};
struct FileItemListEntry
{
	FileItem * pItem;
	FileItemListEntry(FileItem * item) noexcept
		: pItem(item)
	{}
};

struct FilenameComparePredicate
{
	int operator()(CString const & A, CString const & B) const noexcept
	{
		return A.CollateNoCase(B);
	}
};

struct MultiStrDirComparePredicate
{
	int operator()(LPCTSTR pA, LPCTSTR pB) const;  // The strings are multi-strings
};

struct FullPathnameComparePredicate
{
	int operator()(FileItem const* A, FileItem const* B) const;
};

typedef avl_tree<KeyDirectoryEntry, CString, FilenameComparePredicate> name_tree_t;
typedef avl_tree<KeyDirectoryEntry, CString, MultiStrDirComparePredicate> full_dirname_tree_t;  // CString is multi-string
typedef avl_tree<FileItem *, FileItem const*, FullPathnameComparePredicate> file_item_tree_t;

class FileItem
{
public:
	FileItem(const WIN32_FIND_DATA * pWfd,
			const CString & BaseDir, const CString & Dir, OPTIONAL FileItem * pParentDir);

	~FileItem();
	bool Load();
	void Unload() noexcept;

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
	static size_t GetDigestLength() noexcept { return 16; }

	void SetMD5(BYTE const md5[16]);
	void CopyMD5(FileItem *pFileItem) noexcept;

	// add line from memory. Assuming the file created dynamically by the program
	void AddLine(LPCTSTR pLine);
	bool IsFolder() const noexcept
	{
		return m_FileType == FileTypeDirectory;
	}

	bool HasContents() const noexcept
	{
		// Return true if refers to a real file
		return this != nullptr && !IsFolder() && !IsPhantomFile();
	}

	bool IsUnicode() const noexcept
	{
		return m_FileEncoding == FileEncodingUTF8 || m_FileEncoding == FileEncodingUTF16LE || m_FileEncoding == FileEncodingUTF16BE;
	}
	bool IsText() const noexcept
	{
		return m_FileType == FileTypeText || m_FileType == FileTypeCCpp;
	}
	void SetText() noexcept
	{
		m_FileType = FileTypeText;
	}

	bool IsCCpp() const noexcept
	{
		return m_FileType == FileTypeCCpp;
	}
	void SetCCpp() noexcept
	{
		m_FileType = FileTypeCCpp;
	}

	bool HasExtendedCharacters() const noexcept
	{
		return m_bHasExtendedCharacters;
	}

	bool IsBinary() const noexcept
	{
		return m_FileType == FileTypeBinary;
	}
	void SetBinary() noexcept
	{
		m_FileType = FileTypeBinary;
		m_FileEncoding = FileEncodingBinary;
	}

	bool IsPhantomFile() const noexcept
	{
		return m_FileType == FileTypeHashOnly;
	}

	bool IsFolderAlone() const noexcept
	{
		return this != nullptr && m_bIsAlone;
	}

	void SetFolderAlone(bool alone) noexcept
	{
		m_bIsAlone = alone;
	}

	bool IsReparsePoint() const noexcept
	{
		return 0 != (m_Attributes & FILE_ATTRIBUTE_REPARSE_POINT);
	}

	bool IsSymbolicLink() const noexcept
	{//FIXME
		return 0 != (m_Attributes & FILE_ATTRIBUTE_REPARSE_POINT);
	}

	bool IsHardLink() const noexcept
	{//FIXME
		return 0 != (m_Attributes & FILE_ATTRIBUTE_REPARSE_POINT);
	}

	unsigned GetFileData(LONGLONG FileOffset, void * pBuf, unsigned bytes);

	const FileLine * GetLine(int LineNum) const noexcept
	{
		return m_Lines[LineNum];
	}
	unsigned GetNumLines() const noexcept
	{
		return (unsigned)m_Lines.size();
	}

	// get file name ONLY. Empty for a directory
	CString const & GetFileName() const noexcept
	{
		if (IsFolder())
		{
			static CString empty;
			return empty;
		}
		return m_Name;
	}
	// get file (or subdirectory) name
	CString const & GetName() const noexcept
	{
		return m_Name;
	}
	unsigned GetNameLength() const noexcept
	{
		return (unsigned)m_Name.GetLength();
	}

	// get sibdirectory ONLY, with trailing slash. For a directory does NOT includes its own name
	CString const & GetSubdir() const noexcept
	{
		return m_Subdir;
	}
	int GetSubdirLength() const noexcept
	{
		return m_Subdir.GetLength();
	}

	CString const & GetBasedir() const noexcept
	{
		return m_BaseDir;
	}
	int GetBasedirLength() const noexcept
	{
		return m_BaseDir.GetLength();
	}

	CString GetFullName() const
	{
		return m_BaseDir + m_Subdir + m_Name;
	}
	int GetFullNameLength() const noexcept
	{
		return m_BaseDir.GetLength() + m_Subdir.GetLength() + m_Name.GetLength();
	}

	CString const & GetMultiStrSubdir() const
	{
		ASSERT(IsFolder());
		return m_MultiStrDir;
	}

	ULONGLONG GetLastWriteTime() const noexcept
	{
		return m_LastWriteTime;
	}

	ULONGLONG GetCreationTime() const noexcept
	{
		return m_CreationTime;
	}

	LONGLONG GetFileLength() const noexcept
	{
		return m_Length;
	}
	UINT GetDigest(int idx) const noexcept
	{
		return m_Md5[idx];
	}
	BYTE const * GetDigest() const noexcept
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

	static bool FileTypesChanged(FileItem const* Item1, FileItem const* Item2) noexcept
	{
		// returns 'true' if only one of the files is not present,
		// or both present but file type changed.
		return (nullptr != Item1 || nullptr != Item2)
			&& (nullptr == Item1 || nullptr == Item2
				|| Item1->IsBinary() != Item2->IsBinary());
	}
	static bool FilesChanged(FileItem const* Item1, FileItem const* Item2) noexcept
	{
		// returns 'true' if either creation or modification time changed,
		// or filesystem ID changed.
		return Item1 != nullptr && Item2 != nullptr
				&& (Item1->GetLastWriteTime() != Item2->GetLastWriteTime()
					|| Item1->GetCreationTime() != Item2->GetCreationTime()
					|| Item1->GetFileLength() != Item2->GetFileLength());
	}

	// These functions return 1 if Item1 is greater than Item2, -1 if Item1 is less than Item2,
	// and zero if they are equal
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

	// These are indices in a pre-sorted list of m_Name values (collated no case), in the common dictionary for left and right trees, in ascending order.
	ULONG m_NameSortNum;        // zero for directories
	ULONG m_FullDirSortNum;     // full directory names have a separate dictionary, because the sorting predicate is different
	name_tree_t::iterator iNameInTree;  // NULL for directories
	full_dirname_tree_t::iterator iFullDirInTree;

private:
	// This data is used to facilitate sorting and UI.
	// This is a filename or directory name (last element only)
	// For a directory, it has a trailing backslash
	// Used also as a sort key (case-insensitive collate)
	CString m_Name;

	// base directory is common for all items in the list. Ends with backslash.
	// For items built from a fingerprint file, it's FP filename
	CString m_BaseDir;

	// full containing subdirectory name (NOT including the comparison base directory). If not empty, ends with backslash
	// Will be equal for left and right directories (case-insensitive)
	// Is used for UI.
	CString m_Subdir;

	// full sub directory presented as multi-string (includes current directory name).
	// Is set for directory items only and used as the sort key.
	CString m_MultiStrDir;

	ULONGLONG m_LastWriteTime;
	ULONGLONG m_CreationTime;
	LONGLONG m_Length;
	LONGLONG m_Crc64;   // use x64 + x4 + x3 + x + 1 polynomial TODO
	BYTE m_Md5[16];

	BYTE * m_pFileReadBuf;
	ULONG m_FileReadBufSize;
	LONGLONG m_FileReadPos;
	DWORD m_FileReadFilled;
	DWORD m_Attributes;
	HANDLE m_hFile;

	std::vector<FileLine *> m_Lines;
	std::vector<FileLine *> m_NonBlankLines;
	std::vector<FileLine *> m_NormalizedHashSortedLines;   // non-blank only
	std::vector<FileLine *> m_NormalizedHashSortedLineGroups;   // non-blank only

	friend class BinaryFilePair;
	friend class TextFilePair;
	static CSimpleCriticalSection m_Cs;
};

enum PairCheckResult { FilesDeleted, FilesUnchanged, FilesTimeChanged, };

class FilePair : public ListItem<FilePair>
{
public:
	FilePair(FileItem* file1, FileItem* file2);
	void Reference();
	void Dereference() noexcept;
private:
protected:
	virtual ~FilePair();
	int m_RefCount;
	int m_LoadedCount;

public:
	FileItem * pFirstFile;
	FileItem * pSecondFile;

	// UnloadFiles returns 'true' if load count drops to zero, or ForceUnload speecified.
	// If the function returns 'false', the files are still loaded
	bool CloseFiles(bool ForceUnload = false) noexcept;
	CString GetTitle() const;

	virtual bool NeedBinaryComparison() const noexcept = 0;

	bool HasContents() const noexcept
	{
		// At least one FileItem refers to a real file
		return pFirstFile->HasContents()
				|| pSecondFile->HasContents();
	}

	bool CanCompare() const noexcept
	{
		// Both FileItem refers to a real file
		return this != nullptr
				&& pFirstFile->HasContents()
				&& pSecondFile->HasContents();
	}

	bool FilesAreDifferent() const noexcept
	{
		return m_ComparisonResult == FilesDifferent
				|| m_ComparisonResult == DifferentInSpaces
				|| m_ComparisonResult == VersionInfoDifferent
				|| m_ComparisonResult == FirstFileLonger
				|| m_ComparisonResult == SecondFileLonger;
	}
	bool FilesAreIdentical() const noexcept
	{
		return m_ComparisonResult == FilesIdentical || m_ComparisonResult == FilesAttributesIdentical;
	}

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
		FilesAttributesIdentical,

		DirectoriesBothPresent, // this directory is present in both left and right
	};

	int ComparisionResultPriority() const;

	virtual eFileComparisionResult CompareFiles(class CProgressDialog * pProgressDialog) = 0;

	void SetComparisonResult(eFileComparisionResult result) noexcept
	{
		m_ComparisonResult = result;
	}
	CString GetComparisonResultStr() const;
	LPCTSTR GetComparisonResultStr(TCHAR s[], size_t buf_size) const;
	eFileComparisionResult GetComparisonResult() const noexcept
	{
		return m_ComparisonResult;
	}

	virtual eFileComparisionResult PreCompareFiles(CMd5HashCalculator * pMd5Calc, class CProgressDialog * pProgressDialog) = 0;

	eFileComparisionResult m_ComparisonResult;
	bool m_bChanged;
	bool m_bHideFromListView;
	bool m_bSelected;
	bool m_bDeleted;
	bool m_bFocused;

	// used to speed up file list sort
	ULONG m_FilenameSortOrder;
	ULONG m_DirectorySortOrder;
	ULONG m_ListSortOrder;
};

class TextFilePair : public FilePair
{
public:
	TextFilePair(FileItem *file1, FileItem* file2);
private:
	virtual ~TextFilePair();

public:

	bool LoadFiles();
	bool UnloadFiles(bool ForceUnload = false) noexcept;
	void FreeLinePairData();

	bool NeedBinaryComparison() const noexcept override
	{
		return false;
	}

	PairCheckResult CheckForFilesChanged();
	PairCheckResult ReloadIfChanged();

	bool NextDifference(TextPosDisplay PosFrom, BOOL IgnoreWhitespaces,
						TextPosDisplay* DiffPos, TextPosDisplay* EndPos);
	bool PrevDifference(TextPosDisplay PosFrom, BOOL IgnoreWhitespaces,
						TextPosDisplay* DiffPos, TextPosDisplay* EndPos);
	TextPosLine DisplayPosToLinePos(TextPosDisplay position, BOOL IgnoreWhitespaces);
	TextPosDisplay LinePosToDisplayPos(TextPosLine position, BOOL IgnoreWhitespaces, eFileScope FileScope);

	int GetAcceptDeclineFlags(TextPosLine PosFrom, TextPosLine PosTo, bool bIgnoreWhitespaces);
	BOOL ModifyAcceptDeclineFlags(TextPosLine& PosFrom, TextPosLine& PosTo, int Set, int Reset);

	BOOL EnumStringDiffSections(TextPosLine& PosFrom, TextPosLine& PosTo,
								void(*Func)(StringSection* pSection, void* Param), void* pParam);
	static void GetAcceptDeclineFlagsFunc(StringSection* pSection, void* Param);
	static void ModifyAcceptDeclineFlagsFunc(StringSection* pSection, void* Param);

	eFileComparisionResult CompareFiles(class CProgressDialog* pProgressDialog);
	eFileComparisionResult CompareTextFiles(class CProgressDialog* pProgressDialog);

	struct FileSection
	{
		FileSection* pNext;
		unsigned File1LineBegin;
		unsigned File1LineEnd;

		unsigned File2LineBegin;
		unsigned File2LineEnd;
	};

	FileSection* BuildSectionList(int NumLine1Begin, int NumLine1AfterEnd,
								int NumLine2Begin, int NumLine2AfterEnd, bool UseLineGroups);

	eFileComparisionResult PreCompareFiles(CMd5HashCalculator*, class CProgressDialog* pProgressDialog);
	eFileComparisionResult PreCompareFiles(class CProgressDialog* pProgressDialog);

	std::vector<struct LinePair*> m_LinePairs;
	std::vector<FileDiffSection*> m_DiffSections;
};

class BinaryFilePair : public FilePair
{
public:
	BinaryFilePair(FileItem* file1, FileItem* file2);
private:
	~BinaryFilePair();

public:

	bool NeedBinaryComparison() const noexcept override
	{
		return true;
	}

	eFileComparisionResult CompareFiles(class CProgressDialog* pProgressDialog);

	eFileComparisionResult PreCompareFiles(CMd5HashCalculator* pMd5Calc, class CProgressDialog* pProgressDialog);
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

	static int NoOp(const FilePair *, const FilePair *) noexcept
	{
		return 0;
	}
};

enum eLoadFolderResult
{
	eLoadFolderResultSuccess = 0,
	eLoadFolderResultFailure = ERROR_GEN_FAILURE,
	eLoadFolderResultNotFound = ERROR_FILE_NOT_FOUND | 0xC0000000,
	eLoadFolderResultAccessDenied = ERROR_ACCESS_DENIED | 0xC0000000,
	eLoadFolderResultSubdirAccessDenied = ERROR_ACCESS_DENIED,   // non-fatal
	eLoadFolderResultSubdirReadError = ERROR_READ_FAULT,   // non-fatal
};

class FileList
{
public:
	FileList();
	~FileList();
	FileItem *Detach() noexcept
	{
		FileItem * item = m_pList;
		m_pList = NULL;
		m_NumFiles = 0;
		return item;
	}

	eLoadFolderResult LoadFolder(LPCTSTR BaseDir, bool bRecurseSubdirs,
								LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
								LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask, LPCTSTR sIgnoreDirs);
	eLoadFolderResult LoadFingerprintFile(LPCTSTR Filename, bool &bRecurseSubdirs,
										CString & sInclusionMask, CString & sExclusionMask,
										CString & sIgnoreDirs);

	void FreeFileList();

	enum { SortNameFirst = 1, SortDirFirst = 2, SortDataModified = 4, SortBackwards = 8 };

	FileItem * m_pList;
	CPathEx m_BaseDir;
protected:
	int m_NumFiles;
	eLoadFolderResult LoadSubFolder(LPCTSTR Subdir,
									LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
									LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask, LPCTSTR sIgnoreDirs, FileItem * pParentDir);
};


class FilePairList : ListHead<FilePair>
{
public:
	FilePairList() noexcept
		: NumFilePairs(0)
	{}
	enum FileListIndex
	{
		LeftFileList = 1,
		RightFileList = 2,
	};

	bool BuildFilePairList(OPTIONAL FileList *List1, FileList *List2, bool DoNotCompareContents);  // returns 'true' if there were changes in it

	unsigned NumFilePairs;
	bool HasFiles() const noexcept;
	ULONGLONG GetTotalDataSize(ULONG FileOpenOverhead = FILE_OPEN_OVERHEAD);
	void RemovePair(FilePair * pPairToDelete) noexcept;
	void RemoveAll() noexcept;

	using ListHead<FilePair>::First;
	using ListHead<FilePair>::Next;
	using ListHead<FilePair>::Last;
	using ListHead<FilePair>::NotEnd;

private:

	/*
	* The pair list uses sorted directories (dictionary) of all filenames and full directory names
	* for faster sorting of full path lists
	* First, the file lists are added to the dictionaries by calling AddToDictionary.
	* Then the nodes in the dictionaries are numbered by callind ::UpdateFileItemTreeNumbering
	* Before you delete a file list, call RemoveFromDictionary for all its items.
	*/
	void AddToDictionary(FileList const *list);
	void RemoveFromDictionary(FileItem* pItem) noexcept;
	void RemoveFromDictionary(FilePair* pPair) noexcept;
	/*
	* FileListToTree replaces the tree with the contents of the supplied FileList
	* Old contents of the tree is removed from dictionaries.
	*/
	void FileListToTree(FileList *list, file_item_tree_t &Tree);

	/*
	* NameTree contains directory of all filenames in the first and second directory,
	* including filenames of directories. Each item contains a count how many times the name appears.
	*/
	name_tree_t NameTree;
	/*
	* FullDirNameTree contains directory of all full directory names in the first and second directory.
	* Each item contains a count how many times the name appears.
	* Use AddToDictionary to add a list to the dictionary.
	* Use RemoveFromDictionary to remove a list from the dictionary
	*/
	full_dirname_tree_t FullDirNameTree;

	file_item_tree_t Files1;
	file_item_tree_t Files2;
};

bool MatchWildcard(LPCTSTR name, LPCTSTR pattern);
// Empty pattern never matches
bool MultiPatternMatches(LPCTSTR name, LPCTSTR sPattern);
CString PatternToMultiCString(LPCTSTR src);



#endif
