// FileListSupport.h
#if !defined(AFX_FILELISTSUPPORT_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_)
#define AFX_FILELISTSUPPORT_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_
#pragma once
#include <afxtempl.h>

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

	FileLine(const char * src, size_t length, int OrdNum);

	void * operator new (size_t structLen, size_t addLen)
	{
		return new char[structLen + addLen];
	}
	void operator delete(void * ptr, size_t)
	{
		::delete ptr;
	}
public:
	DWORD GetHash() const { return m_HashCode; }
	bool IsEqual(const FileLine & OtherLine) const;
	bool GetNextToken(TextToken & token);
	void SetLink(FileLine * pLine) { m_Link = pLine; }
	FileLine * GetLink() const { return m_Link; }

private:
	mutable DWORD m_HashCode;
	mutable DWORD m_Flags;
	enum { HashValid = 1,
	};
	int m_Number; // ordinal number
	int m_Length;
	int m_FirstTokenIndex;
	FileLine * m_Link;
	// allocated with the buffer, must be the last member
	char m_Data[1];
};

struct FileSection
{
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
	CArray<TextToken, TextToken> m_Tokens;
};

struct FilePair
{
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

	DWORD CompareFiles(bool bCompareAll = false);
	DWORD CompareTextFiles(bool bCompareAll);
	DWORD CompareTextFilesNoExtraSpaces(bool bCompareAll);

	DWORD ComparisionResult;
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
