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
private:
	mutable DWORD m_HashCode;
	mutable DWORD m_Flags;
	enum { HashValid = 1,
	};
	int m_Number; // ordinal number
	int m_Length;
	int m_FirstTokenIndex;

	// allocated with the buffer, must be the last member
	char m_Data[1];
};

class FileItem
{
public:
	FileItem(const CString & Name, const CString & BaseDir, const CString & Dir);
	FileItem(const WIN32_FIND_DATA * pWfd, const CString & BaseDir, const CString & Dir);
	~FileItem();
	bool Load();
	void Unload();
	const char * GetLine(int LineNum) const;
	LPCTSTR GetName() const { return m_Name; }
	LPCTSTR GetSubdir() const { return m_Subdir; }
	LPCTSTR GetBasedir() const { return m_BaseDir; }
	FILETIME GetLastWriteTime() const { return m_LastWriteTime; }
	FileItem * m_pNext;
	static int _cdecl FileItemNameSortFunc(const void * p1, const void * p2);
	static int _cdecl FileItemDirNameSortFunc(const void * p1, const void * p2);
	static int _cdecl FileItemTimeSortFunc(const void * p1, const void * p2);
	static int _cdecl FileItemNameSortBackwardsFunc(const void * p1, const void * p2);
	static int _cdecl FileItemDirNameSortBackwardsFunc(const void * p1, const void * p2);
	static int _cdecl FileItemTimeSortBackwardsFunc(const void * p1, const void * p2);

	static int FileItemNameCompare(FileItem * Item1, FileItem * Item2);
	static int FileItemDirNameCompare(FileItem * Item1, FileItem * Item2);
	static int FileItemTimeCompare(FileItem * Item1, FileItem * Item2);
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
