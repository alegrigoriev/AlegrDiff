// FileListSupport.h
#if !defined(AFX_FILELISTSUPPORT_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_)
#define AFX_FILELISTSUPPORT_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_
#pragma once
#include <afxtempl.h>
#include "SmallAllocator.h"

struct TextToken
{
	int m_Offset;
	int m_Len;
	DWORD m_Hash;
	class FileLine * m_pLine;
};

class FileLine
{
public:

	FileLine(const char * src, bool MakeNormalizedString);
	~FileLine();

#if 1
	static void * operator new(size_t size)
	{
		return m_Allocator.Allocate(size);
	}
	static void operator delete(void * ptr)
	{
		m_Allocator.Free(ptr);
	}
#endif
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
	bool IsBlank() const { return 0 != (m_Flags & BlankString); }

	bool GetNextToken(TextToken & token);

	void SetLink(FileLine * pLine) { m_Link = pLine; }
	FileLine * GetLink() const { return m_Link; }

	int GetLineNumber() const { return m_Number; }
	void SetLineNumber(int num) { m_Number = num; }

	LPCSTR GetText() const { return m_pString; }
	int GetLength() const { return m_Length; }

	LPCSTR GetNormalizedText() const { return m_pNormalizedString; }
	int GetNormalizedLength() const { return m_NormalizedStringLength; }

	static int _cdecl HashCompareFunc(const void * p1, const void * p2);
	static int _cdecl HashAndLineNumberCompareFunc(const void * p1, const void * p2);
	static int _cdecl NormalizedHashAndLineNumberCompareFunc(const void * p1, const void * p2);

	static int _cdecl GroupHashAndLineNumberCompareFunc(const void * p1, const void * p2);
	static int _cdecl NormalizedGroupHashAndLineNumberCompareFunc(const void * p1, const void * p2);

private:
	DWORD m_HashCode;
	DWORD m_GroupHashCode;
	DWORD m_NormalizedHashCode;
	DWORD m_NormalizedGroupHashCode;
	DWORD m_Flags;
	enum { HashValid = 1,
		BlankString = 2,
	};
	int m_Number; // line ordinal number in the file
	// length of the source string
	int m_Length;
	int m_NormalizedStringLength;
	int m_FirstTokenIndex;
	FileLine * m_Link;
	char * m_pAllocatedBuf;
	const char * m_pString;
	// points to the string with extra spaces removed
	const char * m_pNormalizedString;
	// String and normalized string share common buffer.
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
	LPCTSTR pBegin;
	USHORT Length;
	enum
	{
		Identical, Inserted, Erased,
	} Attr;
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

	FileLine * pFirstLine;
	FileLine * pSecondLine;
	StringSection * pFirstSection;
private:
	static CSmallAllocator m_Allocator;
};

struct FileSection
{
	FileSection * pNext;
	int File1LineBegin;
	int File1LineEnd;

	int File2LineBegin;
	int File2LineEnd;
};

class FileItem
{
public:
	FileItem(const CString & Name, const CString & BaseDir, const CString & Dir);
	FileItem(const WIN32_FIND_DATA * pWfd, const CString & BaseDir, const CString & Dir);
	~FileItem();
	bool Load();
	void Unload();
	const char * GetLineString(int LineNum) const;
	const FileLine * GetLine(int LineNum) const { return m_Lines[LineNum]; }
	int GetNumLines() const { return m_Lines.GetSize(); }

	LPCTSTR GetName() const { return m_Name; }
	LPCTSTR GetSubdir() const { return m_Subdir; }
	LPCTSTR GetBasedir() const { return m_BaseDir; }
	FILETIME GetLastWriteTime() const { return m_LastWriteTime; }
	const FileLine * FindMatchingLine(const FileLine * pLine, int nStartLineNum, int nEndLineNum);
	const FileLine * FindMatchingLineGroupLine(const FileLine * pLine, int nStartLineNum, int nEndLineNum);

	enum { MaxLineGroupSize = 32 };
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
	CArray<FileLine *, FileLine *> m_Lines;
	CArray<FileLine *, FileLine *> m_NonBlankLines;
	CArray<FileLine *, FileLine *> m_HashSortedLines;   // non-blank only
	CArray<FileLine *, FileLine *> m_HashSortedLineGroups;   // non-blank only
	CArray<FileLine *, FileLine *> m_NormalizedHashSortedLines;   // non-blank only
	CArray<FileLine *, FileLine *> m_NormalizedHashSortedLineGroups;   // non-blank only
	CArray<TextToken, TextToken> m_Tokens;
	friend class FilePair;
};

class FilePair
{
public:
	FilePair();
	~FilePair();

	FilePair * pNext;
	FileItem * pFirstFile;
	FileItem * pSecondFile;
	CString GetComparisionResult();
	static int _cdecl NameSortFunc(const void * p1, const void * p2);
	static int _cdecl NameSortBackwardsFunc(const void * p1, const void * p2);
	static int _cdecl DirNameSortFunc(const void * p1, const void * p2);
	static int _cdecl DirNameSortBackwardsFunc(const void * p1, const void * p2);
	static int _cdecl Time1SortFunc(const void * p1, const void * p2);
	static int _cdecl Time1SortBackwardsFunc(const void * p1, const void * p2);
	static int _cdecl Time2SortFunc(const void * p1, const void * p2);
	static int _cdecl Time2SortBackwardsFunc(const void * p1, const void * p2);
	static int _cdecl ComparisionSortFunc(const void * p1, const void * p2);
	static int _cdecl ComparisionSortBackwardsFunc(const void * p1, const void * p2);

	static int NameCompare(FilePair * Pair1, FilePair * Pair2);
	static int DirNameCompare(FilePair * Pair1, FilePair * Pair2);

	bool LoadFiles();
	void UnloadFiles();
	DWORD CompareFiles(bool bCompareAll = false, bool bUnload = true);
	DWORD CompareTextFiles(bool bCompareAll);
	DWORD CompareTextFilesNoExtraSpaces(bool bCompareAll);

	DWORD ComparisionResult;
	CArray<LinePair *, LinePair *> m_LinePairs;
};

class FileList
{
public:
	FileList();
	~FileList() { FreeFileList(); }
	bool LoadFolder(const CString & BaseDir, bool bRecurseSubdirs,
					LPCTSTR sInclusionMask, LPCTSTR sExclusionMask);
	bool LoadSubFolder(const CString & Subdir, bool bRecurseSubdirs,
						LPCTSTR sInclusionMask, LPCTSTR sExclusionMask);
	void FreeFileList();
	enum { SortNameFirst = 1, SortDirFirst = 2, SortDataModified = 4, SortBackwards = 8};
	void GetSortedList(CArray<FileItem *, FileItem *> & ItemArray, DWORD SortFlags);
	FileItem * m_pList;
	CString m_BaseDir;
	int m_NumFiles;
};

bool MatchWildcard(LPCTSTR name, LPCTSTR pattern);
bool MultiPatternMatches(LPCTSTR name, LPCTSTR sPattern);
void MiltiSzToCString(CString & str, LPCTSTR pMsz);
#endif
