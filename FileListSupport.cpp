// FileListSupport.cpp
#include "stdafx.h"
#include "FileListSupport.h"
#include "AlegrDiff.h"

#ifdef _DEBUG
#include <mmsystem.h>
#endif
#include <algorithm>

#undef tolower
#undef toupper
static DWORD CalculateHash(void const * data, int len);

CString PatternToMultiCString(LPCTSTR src)
{
	// all ';', ',' are replaced with 0, another 0 is appended
	// if string without * or ?, then * is prepended and appended
	// empty string is removed
	// spaces before and after are removed
	CString tmp(src);
	CString dst;

	bool Wildcard = false;
	int nBeginIndex = 0;
	for (int i = 0; ; i++)
	{
		if (tmp.GetLength() == i
			|| ',' == tmp[i]
			|| ';' == tmp[i])
		{
			// if previous
			// trim spaces
			while (i < tmp.GetLength()
					&& ' ' == tmp[nBeginIndex])
			{
				nBeginIndex++;
			}
			int nEndIndex = i;
			while (nEndIndex > nBeginIndex
					&& ' ' == tmp[nEndIndex - 1])
			{
				nEndIndex--;
			}

			if (nBeginIndex != nEndIndex)
			{
				CString s = tmp.Mid(nBeginIndex, nEndIndex - nBeginIndex);
				if ( ! Wildcard)
				{
					s.Insert(0, _T("*"));
					s += _T("*");
				}
				int BufLen = dst.GetLength() + s.GetLength() + 1;
				LPTSTR pBuf = dst.GetBuffer(BufLen);
				if (NULL != pBuf)
				{
					memcpy(pBuf + dst.GetLength(), LPCTSTR(s), (s.GetLength() + 1) * sizeof pBuf[0]);
					dst.ReleaseBuffer(BufLen);
				}
			}
			Wildcard = false;
			nBeginIndex = i + 1;
		}
		else if ('*' == tmp[i]
				|| '?' == tmp[i])
		{
			Wildcard = true;
		}

		if (0 == src[i])
		{
			break;
		}
	}
	return dst;
}

CString MiltiSzToCString(LPCTSTR pMsz)
{
	CString str;
	// find string length
	int len;
	// limit the string length to 64K
	for (len = 0; ('\0' != pMsz[len] || '\0' != pMsz[len + 1]) && len < 65536; len++)
	{
	}
	len += 2;
	LPTSTR pBuf = str.GetBuffer(len);
	if (NULL != pBuf)
	{
		memcpy(pBuf, pMsz, len * sizeof (TCHAR));
		str.ReleaseBuffer(len - 1);
	}
	return str;
}
bool MatchWildcard(LPCTSTR name, LPCTSTR pattern)
{
	// '?' corresponds to any character or no character,
	// '*' corresponds to any (or none) number of characters
	while (1)
	{
		switch (*pattern)
		{
		case 0:
			return 0 == name[0];
			break;
		case '?':
			pattern++;
			if (name[0] != 0
				&& MatchWildcard(name + 1, pattern))
			{
				return true;
			}
			continue;
			break;
		case '*':
			while ('*' == *pattern
					|| '?' == *pattern)
			{
				pattern++;
			}
			if (0 == *pattern)
			{
				// last character is '*'
				return true;
			}
			for( ; 0 != name[0]; name++)
			{
				if (MatchWildcard(name, pattern))
				{
					return true;
				}
			}
			return false;
			break;
		default:
			if (CSTR_EQUAL != CompareString(LOCALE_USER_DEFAULT,
											NORM_IGNORECASE, pattern, 1, name, 1))
			{
				return false;
			}
			name++;
			pattern++;
			continue;
			break;
		}
	}
}

bool MultiPatternMatches(LPCTSTR name, LPCTSTR sPattern)
{
	// sPattern is MULTI_SZ
	while ('\0' != sPattern[0])
	{
		if (MatchWildcard(name, sPattern))
		{
			return true;
		}
		// skip this wildcard
		while ('\0' != *(sPattern++))
		{
		}
	}
	return false;
}

FileList::FileList()
	: m_pList(NULL), m_NumFiles(0)
{
}

FileItem::FileItem(const WIN32_FIND_DATA * pWfd,
					const CString & BaseDir, const CString & Dir)
	:m_Name(pWfd->cFileName),
	m_Length(pWfd->nFileSizeLow + (LONGLONG(pWfd->nFileSizeHigh) << 32)),
	m_BaseDir(BaseDir),
	m_Subdir(Dir),
	m_C_Cpp(false),
	m_IsBinary(false),
	m_IsUnicode(false),
	m_IsUnicodeBigEndian(false),
	m_bMd5Calculated(false),
	m_pNext(NULL)
{
	memzero(m_Md5);
	m_LastWriteTime = pWfd->ftLastWriteTime;
}

FileItem::FileItem(LPCTSTR name)
	:m_Name(name),
	m_Length(0),
	m_C_Cpp(false),
	m_IsBinary(false),
	m_IsUnicode(false),
	m_IsUnicodeBigEndian(false),
	m_bMd5Calculated(false),
	m_pNext(NULL)
{
	memzero(m_Md5);
	m_LastWriteTime.dwHighDateTime = 0;
	m_LastWriteTime.dwLowDateTime = 0;
}

void FileItem::Unload()
{
	TRACE("FileItem %s Unloaded\n", LPCTSTR(GetFullName()));
	for (unsigned i = 0; i < m_Lines.size(); i++)
	{
		delete m_Lines[i];
	}
	m_Lines.clear();
	m_NonBlankLines.clear();
	m_HashSortedLines.clear();
	m_HashSortedLineGroups.clear();
	m_NormalizedHashSortedLines.clear();
	m_NormalizedHashSortedLineGroups.clear();
}

FileItem::~FileItem()
{
	Unload();
}

void FileItem::AddLine(LPCTSTR pLine)
{
	FileLine * pFileLine = new FileLine(pLine, true, m_C_Cpp);
	if (pLine)
	{
		pFileLine->SetLineNumber(m_Lines.size());
		m_Lines.insert(m_Lines.end(), pFileLine);
	}
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

int _cdecl FileLine::NormalizedHashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2)
{
	if (pLine1->GetNormalizedHash() < pLine2->GetNormalizedHash())
	{
		return 1;
	}
	// if hash is the same, compare line numbers
	if (pLine1->GetNormalizedHash() == pLine2->GetNormalizedHash()
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

int _cdecl FileLine::NormalizedGroupHashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2)
{
	if (pLine1->GetNormalizedGroupHash() < pLine2->GetNormalizedGroupHash())
	{
		return 1;
	}
	// if hash is the same, compare line numbers
	if (pLine1->GetNormalizedGroupHash() == pLine2->GetNormalizedGroupHash()
		&& pLine1->GetLineNumber() < pLine2->GetLineNumber())
	{
		return 1;
	}
	return 0;
}

#undef new
bool FileItem::Load()
{
	CThisApp * pApp = GetApp();
	// check if it is C or CPP file
	FILE * file = _tfopen(LPCTSTR(GetFullName()), _T("r"));
	if (NULL == file)
	{
		return false;
	}
	char lineA[2048];
	wchar_t lineW[2048];
	char buf[512];
	setvbuf(file, buf, _IOFBF, sizeof buf);
#ifdef _DEBUG
	DWORD BeginTime = timeGetTime();
#endif
	LPCTSTR line;
#ifdef _UNICODE
	line = lineW;
#else
	line = lineA;
#endif
	TCHAR TabExpandedLine[2048];

	FileLine * pLineList = NULL;
	// peek two first bytes, to see if it is UNICODE file
	wchar_t FirstChar = fgetwc(file);

	rewind(file);
	clearerr(file);

	if ((FirstChar & 0xFFFF) == 0xFFFE)
	{
		m_IsUnicode = true;
	}
	else if ((FirstChar & 0xFFFF) == 0xFEFF)
	{
		m_IsUnicode = true;
		m_IsUnicodeBigEndian = true;
	}

	for (unsigned LinNum =0; ; LinNum++)
	{
		if (m_IsUnicode)
		{
			if (m_IsUnicodeBigEndian)
			{
			}
			else
			{
				if (NULL == fgetws(lineW, (sizeof lineW / sizeof lineW[0]) - 1, file))
				{
					break;
				}
			}
#ifndef _UNICODE
			wcstombs(lineA, lineW, sizeof lineA  - 1);
#endif
		}
		else
		{
			if (NULL == fgets(lineA, sizeof lineA - 1, file))
			{
				break;
			}
#ifdef _UNICODE
			mbstowcs(lineW, lineA, (sizeof lineW / sizeof lineW[0]) - 1);
#endif
		}
		// expand tabs
		for (int i = 0, pos = 0; line[i] != 0 && pos < sizeof TabExpandedLine - 1; pos++)
		{
			if (line[i] == '\t')
			{
				TabExpandedLine[pos] = ' ';
				if ((pos + 1) % pApp->m_TabIndent == 0)
				{
					i++;
				}
			}
			else if (line[i] == '\n')
			{
				i++;
				pos--;
			}
			else
			{
				TabExpandedLine[pos] = line[i];
				i++;
			}
		}
		TabExpandedLine[pos] = 0;
		FileLine * pLine = new FileLine(TabExpandedLine, true, m_C_Cpp);
		if (pLine)
		{
			pLine->SetNext(pLineList);
			pLineList = pLine;
		}
	}
	fclose(file);
#ifdef _DEBUG
	TRACE("File %s loaded in %d ms\n", LPCTSTR(GetFullName()), timeGetTime() - BeginTime);
	BeginTime = timeGetTime();
#endif
	m_Lines.resize(LinNum);
	unsigned i, j;
	for (i = LinNum; i > 0 && pLineList != NULL; i--)
	{
		FileLine * pLine = pLineList;
		pLineList = pLine->Next();
		pLine->SetLineNumber(i-1);
		m_Lines[i-1] = pLine;
	}

	// make sorted array of the string hash values
	m_NonBlankLines.resize(LinNum);
	for (i = 0, j = 0; i < LinNum; i++)
	{
		if ( ! m_Lines[i]->IsBlank())
		{
			m_NonBlankLines[j] = m_Lines[i];
			j++;
		}
	}
	if (j != m_NonBlankLines.size())
	{
		m_NonBlankLines.resize(j);
	}
	LinNum = j;
	// before we sorted the arrays, make hash codes for groups of lines

	for (i = 0; i < LinNum; i++)
	{
		DWORD GroupHash[MaxLineGroupSize];
		DWORD NormGroupHash[MaxLineGroupSize];
		for (j = 0; j < MaxLineGroupSize && j < pApp->m_NumberOfIdenticalLines && j + i < LinNum; j++)
		{
			GroupHash[j] = m_NonBlankLines[i + j]->GetHash();
			NormGroupHash[j] = m_NonBlankLines[i + j]->GetNormalizedHash();
		}
		m_NonBlankLines[i]->SetGroupHash(CalculateHash(GroupHash, j * sizeof GroupHash[0]));
		m_NonBlankLines[i]->SetNormalizedGroupHash(
													CalculateHash(NormGroupHash, j * sizeof NormGroupHash[0]));
	}

	m_HashSortedLines = m_NonBlankLines;
	std::sort(m_HashSortedLines.begin(), m_HashSortedLines.end(),
			FileLine::HashAndLineNumberCompareFunc);

	m_NormalizedHashSortedLines = m_NonBlankLines;
	std::sort(m_NormalizedHashSortedLines.begin(), m_NormalizedHashSortedLines.end(),
			FileLine::NormalizedHashAndLineNumberCompareFunc);

	m_HashSortedLineGroups = m_NonBlankLines;
	std::sort(m_HashSortedLineGroups.begin(), m_HashSortedLineGroups.end(),
			FileLine::GroupHashAndLineNumberCompareFunc);

	m_NormalizedHashSortedLineGroups = m_NonBlankLines;
	std::sort(m_NormalizedHashSortedLineGroups.begin(), m_NormalizedHashSortedLineGroups.end(),
			FileLine::NormalizedGroupHashAndLineNumberCompareFunc);
#ifdef _DEBUG
	// check if the array is sorted correctly
	{
		unsigned i;
		for (i = 1; i < m_HashSortedLines.size(); i++)
		{
			if (m_HashSortedLines[i]->GetHash() < m_HashSortedLines[i - 1]->GetHash()
				|| (m_HashSortedLines[i]->GetHash() == m_HashSortedLines[i - 1]->GetHash()
					&& m_HashSortedLines[i]->GetLineNumber() < m_HashSortedLines[i - 1]->GetLineNumber()))
			{
				TRACE("Item %d: hash=%x, lineNum=%d, item %d: hash=%x, LineNum=%d\n",
					i - 1, m_HashSortedLines[i - 1]->GetHash(), m_HashSortedLines[i - 1]->GetLineNumber(),
					i, m_HashSortedLines[i]->GetHash(), m_HashSortedLines[i]->GetLineNumber());
				//break;
			}
		}
		for (i = 1; i < m_NormalizedHashSortedLines.size(); i++)
		{
			if (m_NormalizedHashSortedLines[i]->GetNormalizedHash() < m_NormalizedHashSortedLines[i - 1]->GetNormalizedHash()
				|| (m_NormalizedHashSortedLines[i]->GetNormalizedHash() == m_NormalizedHashSortedLines[i - 1]->GetNormalizedHash()
					&& m_NormalizedHashSortedLines[i]->GetLineNumber() < m_NormalizedHashSortedLines[i - 1]->GetLineNumber()))
			{
				TRACE("Item %d: NormHash=%x, lineNum=%d, item %d: NormHash=%x, LineNum=%d\n",
					i - 1, m_NormalizedHashSortedLines[i - 1]->GetNormalizedHash(), m_NormalizedHashSortedLines[i - 1]->GetLineNumber(),
					i, m_NormalizedHashSortedLines[i]->GetNormalizedHash(), m_NormalizedHashSortedLines[i]->GetLineNumber());
				//break;
			}
		}
		for (i = 1; i < m_HashSortedLineGroups.size(); i++)
		{
			if (m_HashSortedLineGroups[i]->GetGroupHash() < m_HashSortedLineGroups[i - 1]->GetGroupHash()
				|| (m_HashSortedLineGroups[i]->GetGroupHash() == m_HashSortedLineGroups[i - 1]->GetGroupHash()
					&& m_HashSortedLineGroups[i]->GetLineNumber() < m_HashSortedLineGroups[i - 1]->GetLineNumber()))
			{
				TRACE("Item %d: GroupHash=%x, lineNum=%d, item %d: GroupHash=%x, LineNum=%d\n",
					i - 1, m_HashSortedLineGroups[i - 1]->GetGroupHash(), m_HashSortedLineGroups[i - 1]->GetLineNumber(),
					i, m_HashSortedLineGroups[i]->GetGroupHash(), m_HashSortedLineGroups[i]->GetLineNumber());
				//break;
			}
		}
		for (i = 1; i < m_NormalizedHashSortedLineGroups.size(); i++)
		{
			if (m_NormalizedHashSortedLineGroups[i]->GetNormalizedGroupHash() < m_NormalizedHashSortedLineGroups[i - 1]->GetNormalizedGroupHash()
				|| (m_NormalizedHashSortedLineGroups[i]->GetNormalizedGroupHash() == m_NormalizedHashSortedLineGroups[i - 1]->GetNormalizedGroupHash()
					&& m_NormalizedHashSortedLineGroups[i]->GetLineNumber() < m_NormalizedHashSortedLineGroups[i - 1]->GetLineNumber()))
			{
				TRACE("Item %d: GroupNormHash=%x, lineNum=%d, item %d: GroupNormHash=%x, LineNum=%d\n",
					i - 1, m_NormalizedHashSortedLineGroups[i - 1]->GetNormalizedGroupHash(), m_NormalizedHashSortedLineGroups[i - 1]->GetLineNumber(),
					i, m_NormalizedHashSortedLineGroups[i]->GetNormalizedGroupHash(), m_NormalizedHashSortedLineGroups[i]->GetLineNumber());
				//break;
			}
		}
	}
#endif
	// make sorted array of the normalized string hash values
#ifdef _DEBUG
	TRACE("Lines sorted in %d ms\n", timeGetTime() - BeginTime);
#endif

	return true;
}

// sort directories first then names
int _cdecl FileItem::DirNameSortFunc(const void * p1, const void * p2)
{
	return DirNameCompare(*(FileItem * const *) p1, *(FileItem * const *) p2);
}

int _cdecl FileItem::DirNameSortBackwardsFunc(const void * p1, const void * p2)
{
	return DirNameCompare(*(FileItem * const *) p2, *(FileItem * const *) p1);
}

int FileItem::DirNameCompare(FileItem * Item1, FileItem * Item2)
{
	if (NULL == Item1)
	{
		return NULL != Item2;
	}
	if (NULL == Item2)
	{
		return -1;
	}
	// compare subdirs
	// compare pointers to dir name buffers
	if (Item1->GetSubdir() == Item2->GetSubdir())
	{
		return Item1->m_Name.CompareNoCase(Item2->m_Name);
	}

	if (0 == Item1->m_Subdir.GetLength()
		&& 0 != Item2->m_Subdir.GetLength())
	{
		return -1;
	}
	if (0 == Item2->m_Subdir.GetLength()
		&& 0 != Item1->m_Subdir.GetLength())
	{
		return 1;
	}
	int result = Item1->m_Subdir.CompareNoCase(Item2->m_Subdir);
	if (0 != result)
	{
		return result;
	}
	return Item1->m_Name.CompareNoCase(Item2->m_Name);
}

// sort names first then directories
int _cdecl FileItem::NameSortFunc(const void * p1, const void * p2)
{
	return NameCompare(*(FileItem * const *) p1, *(FileItem * const *) p2);
}

int _cdecl FileItem::NameSortBackwardsFunc(const void * p1, const void * p2)
{
	return NameCompare(*(FileItem * const *) p2, *(FileItem * const *) p1);
}

int FileItem::NameCompare(FileItem * Item1, FileItem * Item2)
{
	if (NULL == Item1)
	{
		return NULL != Item2;
	}
	if (NULL == Item2)
	{
		return -1;
	}
	int result = Item1->m_Name.CollateNoCase(Item2->m_Name);
	if (0 != result)
	{
		return result;
	}
	// compare subdirs

	if (0 == Item1->m_Subdir.GetLength()
		&& 0 != Item2->m_Subdir.GetLength())
	{
		return 1;
	}
	if (0 == Item2->m_Subdir.GetLength()
		&& 0 != Item1->m_Subdir.GetLength())
	{
		return -1;
	}

	return Item1->m_Subdir.CollateNoCase(Item2->m_Subdir);
}


int _cdecl FileItem::TimeSortFunc(const void * p1, const void * p2)
{
	return TimeCompare(*(FileItem * const *) p1, *(FileItem * const *) p2);
}

int _cdecl FileItem::TimeSortBackwardsFunc(const void * p1, const void * p2)
{
	return TimeCompare(*(FileItem * const *) p2, *(FileItem * const *) p1);
}

int FileItem::TimeCompare(FileItem * Item1, FileItem * Item2)
{
	if (NULL == Item1)
	{
		return NULL != Item2;
	}
	if (NULL == Item2)
	{
		return -1;
	}
	if (Item1->m_LastWriteTime.dwLowDateTime == Item2->m_LastWriteTime.dwLowDateTime
		&& Item1->m_LastWriteTime.dwHighDateTime == Item2->m_LastWriteTime.dwHighDateTime)
	{
		return NameCompare(Item1, Item2);
	}
	if (Item1->m_LastWriteTime.dwHighDateTime > Item2->m_LastWriteTime.dwHighDateTime)
	{
		return 1;
	}
	if (Item1->m_LastWriteTime.dwHighDateTime < Item2->m_LastWriteTime.dwHighDateTime)
	{
		return -1;
	}
	if (Item1->m_LastWriteTime.dwLowDateTime > Item2->m_LastWriteTime.dwLowDateTime)
	{
		return 1;
	}
	if (Item1->m_LastWriteTime.dwLowDateTime < Item2->m_LastWriteTime.dwLowDateTime)
	{
		return -1;
	}
	return 0;
}

FileCheckResult FileItem::CheckForFileChanged()
{
	CString s = GetFullName();
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(s, & wfd);
	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		return FileDeleted;
	}
	FindClose(hFind);
	if (wfd.ftLastWriteTime.dwHighDateTime != m_LastWriteTime.dwHighDateTime
		|| wfd.ftLastWriteTime.dwLowDateTime != m_LastWriteTime.dwLowDateTime)
	{
		return FileTimeChanged;
	}
	return FileUnchanged;
}

FileCheckResult FileItem::ReloadIfChanged()
{
	CString s = GetFullName();
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(s, & wfd);
	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		Unload();
		return FileDeleted;
	}
	FindClose(hFind);
	if (wfd.ftLastWriteTime.dwHighDateTime != m_LastWriteTime.dwHighDateTime
		|| wfd.ftLastWriteTime.dwLowDateTime != m_LastWriteTime.dwLowDateTime)
	{
		Unload();
		m_LastWriteTime = wfd.ftLastWriteTime;
		if (Load())
		{
			return FileTimeChanged;
		}
		else
		{
			return FileDeleted;
		}
	}
	return FileUnchanged;
}

PairCheckResult FilePair::CheckForFilesChanged()
{
	if (MemoryFile == m_ComparisionResult)
	{
		return FilesUnchanged;
	}
	FileCheckResult res1 = FileDeleted;
	FileCheckResult res2 = FileDeleted;
	if (NULL != pFirstFile)
	{
		res1 = pFirstFile->CheckForFileChanged();
	}

	if (NULL != pSecondFile)
	{
		res2 = pSecondFile->CheckForFileChanged();
	}
	if (FileDeleted == res1
		&& FileDeleted == res2)
	{
		return FilesDeleted;
	}
	if (NULL != pFirstFile
		&& res1 != FileUnchanged
		|| NULL != pSecondFile
		&& res2 != FileUnchanged)
	{
		return FilesTimeChanged;
	}
	return FilesUnchanged;
}

PairCheckResult FilePair::ReloadIfChanged()
{
	if (MemoryFile == m_ComparisionResult)
	{
		return FilesUnchanged;
	}
	FileCheckResult res1 = FileDeleted;
	FileCheckResult res2 = FileDeleted;
	if (NULL != pFirstFile)
	{
		res1 = pFirstFile->CheckForFileChanged();
	}

	if (NULL != pSecondFile)
	{
		res2 = pSecondFile->CheckForFileChanged();
	}
	if ((NULL == pFirstFile
			|| res1 == FileUnchanged)
		&& (NULL == pSecondFile
			|| res2 == FileUnchanged))
	{
		return FilesUnchanged;
	}

	FreeLinePairData();
	if (NULL != pFirstFile)
	{
		if (res1 != FileDeleted)
		{
			res1 = pFirstFile->ReloadIfChanged();
		}

		if (FileDeleted == res1)
		{
			delete pFirstFile;
			pFirstFile = NULL;
			m_ComparisionResult = OnlySecondFile;
		}
	}
	if (NULL != pSecondFile)
	{
		if (res2 != FileDeleted)
		{
			res2 = pSecondFile->ReloadIfChanged();
		}

		if (FileDeleted == res2)
		{
			delete pSecondFile;
			pSecondFile = NULL;
			m_ComparisionResult = OnlyFirstFile;
		}
	}
	if (FileDeleted == res1
		&& FileDeleted == res2)
	{
		m_ComparisionResult = FileUnaccessible;
		return FilesDeleted;
	}
	return FilesTimeChanged;
}

void FileList::GetSortedList(CArray<FileItem *, FileItem *> & ItemArray, DWORD SortFlags)
{
	ItemArray.SetSize(m_NumFiles);
	if (0 == m_NumFiles)
	{
		return;
	}
	// sort the file lists by subdirs and names
	FileItem ** pItems = ItemArray.GetData();
	if (NULL == pItems)
	{
		return;
	}
	FileItem * pCurItem = m_pList;
	for (int i = 0; i < m_NumFiles; i++)
	{
		pItems[i] = pCurItem;
		if (NULL != pCurItem)
		{
			pCurItem = pCurItem->m_pNext;
		}
	}
	int (_cdecl * SortFunc)(const void * , const void * );
	switch (SortFlags)
	{
	case SortNameFirst:
		SortFunc = FileItem::NameSortFunc;
		break;
	default:
	case SortDirFirst:
		SortFunc = FileItem::DirNameSortFunc;
		break;
	case SortDataModified:
		SortFunc = FileItem::TimeSortFunc;
		break;
	case SortNameFirst | SortBackwards:
		SortFunc = FileItem::NameSortBackwardsFunc;
		break;
	case SortDirFirst | SortBackwards:
		SortFunc = FileItem::DirNameSortBackwardsFunc;
		break;
	case SortDataModified | SortBackwards:
		SortFunc = FileItem::TimeSortBackwardsFunc;
		break;
	}
	qsort(pItems, m_NumFiles, sizeof (FileItem *), SortFunc);
#ifdef _DEBUG
	for (i = 0; i < m_NumFiles && NULL != pItems[i]; i++)
	{
		if (0) TRACE("Sorted file item: subdir=%s, Name=\"%s\"\n",
					pItems[i]->GetSubdir(), pItems[i]->GetName());
	}
#endif

}


struct LineHashComparison
{
	LineHashComparison(unsigned n) : LineNum(n) {}
	unsigned LineNum;
	bool operator ()(FileLine const * pLine2, FileLine const * pKeyLine)
	{
		if (pKeyLine->GetNormalizedHash() > pLine2->GetNormalizedHash())
		{
			return 1;
		}
		if (pKeyLine->GetNormalizedHash() < pLine2->GetNormalizedHash())
		{
			return 0;
		}
		return LineNum > pLine2->GetLineNumber();
	}
};

struct LineGroupHashComparison
{
	LineGroupHashComparison(unsigned n) : LineNum(n) {}
	unsigned LineNum;
	bool operator ()(FileLine const * pLine2, FileLine const * pKeyLine)
	{
		if (pKeyLine->GetNormalizedGroupHash() > pLine2->GetNormalizedGroupHash())
		{
			return 1;
		}
		if (pKeyLine->GetNormalizedGroupHash() < pLine2->GetNormalizedGroupHash())
		{
			return 0;
		}
		return LineNum > pLine2->GetLineNumber();
	}
};

// find the line with the same hash and same and greater number, return -1 if not found
const FileLine * FileItem::FindMatchingLine(const FileLine * pLine, unsigned nStartLineNum, unsigned nEndLineNum)
{
	vector<FileLine *>::iterator ppLine =
		lower_bound(m_NormalizedHashSortedLines.begin(),
					m_NormalizedHashSortedLines.end(), pLine,
					LineHashComparison(nStartLineNum));

#ifdef _DEBUG
	{
		if (ppLine < m_NormalizedHashSortedLines.begin()
			|| ppLine > m_NormalizedHashSortedLines.end())
		{
			TRACE("Inconsistent address returned from BinLookupAbout\n");
			return NULL;
		}

		// the item should be >= than the key,
		// and the previous item should be < than key
		if (ppLine < m_NormalizedHashSortedLines.end())
		{
			const FileLine * pFoundLine = *ppLine;
			if (pFoundLine->GetNormalizedHash() < pLine->GetNormalizedHash()
				|| (pFoundLine->GetNormalizedHash() == pLine->GetNormalizedHash()
					&& pFoundLine->GetLineNumber() < nStartLineNum))
			{
				TRACE("total lines: %d, "
					"Key hash=%X, LineNumber=%d  > found hash=%X, LineNumber=%d\n",
					m_NormalizedHashSortedLines.size(),
					pLine->GetNormalizedHash(), nStartLineNum,
					pFoundLine->GetNormalizedHash(),
					pFoundLine->GetLineNumber());
				__asm int 3
			}
		}
		if (ppLine > m_NormalizedHashSortedLines.begin())
		{
			const FileLine * pPrevLine = *(ppLine-1);
			if ( pPrevLine->GetNormalizedHash() > pLine->GetNormalizedHash()
				|| (pPrevLine->GetNormalizedHash() == pLine->GetNormalizedHash()
					&& pPrevLine->GetLineNumber() >= nStartLineNum))
			{
				TRACE("total lines: %d, "
					"Key hash=%X, LineNumber=%d,  <= previous hash=%X, LineNumber=%d\n",
					m_NormalizedHashSortedLines.size(),
					pLine->GetNormalizedHash(), nStartLineNum,
					pPrevLine->GetNormalizedHash(),
					pPrevLine->GetLineNumber());
				__asm int 3
			}
		}
	}
#endif
	if (ppLine >= m_NormalizedHashSortedLines.end())
	{
		return NULL;
	}
	FileLine * pFoundLine = *ppLine;
	if (pFoundLine->GetLineNumber() < nEndLineNum
		&& pFoundLine->IsNormalizedEqual(pLine))
	{
#ifdef _DEBUG
		if (pFoundLine->GetLineNumber() < nStartLineNum)
		{
			TRACE("Found line num=%d, required: %d\n",
				pFoundLine->GetLineNumber(), nStartLineNum);
			__asm int 3
		}
#endif
		return pFoundLine;
	}
	else
		return NULL;
}

// find the line with the same hash and same and greater number, return -1 if not found
const FileLine * FileItem::FindMatchingLineGroupLine(const FileLine * pLine, unsigned nStartLineNum, unsigned nEndLineNum)
{
	vector<FileLine *>::iterator ppLine =
		lower_bound(m_NormalizedHashSortedLineGroups.begin(),
					m_NormalizedHashSortedLineGroups.end(), pLine,
					LineGroupHashComparison(nStartLineNum));

#ifdef _DEBUG
	{
		// verify that the correct position found
		if (ppLine < m_NormalizedHashSortedLineGroups.begin()
			|| ppLine > m_NormalizedHashSortedLineGroups.end())
		{
			TRACE("Inconsistent address returned from BinLookupAbout\n");
			return NULL;
		}
		// the item should be >= than the key,
		// and the previous item should be < than key
		if (ppLine < m_NormalizedHashSortedLineGroups.end())
		{
			const FileLine * pFoundLine = *ppLine;
			if (pFoundLine->GetNormalizedGroupHash() < pLine->GetNormalizedGroupHash()
				|| (pFoundLine->GetNormalizedGroupHash() == pLine->GetNormalizedGroupHash()
					&& pFoundLine->GetLineNumber() < nStartLineNum))
			{
				TRACE("total lines: %d, "
					"Key hash=%X, LineNumber=%d  > found hash=%X, LineNumber=%d\n",
					m_NormalizedHashSortedLineGroups.size(),
					pLine->GetNormalizedGroupHash(), nStartLineNum,
					pFoundLine->GetNormalizedGroupHash(),
					pFoundLine->GetLineNumber());
				__asm int 3
			}
		}
		if (ppLine > m_NormalizedHashSortedLineGroups.begin())
		{
			const FileLine * pPrevLine = *(ppLine-1);
			if ( pPrevLine->GetNormalizedGroupHash() > pLine->GetNormalizedGroupHash()
				|| (pPrevLine->GetNormalizedGroupHash() == pLine->GetNormalizedGroupHash()
					&& pPrevLine->GetLineNumber() >= nStartLineNum))
			{
				TRACE("total lines: %d, "
					"Key hash=%X, LineNumber=%d,  <= previous hash=%X, LineNumber=%d\n",
					m_NormalizedHashSortedLineGroups.size(),
					pLine->GetNormalizedGroupHash(), nStartLineNum,
					pPrevLine->GetNormalizedGroupHash(),
					pPrevLine->GetLineNumber());
				__asm int 3
			}
		}
	}
#endif
	if (ppLine >= m_NormalizedHashSortedLineGroups.end())
	{
		return NULL;
	}
	FileLine * pFoundLine = *ppLine;
	if (pFoundLine->GetLineNumber() <= nEndLineNum
		&& pFoundLine->GetNormalizedGroupHash() == pLine->GetNormalizedGroupHash()
		&& pFoundLine->IsNormalizedEqual(pLine))
	{
#ifdef _DEBUG
		if (pFoundLine->GetLineNumber() < nStartLineNum)
		{
			TRACE("Found line num=%d, required: %d\n",
				pFoundLine->GetLineNumber(), nStartLineNum);
			__asm int 3
		}
#endif
		return pFoundLine;
	}
	else
		return NULL;
}

bool FileList::LoadFolder(const CString & BaseDir, bool bRecurseSubdirs,
						LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
						LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask)
{
	// make sure the directory is appended with '\', or ends with ':'
	m_BaseDir = BaseDir;
	if ( ! BaseDir.IsEmpty())
	{
		TCHAR c;
		c = BaseDir[BaseDir.GetLength() - 1];
		if (':' != c
			&& '\\' != c
			&& '/' != c)
		{
			m_BaseDir += _T("\\");
		}
	}
	return LoadSubFolder(CString(), bRecurseSubdirs, sInclusionMask, sExclusionMask,
						sC_CPPMask, sBinaryMask);
}

bool FileList::LoadSubFolder(const CString & Subdir, bool bRecurseSubdirs,
							LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
							LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask)
{
	TRACE("LoadSubFolder: scanning %s\n", LPCTSTR(Subdir));
	CThisApp * pApp = GetApp();

	WIN32_FIND_DATA wfd;
	CString SubDirectory(Subdir);
	// make sure the name contains '\'.
	if ( ! Subdir.IsEmpty())
	{
		SubDirectory += _T("\\");
	}
	CString name = m_BaseDir + SubDirectory + "*";
	HANDLE hFind = FindFirstFile(name, & wfd);
	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		return false;
	}
	do
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (0) TRACE("Found the subdirectory %s\n", wfd.cFileName);
			if (bRecurseSubdirs
				&& 0 != _tcscmp(wfd.cFileName, _T("."))
				&& 0 != _tcscmp(wfd.cFileName, _T(".."))
				)
			{
				// scan the subdirectory
				CString NewDir = SubDirectory + wfd.cFileName;
				LoadSubFolder(NewDir, true,
							sInclusionMask, sExclusionMask, sC_CPPMask, sBinaryMask);
			}
		}
		else
		{
			// filter the file and add it to the list, if it matches the pattern.
			if (0) TRACE("File %s found\n", wfd.cFileName);
			if (! MultiPatternMatches(wfd.cFileName, sInclusionMask)
				|| MultiPatternMatches(wfd.cFileName, sExclusionMask))
			{
				TRACE("File name does not match\n");
				continue;
			}
			if (0) TRACE("New file item: Name=\"%s\", base dir=%s, subdir=%s\n",
						wfd.cFileName, m_BaseDir, SubDirectory);

			FileItem * pFile = new FileItem( & wfd, m_BaseDir, SubDirectory);
			if (NULL == pFile)
			{
				continue;
			}
			if (pApp->m_bUseBinaryFilesFilter
				&& MultiPatternMatches(wfd.cFileName, sBinaryMask))
			{
				pFile->m_IsBinary = true;
			}
			else if (pApp->m_bUseCppFilter)
			{
				pFile->m_C_Cpp = MultiPatternMatches(wfd.cFileName, sC_CPPMask);
			}
			// add to the array
			pFile->m_pNext = m_pList;
			m_pList = pFile;
			m_NumFiles++;
		}
	}
	while (FindNextFile(hFind, & wfd));
	FindClose(hFind);
	return true;
}

void FileList::FreeFileList()
{
	while (NULL != m_pList)
	{
		FileItem * tmp = m_pList;
		m_pList = tmp->m_pNext;
		delete tmp;
	}
	m_NumFiles = 0;
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
static int RemoveExtraWhitespaces(LPTSTR pDst, LPCTSTR Src, unsigned DstLen,
								char * pWhitespaceMask, int WhitespaceMaskSize,
								bool c_cpp_file)
{
	// pDst buffer size must be greater or equal strlen(pSrc)
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
	if (c_cpp_file && '#' == Src[SrcIdx])
	{
		// consider all characters alpha, that is don't remove all but one spaces
		LeaveOneExtraSpace = true;
	}

	while (Src[SrcIdx] && DstIdx + 1 < DstLen)
	{
		// it's OK to remove extra spaces between non-alpha bytes,
		// unless these are the following pairs:
		// /*, */, //, ++, --, &&, ||, ##, ->*, ->, >>, << >=, <=, ==, ::
		// that is most two-char operators
		// strings also must be kept intact.
		// Can remove extra spaces between alpha and non-alpha, unless LeaveOneExtraSpace = true
		c = Src[SrcIdx];
		bool c_IsAlpha = (_istalnum(TCHAR_MASK & c)  || '_' == c);
		if ((RemovedWhitespaces && LeaveOneExtraSpace) || (PrevCharAlpha && c_IsAlpha))
		{
			if(DstIdx + 1 >= DstLen)
			{
				break;
			}
			// insert one whitespace to the output string
			pDst[DstIdx] = ' ';
			DstIdx++;
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
				if(DstIdx + 1 >= DstLen)
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
				for (int i = 0; i < sizeof ReservedPairs / sizeof ReservedPairs[0]; i++)
				{
					int ReservedPair = (ReservedPairs[i] & 0xFF) | ((ReservedPairs[i] & 0xFF00) << 8);
					if (pair == ReservedPair)
					{
						if(DstIdx + 1 < DstLen)
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
					if(DstIdx + 1 >= DstLen)
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
						if(DstIdx + 1 >= DstLen)
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
						if(DstIdx + 1 >= DstLen)
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
					if(DstIdx + 1 >= DstLen)
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
						if(DstIdx + 1 >= DstLen)
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
						if(DstIdx + 1 >= DstLen)
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
				if(DstIdx + 1 >= DstLen)
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

// find difference in the strings, and build the array of inserts and
// and deleted chars
// returns number of different characters.
// if ppSections is not NULL, builds a list of sections
int MatchStrings(const FileLine * pStr1, const FileLine * pStr2, StringSection ** ppSections, int nMinMatchingChars)
{
	int nDifferentChars = 0;
	// zero the sections list
	if (ppSections != NULL)
	{
		*ppSections = NULL;
	}
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
			pSection->Length = pStr1->GetLength();
			pSection->Attr = StringSection::Erased | StringSection::Undefined;
			pSection->pNext = NULL;

			// check if it is whitespace difference
			if (pStr1->IsBlank())
			{
				pSection->Attr |= pSection->Whitespace;
			}
			*ppSections = pSection;
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
			pSection->Length = pStr2->GetLength();
			pSection->Attr = StringSection::Inserted | StringSection::Undefined;
			pSection->pNext = NULL;

			// check if it is whitespace difference
			if (pStr2->IsBlank())
			{
				pSection->Attr |= pSection->Whitespace;
			}
			*ppSections = pSection;
		}
		return pStr2->GetLength();
	}

	LPCTSTR str1 = pStr1->GetText();
	LPCTSTR str2 = pStr2->GetText();

	LPCTSTR pEqualStrBegin = str1;
	StringSection * pLastSection = NULL;
	while (1)
	{
		if (str1[0] == str2[0]
			&& str1[0] != 0)
		{
			str1++;
			str2++;
			continue;
		}
		if (str1 != pEqualStrBegin
			&& ppSections != NULL)
		{
			StringSection * pSection = new StringSection;
			if (NULL != pSection)
			{
				pSection->pBegin = pEqualStrBegin;
				pSection->Length = str1 - pEqualStrBegin;
				pSection->Attr = StringSection::Identical;
				pSection->pNext = NULL;
				if (NULL != pLastSection)
				{
					pLastSection->pNext = pSection;
				}
				else
				{
					* ppSections = pSection;
				}
				pLastSection = pSection;
			}
		}
		if (str1[0] == 0
			&& str2[0] == 0)
		{
			break;
		}
		// difference found, starting from str1, str2

		bool found = false;
		for (unsigned idx1 = 0, idx2 = 0; (str1[idx1] != 0 || str2[idx2] != 0) && ! found; )
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
				for (i = 0; i <= idx2; i++)
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

		int nDiffChars1 = 0;
		int nDiffChars2 = 0;
		StringSection * pErasedSection = NULL;
		StringSection * pAddedSection = NULL;

		if (idx1 != 0
			&& ppSections != NULL)
		{
			pErasedSection = new StringSection;
			if (NULL != pErasedSection)
			{
				pErasedSection->pBegin = str1;
				pErasedSection->Length = idx1;
				pErasedSection->Attr = StringSection::Erased | StringSection::Undefined;
				pErasedSection->pNext = NULL;

				// check if it is whitespace difference
				pErasedSection->Attr |= pErasedSection->Whitespace;
				for (unsigned i = 0, CharIdx = str1 - pStr1->GetText(); i < idx1; i++, CharIdx++)
				{
					if ( ! pStr1->IsExtraWhitespace(CharIdx))
					{
						// it is NOT whitespace diff
						pErasedSection->Attr &= ~pErasedSection->Whitespace;
						nDiffChars1 = idx1;
						break;
					}
				}
				// whitespace difference is not counted in nDifferentChars

			}
		}
		else
		{
			// check if it is whitespace difference
			for (unsigned i = 0, CharIdx = str1 - pStr1->GetText(); i < idx1; i++, CharIdx++)
			{
				if ( ! pStr1->IsExtraWhitespace(CharIdx))
				{
					// it is NOT whitespace diff
					nDiffChars1 = idx1;
					break;
				}
			}
			// whitespace difference is not counted in nDifferentChars
		}

		if (idx2 != 0
			&& ppSections != NULL)
		{
			pAddedSection = new StringSection;
			if (NULL != pAddedSection)
			{
				pAddedSection->pBegin = str2;
				pAddedSection->Length = idx2;
				pAddedSection->Attr = StringSection::Inserted | StringSection::Undefined;
				pAddedSection->pNext = NULL;

				// check if it is whitespace difference
				pAddedSection->Attr |= pAddedSection->Whitespace;
				for (unsigned i = 0, CharIdx = str2 - pStr2->GetText(); i < idx2; i++, CharIdx++)
				{
					if ( ! pStr2->IsExtraWhitespace(CharIdx))
					{
						// it is NOT whitespace diff
						pAddedSection->Attr &= ~pAddedSection->Whitespace;
						nDiffChars2 = idx2;
						break;
					}
				}
				// whitespace difference is not counted in nDifferentChars

			}
		}
		else
		{
			// check if it is whitespace difference
			for (unsigned i = 0, CharIdx = str2 - pStr2->GetText(); i < idx2; i++, CharIdx++)
			{
				if ( ! pStr2->IsExtraWhitespace(CharIdx))
				{
					// it is NOT whitespace diff
					nDiffChars2 = idx2;
					break;
				}
			}
			// whitespace difference is not counted in nDifferentChars
		}

		// number of different chars is the greater of different string length
		if (nDiffChars1 > nDiffChars2)
		{
			nDifferentChars += nDiffChars1;
		}
		else
		{
			nDifferentChars += nDiffChars2;
		}

		if (NULL != pErasedSection
			&& NULL != pAddedSection)
		{
			// if there is space in the beginning of the next text, or if it is end of the line,
			// then check last chars of the deleted text. If it's
			if ((' ' != pAddedSection->pBegin[pAddedSection->Length - 1]
					&& ' ' == pErasedSection->pBegin[pErasedSection->Length - 1]
					&& (' ' == pErasedSection->pBegin[pErasedSection->Length]
						|| 0 == pErasedSection->pBegin[pErasedSection->Length]))

				|| (' ' != pErasedSection->pBegin[0]
					&& ' ' == pAddedSection->pBegin[0]
					&& (NULL == pLastSection
						|| ' ' == pLastSection->pBegin[pLastSection->Length - 1])))
			{
				StringSection * tmp = pErasedSection;
				pErasedSection = pAddedSection;
				pAddedSection = tmp;
			}
		}

		if (NULL != pErasedSection)
		{
			if (NULL != pLastSection)
			{
				pLastSection->pNext = pErasedSection;
			}
			else
			{
				* ppSections = pErasedSection;
			}
			pLastSection = pErasedSection;
		}

		if (NULL != pAddedSection)
		{
			if (NULL != pLastSection)
			{
				pLastSection->pNext = pAddedSection;
			}
			else
			{
				* ppSections = pAddedSection;
			}
			pLastSection = pAddedSection;
		}

		str1 += idx1;
		str2 += idx2;
		pEqualStrBegin = str1;
	}
	return nDifferentChars;
}

FileLine::FileLine(LPCTSTR src, bool MakeNormalizedString, bool c_cpp_file)
	: //m_Flags(0),
	m_pNext(NULL),
	m_Number(-1),
//m_Link(NULL),
//m_FirstTokenIndex(-1),
	m_HashCode(0),
	m_GroupHashCode(0),
	m_NormalizedHashCode(0),
	m_NormalizedGroupHashCode(0)
{
	m_Length = _tcslen(src);
	TCHAR TmpBuf[2050];
	char WhitespaceMask[2048 / 8];

	m_NormalizedStringLength = RemoveExtraWhitespaces(TmpBuf, src, sizeof TmpBuf,
													WhitespaceMask, sizeof WhitespaceMask,
													c_cpp_file);

	int WhitespaceMaskLength = (m_Length + 7) / 8;

	m_pAllocatedBuf = ::new char[sizeof(TCHAR) * (m_Length + m_NormalizedStringLength + 2) + WhitespaceMaskLength];
	if (NULL != m_pAllocatedBuf)
	{
		TCHAR * pTmp = (TCHAR*) m_pAllocatedBuf;
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
	delete m_pAllocatedBuf;
}

bool FileLine::LooksLike(const FileLine * pOtherLine, int PercentsDifferent) const
{
	unsigned nCharsDifferent = MatchStrings(this, pOtherLine, NULL, GetApp()->m_MinMatchingChars);
	// nCharsDifferent won't count whitespace difference

	unsigned nLength = GetNormalizedLength();
	if (nLength < pOtherLine->GetNormalizedLength())
	{
		nLength = pOtherLine->GetNormalizedLength();
	}
	return (nLength * PercentsDifferent / 100) >= nCharsDifferent;
}

FilePair::FilePair()
	: pNext(NULL),
	pFirstFile(NULL),
	pSecondFile(NULL),
	m_RefCount(1),
	m_LoadedCount(0),
	m_bChanged(false),
	m_bHideFromListView(false),
	m_bSelected(false),
	m_CompletedPercent(0),
	m_ComparisionResult(ResultUnknown)
{
}

void FilePair::Reference()
{
	m_RefCount++;
}

void FilePair::Dereference()
{
	m_RefCount--;
	if (0 == m_RefCount)
	{
		delete this;
	}
}

FilePair::~FilePair()
{
	m_LoadedCount = 0;
	UnloadFiles(true);
	if (NULL != pFirstFile)
	{
		delete pFirstFile;
	}
	if (NULL != pSecondFile)
	{
		delete pSecondFile;
	}
}

int FilePair::Time1SortFunc(const FilePair * Pair1, const FilePair * Pair2)
{
	FileItem * Item1 = Pair1->pFirstFile;
	FileItem * Item2 = Pair2->pFirstFile;

	if (NULL == Item1)
	{
		return NULL != Item2;
	}
	if (NULL == Item2)
	{
		return -1;
	}
	return FileItem::TimeCompare(Item2, Item1);
}

int FilePair::Time1SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2)
{
	FileItem * Item1 = Pair1->pFirstFile;
	FileItem * Item2 = Pair2->pFirstFile;

	if (NULL == Item1)
	{
		return NULL != Item2;
	}
	if (NULL == Item2)
	{
		return -1;
	}
	return FileItem::TimeCompare(Item1, Item2);
}

int FilePair::Time2SortFunc(const FilePair * Pair1, const FilePair * Pair2)
{
	FileItem * Item1 = Pair1->pSecondFile;
	FileItem * Item2 = Pair2->pSecondFile;

	if (NULL == Item1)
	{
		return NULL != Item2;
	}
	if (NULL == Item2)
	{
		return -1;
	}
	return FileItem::TimeCompare(Item2, Item1);
}

int FilePair::Time2SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2)
{
	FileItem * Item1 = Pair1->pSecondFile;
	FileItem * Item2 = Pair2->pSecondFile;

	if (NULL == Item1)
	{
		return NULL != Item2;
	}
	if (NULL == Item2)
	{
		return -1;
	}
	return FileItem::TimeCompare(Item1, Item2);
}

int FilePair::NameCompare(const FilePair * Pair1, const FilePair * Pair2)
{
	FileItem * Item1;
	FileItem * Item2;

	Item1 = Pair1->pFirstFile;
	if (NULL == Item1)
	{
		Item1 = Pair1->pSecondFile;
	}

	Item2 = Pair2->pFirstFile;
	if (NULL == Item2)
	{
		Item2 = Pair2->pSecondFile;
	}
	return FileItem::NameCompare(Item1, Item2);
}

int FilePair::DirNameCompare(const FilePair * Pair1, const FilePair * Pair2)
{
	FileItem * Item1;
	FileItem * Item2;

	Item1 = Pair1->pFirstFile;
	if (NULL == Item1)
	{
		Item1 = Pair1->pSecondFile;
	}

	Item2 = Pair2->pFirstFile;
	if (NULL == Item2)
	{
		Item2 = Pair2->pSecondFile;
	}
	return FileItem::DirNameCompare(Item1, Item2);
}

int FilePair::ComparisionResultPriority() const
{
	// the following order:
	// ResultUnknown, FilesDifferent, VersionInfoDifferent, DifferentInSpaces, FilesIdentical,
	// OnlyFirstFile, OnlySecondFile
	switch (m_ComparisionResult)
	{
	default:
	case ErrorReadingFirstFile:
	case ErrorReadingSecondFile:
	case ResultUnknown:
		return 0;
	case FileUnaccessible:
		return 1;
	case FilesDifferent:
		return 2;
	case FirstFileLonger:
		return 3;
	case SecondFileLonger:
		return 4;
	case DifferentInSpaces:
		return 5;
	case VersionInfoDifferent:
		return 6;
	case FilesIdentical:
	case OnlySecondDirectory:
		return 7;
	case OnlyFirstFile:
	case OnlySecondFile:
		return 8;
	case ReadingFirstFile:
	case ReadingSecondFile:
		return 9;
	}
}

bool FilePair::Compare(const FilePair * Pair1, const FilePair * Pair2, const CompareParam comp)
{
	int result = 0;
	switch (comp.PrimarySort)
	{
	case CompareSubitemName:
		result = NameCompare(Pair1, Pair2);
		break;
	default:
	case CompareSubitemDir:
		result = DirNameCompare(Pair1, Pair2);
		break;
	case CompareSubitemDate1:
		if (comp.PrimaryBackward)
		{
			return Time1SortBackwardsFunc(Pair1, Pair2) < 0;
		}
		else
		{
			return Time1SortFunc(Pair1, Pair2) < 0;
		}
		break;
	case CompareSubitemDate2:
		if (comp.PrimaryBackward)
		{
			return Time2SortBackwardsFunc(Pair1, Pair2) < 0;
		}
		else
		{
			return Time2SortFunc(Pair1, Pair2) < 0;
		}
		break;
	case CompareSubitemResult:
		result = ComparisionSortFunc(Pair1, Pair2);
		if (0 == result)
		{
			CompareParam comp1;
			comp1.PrimarySort = comp.SecondarySort;
			comp1.PrimaryBackward = comp.SecondaryBackward;
			comp1.SecondaryBackward =0;
			comp1.SecondarySort = CompareSubitemName;
			return Compare(Pair1, Pair2, comp1);
		}
		break;
	}
	if (comp.PrimaryBackward)
	{
		return result > 0;
	}
	else
	{
		return result < 0;
	}
}

int FilePair::ComparisionSortFunc(const FilePair * Pair1, const FilePair * Pair2)
{
	int priority1 = Pair1->ComparisionResultPriority();
	int priority2 = Pair2->ComparisionResultPriority();

	if (priority1 > priority2)
	{
		return 1;
	}
	if (priority1 < priority2)
	{
		return -1;
	}

	return 0;
}

CString FilePair::GetComparisionResult() const
{
	static CString sFilesUnaccessible(MAKEINTRESOURCE(IDS_STRING_FILES_UNACCESSIBLE));
	static CString sFilesIdentical(MAKEINTRESOURCE(IDS_STRING_FILES_IDENTICAL));
	static CString sVersionInfoDifferent(MAKEINTRESOURCE(IDS_VERSION_INFO_DIFFERENT));
	static CString sDifferentInSpaces(MAKEINTRESOURCE(IDS_DIFFERENT_IN_SPACES));
	static CString sFilesDifferent(MAKEINTRESOURCE(IDS_FILES_DIFFERENT));
	static CString sOnlyOneExists(MAKEINTRESOURCE(IDS_STRING_ONLY_ONE_EXISTS));
	static CString sOnlyOneSubdirExists(MAKEINTRESOURCE(IDS_STRING_ONE_SUBDIR_EXISTS));
	static CString sOneFileLonger(MAKEINTRESOURCE(IDS_STRING_FILE_IS_LONGER));
	static CString sReadingFile(MAKEINTRESOURCE(IDS_STRING_READING_FILE));
	static CString sErrorReadingFile(MAKEINTRESOURCE(IDS_STRING_ERROR_READING_FILE));
	static CString sCalculatingFingerprint(MAKEINTRESOURCE(IDS_STRING_CALC_FINGERPRINT));
	static CString sComparingFiles(MAKEINTRESOURCE(IDS_STRING_COMPARING));

	CString s;
	switch(m_ComparisionResult)
	{
	case ResultUnknown:
		break;
	case FileUnaccessible:
		return sFilesUnaccessible;
		break;
	case FilesIdentical:
		return sFilesIdentical;
		break;
	case VersionInfoDifferent:
		return sVersionInfoDifferent;
		break;
	case DifferentInSpaces:
		return sDifferentInSpaces;
		break;
	case FilesDifferent:
		return sFilesDifferent;
		break;
	case OnlyFirstFile:
		s.Format(sOnlyOneExists,
				pFirstFile->GetBasedir(), pFirstFile->GetSubdir());
		break;
	case OnlySecondFile:
		s.Format(sOnlyOneExists,
				pSecondFile->GetBasedir(), pSecondFile->GetSubdir());
		break;
	case OnlyFirstDirectory:
		s.Format(sOnlyOneSubdirExists,
				pFirstFile->GetBasedir(), pFirstFile->GetSubdir());
		break;
	case OnlySecondDirectory:
		s.Format(sOnlyOneSubdirExists,
				pSecondFile->GetBasedir(), pSecondFile->GetSubdir());
		break;
	case FirstFileLonger:
		s.Format(sOneFileLonger,
				pFirstFile->GetBasedir(), pFirstFile->GetSubdir());
		break;
	case SecondFileLonger:
		s.Format(sOneFileLonger,
				pSecondFile->GetBasedir(), pSecondFile->GetSubdir());
		break;
	case ErrorReadingFirstFile:
		s.Format(sErrorReadingFile,
				LPCTSTR(pFirstFile->GetFullName()));
		break;
	case ErrorReadingSecondFile:
		s.Format(sErrorReadingFile,
				LPCTSTR(pSecondFile->GetFullName()));
		break;
	case ReadingFirstFile:
		return sReadingFile;
		break;
	case ReadingSecondFile:
		return sReadingFile;
		break;
	case CalculatingFirstFingerprint:
		s.Format(sCalculatingFingerprint, LPCTSTR(pFirstFile->GetFullName()));
		if (m_CompletedPercent != 0)
		{
			CString Percent;
			Percent.Format(_T(" %d%%"), m_CompletedPercent);
			s += Percent;
		}
		break;
	case CalculatingSecondFingerprint:
		s.Format(sCalculatingFingerprint, LPCTSTR(pSecondFile->GetFullName()));
		if (m_CompletedPercent != 0)
		{
			CString Percent;
			Percent.Format(_T(" %d%%"), m_CompletedPercent);
			s += Percent;
		}
		break;
	case ComparingFiles:
		s.Format(sComparingFiles,
				LPCTSTR(pFirstFile->GetFullName()),
				LPCTSTR(pSecondFile->GetFullName()));
		break;
	}
	return s;
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

bool FilePair::LoadFiles()
{
	m_LoadedCount++;
	if (m_LoadedCount > 1)
	{
		return true;
	}
	bool result = true;
	if (NULL != pFirstFile)
	{
		result = (pFirstFile->Load() && result);
	}
	if (NULL != pSecondFile)
	{
		result = (pSecondFile->Load() && result);
	}
	if ( ! result)
	{
		UnloadFiles();
	}
	return result;
}

void FilePair::UnloadFiles(bool ForceUnload)
{
	m_LoadedCount--;
	if (m_LoadedCount > 0 && ! ForceUnload)
	{
		return;
	}
	m_LoadedCount = 0;
	TRACE("Unloading file pair\n");
	FreeLinePairData();

	if (NULL != pFirstFile)
	{
		pFirstFile->Unload();
	}
	if (NULL != pSecondFile)
	{
		pSecondFile->Unload();
	}
}

void FilePair::FreeLinePairData()
{
	unsigned i;
	for (i = 0; i < m_LinePairs.size(); i++)
	{
		LinePair * pPair = m_LinePairs[i];
		while(pPair->pFirstSection != NULL)
		{
			StringSection * tmp = pPair->pFirstSection;
			pPair->pFirstSection = tmp->pNext;
			delete tmp;
		}
		delete pPair;
	}
	m_LinePairs.clear();
	for (i = 0; i < m_DiffSections.size(); i++)
	{
		delete m_DiffSections[i];
	}
	m_DiffSections.clear();

}

FilePair::eFileComparisionResult FilePair::PreCompareFiles(BOOL volatile & bStopOperation)
{
	// TODO: different function for binary comparision
	if ((pFirstFile != NULL && pFirstFile->m_IsBinary)
		|| (pSecondFile != NULL && pSecondFile->m_IsBinary))
	{
		return PreCompareBinaryFiles(bStopOperation);
	}
	if (! LoadFiles())
	{
		return FileUnaccessible;
	}
#ifdef _DEBUG
	DWORD BeginTime = timeGetTime();
#endif
	eFileComparisionResult result = ResultUnknown;
	if (NULL != pFirstFile
		&& NULL != pSecondFile)
	{
		result = PreCompareTextFiles(bStopOperation);
	}
#ifdef _DEBUG
	TRACE("Files compared in %d ms\n", timeGetTime() - BeginTime);
	BeginTime = timeGetTime();
#endif
	UnloadFiles();
	// different comparision for different modes
	return result;
}

FilePair::eFileComparisionResult FilePair::CompareFiles(BOOL volatile & bStopOperation)
{
	// TODO: different function for binary comparision
	if (! LoadFiles())
	{
		return ResultUnknown;
	}
	eFileComparisionResult result = ResultUnknown;
	if (NULL != pFirstFile
		&& NULL != pSecondFile)
	{
		result = CompareTextFiles(bStopOperation);
	}
	else
	{
		// just build the line array
		FileItem * pFile = pFirstFile;
		result = OnlyFirstFile;
		if (NULL == pFile)
		{
			result = OnlySecondFile;
			pFile = pSecondFile;
		}
		if (NULL != pFile)
		{
			m_LinePairs.resize(pFile->GetNumLines());
			for (unsigned i = 0; i < m_LinePairs.size(); i++)
			{
				LinePair * pPair = new LinePair;
				StringSection * pSection = new StringSection;
				if (pPair != NULL
					&& pSection != NULL)
				{
					pPair->pFirstLine = pFile->GetLine(i);
					pPair->pSecondLine = pPair->pFirstLine;
					pSection->pNext = NULL;
					pSection->Attr = pSection->Identical;
					pSection->pBegin = pPair->pFirstLine->GetText();
					pSection->Length = pPair->pFirstLine->GetLength();
					pPair->pFirstSection = pSection;
					pSection->pDiffSection = NULL;
				}
				else
				{
					delete pPair;
					delete pSection;
				}
				m_LinePairs[i] = pPair;
			}
		}
	}
	if (MemoryFile == m_ComparisionResult)
	{
		return MemoryFile;
	}
	// different comparision for different modes
	return result;
}

FilePair::eFileComparisionResult FilePair::PreCompareTextFiles(BOOL volatile & bStopOperation)
{
	int nLine1 = 0;
	int nLine2 = 0;

	int NumLines1 = pFirstFile->GetNumLines();
	int NumNonBlankLines1 = pFirstFile->m_NormalizedHashSortedLines.size();

	int NumLines2 = pSecondFile->GetNumLines();
	int NumNonBlankLines2 = pSecondFile->m_NormalizedHashSortedLines.size();
#if 0
	// files may differ in version stamp only, for example
	if (NumNonBlankLines1 != NumNonBlankLines2)
	{
		return FilesDifferent;
	}
#endif
	bool SpacesDifferent = false;
	while (nLine1 < NumLines1
			|| nLine2 < NumLines2)
	{
		if (nLine1 >= NumLines1)
		{
			if (pSecondFile->GetLine(nLine2)->IsBlank())
			{
				nLine2++;
				SpacesDifferent = true;
				continue;
			}
			return FilesDifferent;
		}

		if (nLine2 >= NumLines2)
		{
			if (pFirstFile->GetLine(nLine1)->IsBlank())
			{
				nLine1++;
				SpacesDifferent = true;
				continue;
			}
			return FilesDifferent;
		}

		const FileLine * Line1 = pFirstFile->GetLine(nLine1);
		const FileLine * Line2 = pSecondFile->GetLine(nLine2);
		if (Line1->IsBlank())
		{
			if (Line2->IsBlank())
			{
				nLine1++;
				nLine2++;
				continue;
			}
			nLine1++;
			SpacesDifferent = true;
			continue;
		}
		else if (Line2->IsBlank())
		{
			nLine2++;
			SpacesDifferent = true;
			continue;
		}
		else
		{
			if ( ! Line1->IsEqual(Line2))
			{
				if (Line1->IsNormalizedEqual(Line2))
				{
					SpacesDifferent = true;
				}
				else
				{
					return FilesDifferent;
				}
			}
			nLine1++;
			nLine2++;
		}
	}
	if (SpacesDifferent)
	{
		return DifferentInSpaces;
	}
	else
	{
		return FilesIdentical;
	}
}

FilePair::FileSection * FilePair::BuildSectionList(int NumLine1Begin, int NumLine1AfterEnd,
													int NumLine2Begin, int NumLine2AfterEnd, bool UseLineGroups)
{
	CThisApp * pApp = GetApp();
	FileSection * pFirstSection = NULL;
	FileSection * pLastSection = NULL;
	FileSection * pSection;

	int nLine1 = NumLine1Begin;
	int nLine2 = NumLine2Begin;

	int NumLines1 = NumLine1AfterEnd;

	int NumLines2 = NumLine2AfterEnd;

	// build list of equal sections
	while (nLine1 < NumLines1
			&& nLine2 < NumLines2)
	{
		// find the beginning of the section
		// find a few identical lines
		TRACE("nLine1 = %d, nLine2 = %d, looking for identical section\n", nLine1, nLine2);
		int Line1Begin = nLine1;
		int Line2Begin = nLine2;
		// remember number of line with a found match and number of matched line
		int Line1Found = -1;
		int Line1MatchIn2 = NumLines2;  // number of line that matches it
		int Line2Found = -1;
		int Line2MatchIn1 = NumLines1;

		while (nLine1 < Line2MatchIn1
				&& nLine2 < Line1MatchIn2)
		{
			// Suppose, there are two pieces of code swapped. We need to find,
			// which file gives the nearest group of lines, that is less lines will be
			// reported as added or deleted.
			const FileLine * Line1 = pFirstFile->GetLine(nLine1);
			if ( ! Line1->IsBlank())
			{
				// check the lines in file2 in range Line2 to Line2+dist
				const FileLine * pFoundLine = NULL;
				if (UseLineGroups)
				{
					pFoundLine = pSecondFile->FindMatchingLineGroupLine(Line1, nLine2, Line1MatchIn2);
				}
				else
				{
					if (Line1->GetNormalizedLength() >= pApp->m_MinimalLineLength)
					{
						pFoundLine = pSecondFile->FindMatchingLine(Line1, nLine2, Line1MatchIn2);
					}
				}
				if (NULL != pFoundLine)
				{
					Line1MatchIn2 = pFoundLine->GetLineNumber();
					Line1Found = nLine1;
					TRACE("Found Line1=%d, Line2=%d, \"%s\"\n",
						Line1Found, Line1MatchIn2, pFoundLine->GetText());
				}
				// there is no equivalent line for this one from nLine2 to end
			}
			nLine1++;
			const FileLine * Line2 = pSecondFile->GetLine(nLine2);
			if ( ! Line2->IsBlank())
			{
				// check the lines in file1 in range Line2 to Line2+dist
				const FileLine * pFoundLine = NULL;
				if (UseLineGroups)
				{
					pFoundLine = pFirstFile->FindMatchingLineGroupLine(Line2, nLine1, Line2MatchIn1);
				}
				else
				{
					if (Line2->GetNormalizedLength() >= pApp->m_MinimalLineLength)
					{
						pFoundLine = pFirstFile->FindMatchingLine(Line2, nLine1, Line2MatchIn1);
					}
				}
				if (NULL != pFoundLine)
				{
					Line2MatchIn1 = pFoundLine->GetLineNumber();
					Line2Found = nLine2;
					TRACE("Found Line2=%d, Line1=%d, \"%s\"\n",
						Line2Found, Line2MatchIn1, pFoundLine->GetText());
				}
			}
			nLine2++;
		}
		// there are two pairs of lines found
		// choose one
		if (-1 == Line1Found)
		{
			if (-1 == Line2Found)
			{
				// no identical lines found
				TRACE("No more identical lines found\n");
				nLine1 = NumLines1;
				nLine2 = NumLines2;
			}
			else
			{
				nLine1 = Line2MatchIn1;
				nLine2 = Line2Found;
				TRACE("Found first line %d in file2 and matching line %d in file1\n",
					nLine2, nLine1);
			}
		}
		else if (-1 == Line2Found)
		{
			nLine2 = Line1MatchIn2;
			nLine1 = Line1Found;
			TRACE("Found first line %d in file1 and matching line %d in file2\n",
				nLine1, nLine2);
		}
		else
		{
			TRACE("Found first line %d in file1 and matching line %d in file2\n",
				Line1Found, Line1MatchIn2);
			TRACE("Found first line %d in file2 and matching line %d in file1\n",
				Line2Found, Line2MatchIn1);
			// choose the one with less distance
			if (Line1MatchIn2 - Line2Found < Line2MatchIn1 - Line1Found)
			{
				nLine1 = Line1Found;
				nLine2 = Line1MatchIn2;
			}
			else
			{
				nLine1 = Line2MatchIn1;
				nLine2 = Line2Found;
			}
			TRACE("Chosen line %d in file1 and line %d in file2\n",
				nLine1, nLine2);
		}
		Line1Begin = nLine1;
		Line2Begin = nLine2;

		while (nLine1 < NumLines1
				&& nLine2 < NumLines2)
		{
			const FileLine * Line1 = pFirstFile->GetLine(nLine1);
			const FileLine * Line2 = pSecondFile->GetLine(nLine2);
			if (Line1->IsNormalizedEqual(Line2))
			{
				nLine1++;
				nLine2++;
			}
			else
			{
				// the lines are different
				TRACE("Difference found at lines %d, %d\n", nLine1, nLine2);
				// check if the lines are similar enough
				// the lines can be considered similar if < 1/4 of the characters is different,
				// or the only difference is in whitespaces
				if (0 && Line1->LooksLike(Line2, 15))
				{
					nLine1++;
					nLine2++;
				}
				else
				{
					break;
				}
			}
		}

		pSection = new FileSection;
		if (NULL != pSection)
		{
			pSection->File1LineBegin = Line1Begin;

			pSection->File2LineBegin = Line2Begin;
			pSection->File1LineEnd = nLine1;
			pSection->File2LineEnd = nLine2;
			pSection->pNext = NULL;
			if (pLastSection == NULL)
			{
				pFirstSection = pSection;
			}
			else
			{
				pLastSection->pNext = pSection;
			}
			pLastSection = pSection;
		}
	}
	return pFirstSection;
}
FilePair::eFileComparisionResult FilePair::CompareBinaryFiles(BOOL volatile & bStopOperation)
{
	CThisApp * pApp = GetApp();
	return ResultUnknown;
}

FilePair::eFileComparisionResult FilePair::PreCompareBinaryFiles(BOOL volatile & bStopOperation)
{
	CThisApp * pApp = GetApp();
	// comparison can be done through CRC, or direct comparison
	// if length is different, return it:
	if (pFirstFile->m_Length != pSecondFile->m_Length)
	{
		if (pFirstFile->m_Length > pSecondFile->m_Length)
		{
			return FirstFileLonger;
		}
		else
		{
			return SecondFileLonger;
		}
	}
	if (pApp->m_bUseMd5)
	{
		if ( ! pFirstFile->m_bMd5Calculated)
		{
			pFirstFile->CalculateHashes(bStopOperation);
		}
		if ( ! pSecondFile->m_bMd5Calculated)
		{
			pSecondFile->CalculateHashes(bStopOperation);
		}
	}
	// if files have MD5 calculated, use it
	if (pFirstFile->m_bMd5Calculated
		&& pSecondFile->m_bMd5Calculated)
	{
		if (memcmp(pFirstFile->m_Md5, pSecondFile->m_Md5,
					sizeof pFirstFile->m_Md5))
		{
			return FilesDifferent;
		}
		return FilesIdentical;
	}

	return ResultUnknown;
}

FilePair::eFileComparisionResult FilePair::CompareTextFiles(BOOL volatile & bStopOperation)
{
	// find similar lines
	CThisApp * pApp = GetApp();
	FileSection * pSection;
	FileSection * pFirstSection =
		BuildSectionList(0, pFirstFile->GetNumLines(),
						0, pSecondFile->GetNumLines(), true);
	// if the files don't start from identical lines, add an empty section
	if (NULL == pFirstSection
		|| (pFirstSection->File1LineBegin != 0
			&& pFirstSection->File2LineBegin != 0))
	{
		pSection = new FileSection;

		if (NULL != pSection)
		{
			pSection->File1LineBegin = 0;
			pSection->File2LineBegin = 0;

			pSection->File1LineEnd = 0;
			pSection->File2LineEnd = 0;

			pSection->pNext = pFirstSection;
			pFirstSection = pSection;
		}
	}
	// add the final section
	pSection = pFirstSection;
	while (NULL != pSection->pNext)
	{
		pSection = pSection->pNext;
	}
	if (pSection->File1LineEnd < pFirstFile->GetNumLines()
		|| pSection->File2LineEnd < pSecondFile->GetNumLines())
	{
		FileSection * pLastSection = new FileSection;
		if (NULL != pLastSection)
		{
			pLastSection->File1LineBegin = pFirstFile->GetNumLines();
			pLastSection->File1LineEnd = pLastSection->File1LineBegin;

			pLastSection->File2LineBegin = pSecondFile->GetNumLines();
			pLastSection->File2LineEnd = pLastSection->File2LineBegin;

			pSection->pNext = pLastSection;
			pLastSection->pNext = NULL;
		}
	}
	// try to match single lines inside the difference areas, but limit the lookup
	int nPrevSectionEnd1 = 0;
	int nPrevSectionEnd2 = 0;
	FileSection * pPrevSection = NULL;
	for (pSection = pFirstSection; pSection != NULL; pPrevSection = pSection, pSection = pSection->pNext)
	{
		if (pSection->File1LineBegin > nPrevSectionEnd1
			&& pSection->File2LineBegin > nPrevSectionEnd2)
		{
			FileSection * pMoreSection = BuildSectionList
										(nPrevSectionEnd1, pSection->File1LineBegin,
											nPrevSectionEnd2, pSection->File2LineBegin, false);
			if (NULL != pMoreSection)
			{
				// check that there is enough equal lines
				int nTotalLines = pSection->File1LineBegin - nPrevSectionEnd1
								+ pSection->File2LineBegin - nPrevSectionEnd2;
				int nTotalEqualLines = 0;
				for (FileSection * pTmpSection = pMoreSection; pTmpSection != NULL; pTmpSection = pTmpSection->pNext)
				{
					nTotalEqualLines += pTmpSection->File1LineEnd - pTmpSection->File1LineBegin
										+ pTmpSection->File2LineEnd - pTmpSection->File2LineBegin;
				}

				if (nTotalEqualLines >= nTotalLines * pApp->m_MinPercentWeakIdenticalLines / 100)
				{
					// insert to the list
					if (pPrevSection != NULL)
					{
						pPrevSection->pNext = pMoreSection;
					}
					else
					{
						pFirstSection = pMoreSection;
					}
					// find the last item
					while (NULL != pMoreSection->pNext)
					{
						pMoreSection = pMoreSection->pNext;
					}
					pMoreSection->pNext = pSection;
				}
				else
				{
					// free them
				}
			}
		}

		nPrevSectionEnd1 = pSection->File1LineEnd;
		nPrevSectionEnd2 = pSection->File2LineEnd;
	}

	// scan list of sections and try to expand them downwards with looking like and blank lines
	nPrevSectionEnd1 = 0;
	nPrevSectionEnd2 = 0;
	for (pSection = pFirstSection; pSection != NULL; pSection = pSection->pNext)
	{
		while (1)
		{
			if (pSection->File1LineBegin > nPrevSectionEnd1
				&& pFirstFile->GetLine(pSection->File1LineBegin - 1)->IsBlank())
			{
				pSection->File1LineBegin--;
				continue;
			}
			if (pSection->File2LineBegin > nPrevSectionEnd2
				&& pSecondFile->GetLine(pSection->File2LineBegin - 1)->IsBlank())
			{
				pSection->File2LineBegin--;
				continue;
			}
			if (pSection->File1LineBegin > nPrevSectionEnd1
				&& pSection->File2LineBegin > nPrevSectionEnd2
				&& pFirstFile->GetLine(pSection->File1LineBegin - 1)->
				LooksLike(pSecondFile->GetLine(pSection->File2LineBegin - 1),
						pApp->m_PercentsOfLookLikeDifference))
			{
				// expand the section down, to include alike lines
				pSection->File1LineBegin--;
				pSection->File2LineBegin--;
			}
			else
			{
				break;
			}
		}

		nPrevSectionEnd1 = pSection->File1LineEnd;
		nPrevSectionEnd2 = pSection->File2LineEnd;
	}
	// concatenate adjacent sections
	for (pSection = pFirstSection; pSection != NULL && pSection->pNext != NULL; )
	{
		FileSection * pNext = pSection->pNext;
		if (pSection->File1LineEnd != pNext->File1LineBegin
			|| pSection->File2LineEnd != pNext->File2LineBegin)
		{
			pSection = pNext;
			continue;
		}
		pSection->File1LineEnd = pNext->File1LineEnd;
		pSection->File2LineEnd = pNext->File2LineEnd;
		pSection->pNext = pNext->pNext;
		delete pNext;
	}

	// scan list of sections and try to expand them upwards with looking like lines
	for (pSection = pFirstSection; pSection != NULL && pSection->pNext != NULL; pSection = pSection->pNext)
	{
		int nNextSectionBegin1 = pSection->pNext->File1LineBegin;
		int nNextSectionBegin2 = pSection->pNext->File2LineBegin;

		while (1)
		{
			if (pSection->File1LineEnd < nNextSectionBegin1
				&& pFirstFile->GetLine(pSection->File1LineEnd)->IsBlank())
			{
				pSection->File1LineEnd++;
				continue;
			}
			if (pSection->File2LineEnd < nNextSectionBegin2
				&& pSecondFile->GetLine(pSection->File2LineEnd)->IsBlank())
			{
				pSection->File2LineEnd++;
				continue;
			}

			if (pSection->File1LineEnd < nNextSectionBegin1
				&& pSection->File2LineEnd < nNextSectionBegin2
				&& pFirstFile->GetLine(pSection->File1LineEnd)->
				LooksLike(pSecondFile->GetLine(pSection->File2LineEnd),
						pApp->m_PercentsOfLookLikeDifference))
			{
				// expand the section up, to include alike lines
				pSection->File1LineEnd++;
				pSection->File2LineEnd++;
			}
			else
			{
				break;
			}
		}

	}
	// build the array of line pairs
	// calculate number of line pairs
	nPrevSectionEnd1 = 0;
	nPrevSectionEnd2 = 0;
	int nTotalLines = 0;
	for (pSection = pFirstSection; pSection != NULL; pSection = pSection->pNext)
	{
		nTotalLines += pSection->File1LineEnd - nPrevSectionEnd1
						+ pSection->File2LineBegin - nPrevSectionEnd2;
		nPrevSectionEnd1 = pSection->File1LineEnd;
		nPrevSectionEnd2 = pSection->File2LineEnd;
	}
	// build line pair array
	m_LinePairs.resize(nTotalLines);
	nPrevSectionEnd1 = 0;
	nPrevSectionEnd2 = 0;
	unsigned nLineIndex = 0;
	for (pSection = pFirstSection; pSection != NULL; pSection = pSection->pNext)
	{
		int i;
		LinePair * pPair;
		// add lines from first file (mark as removed)
		if (pSection->File1LineBegin > nPrevSectionEnd1)
		{
			FileDiffSection * pDiffSection = new FileDiffSection;
			if (NULL != pDiffSection)
			{
				pDiffSection->m_Begin.line = nLineIndex;
				pDiffSection->m_Begin.pos = 0;
				pDiffSection->m_End.line = nLineIndex + pSection->File1LineBegin - nPrevSectionEnd1;
				pDiffSection->m_End.pos = 0;
				pDiffSection->m_Flags |= FileDiffSection::FlagWhitespace;
				m_DiffSections.push_back(pDiffSection);
			}
			for (i = nPrevSectionEnd1; i < pSection->File1LineBegin; i++, nLineIndex++)
			{
				pPair = new LinePair;
				if (NULL == pPair)
				{
					break;
				}
				if (m_LinePairs.size() <= nLineIndex)
				{
					m_LinePairs.resize(nLineIndex+1);
				}
				m_LinePairs[nLineIndex] = pPair;

				pPair->pFirstLine = pFirstFile->GetLine(i);
				pPair->pSecondLine = NULL;

				StringSection * pSection = new StringSection;
				pPair->pFirstSection = pSection;
				if (NULL == pSection)
				{
					break;
				}
				pSection->pNext = NULL;
				pSection->Attr = StringSection::Erased | StringSection::Undefined;
				pSection->pBegin = pPair->pFirstLine->GetText();
				pSection->Length = pPair->pFirstLine->GetLength();
				pSection->pDiffSection = pDiffSection;
				if (pPair->pFirstLine->IsBlank())
				{
					pSection->Attr |= StringSection::Whitespace;
				}
				else if (NULL != pDiffSection)
				{
					pDiffSection->m_Flags &= ~FileDiffSection::FlagWhitespace;
				}
			}
		}
		if (pSection->File2LineBegin > nPrevSectionEnd2)
		{
			FileDiffSection * pDiffSection = new FileDiffSection;
			if (NULL != pDiffSection)
			{
				pDiffSection->m_Begin.line = nLineIndex;
				pDiffSection->m_Begin.pos = 0;
				pDiffSection->m_End.line = nLineIndex + pSection->File2LineBegin - nPrevSectionEnd2;
				pDiffSection->m_End.pos = 0;
				pDiffSection->m_Flags |= FileDiffSection::FlagWhitespace;
				m_DiffSections.push_back(pDiffSection);
			}
			// add lines from second file (mark as added)
			for (i = nPrevSectionEnd2; i < pSection->File2LineBegin; i++, nLineIndex++)
			{
				pPair = new LinePair;
				if (NULL == pPair)
				{
					break;
				}
				if (m_LinePairs.size() <= nLineIndex)
				{
					m_LinePairs.resize(nLineIndex+1);
				}
				m_LinePairs[nLineIndex] = pPair;

				pPair->pFirstLine = NULL;
				pPair->pSecondLine = pSecondFile->GetLine(i);

				StringSection * pSection = new StringSection;
				pPair->pFirstSection = pSection;
				if (NULL == pSection)
				{
					break;
				}
				pSection->pNext = NULL;
				pSection->Attr = StringSection::Inserted | StringSection::Undefined;
				pSection->pBegin = pPair->pSecondLine->GetText();
				pSection->Length = pPair->pSecondLine->GetLength();
				pSection->pDiffSection = pDiffSection;
				if (pPair->pSecondLine->IsBlank())
				{
					pSection->Attr |= StringSection::Whitespace;
				}
				else if (NULL != pDiffSection)
				{
					pDiffSection->m_Flags &= ~FileDiffSection::FlagWhitespace;
				}
			}

		}
		int line1 = pSection->File1LineBegin;
		int line2 = pSection->File2LineBegin;
		for ( ; line1 < pSection->File1LineEnd || line2 < pSection->File2LineEnd; nLineIndex++)
		{
			pPair = new LinePair;
			if (NULL == pPair)
			{
				break;
			}
			if (m_LinePairs.size() <= nLineIndex)
			{
				m_LinePairs.resize(nLineIndex+1);
			}
			m_LinePairs[nLineIndex] = pPair;

			pPair->pFirstSection = NULL;

			if (line1 < pSection->File1LineEnd)
			{
				pPair->pFirstLine = pFirstFile->GetLine(line1);
			}
			else
			{
				pPair->pFirstLine = NULL;
			}

			if (line2 < pSection->File2LineEnd)
			{
				pPair->pSecondLine = pSecondFile->GetLine(line2);
			}
			else
			{
				pPair->pSecondLine = NULL;
			}

			// if only one of the strings is blank, make a single-line entry
			if (NULL != pPair->pFirstLine
				&& pPair->pFirstLine->IsBlank())
			{
				if (NULL == pPair->pSecondLine
					|| ! pPair->pSecondLine->IsBlank())
				{
					pPair->pSecondLine = NULL;
				}
			}

			if (NULL != pPair->pSecondLine
				&& pPair->pSecondLine->IsBlank())
			{
				if (NULL == pPair->pFirstLine
					|| ! pPair->pFirstLine->IsBlank())
				{
					pPair->pFirstLine = NULL;
				}
			}
			if (NULL != pPair->pFirstLine)
			{
				line1++;
			}

			if (NULL != pPair->pSecondLine)
			{
				line2++;
			}

			MatchStrings(pPair->pFirstLine, pPair->pSecondLine,
						& pPair->pFirstSection, pApp->m_MinMatchingChars);

			int pos = 0;
			for (StringSection * pSection = pPair->pFirstSection; pSection != NULL; pSection = pSection->pNext)
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
						if (pSection->pNext != NULL
							&& pSection->pNext->Attr != pSection->Identical)
						{
							if (0 == (pSection->pNext->Attr & pSection->Whitespace))
							{
								pDiffSection->m_Flags &= ~FileDiffSection::FlagWhitespace;
							}

							pSection = pSection->pNext;
							pSection->pDiffSection = pDiffSection;
							pos += pSection->Length;
							pDiffSection->m_End.pos = pos;
						}
						m_DiffSections.push_back(pDiffSection);
					}
				}
				else
				{
					pSection->pDiffSection = NULL;
					pos += pSection->Length;
				}
			}
		}
		nPrevSectionEnd1 = pSection->File1LineEnd;
		nPrevSectionEnd2 = pSection->File2LineEnd;
	}
	//ASSERT(nLineIndex == nTotalLines);
	m_LinePairs.resize(nLineIndex);
	// deallocate the sections, don't need them anymore
	for (pSection = pFirstSection; pSection != NULL; )
	{
		FileSection * tmp = pSection;
		pSection = pSection->pNext;
		delete tmp;
	}
	return FilesDifferent;
}

inline int CompareTextPosBegin(const TextPos * pos,  FileDiffSection * const *sec, int)
{
	if (*pos < (*sec)->m_Begin)
	{
		return -1;
	}
	return 1;
}

inline int CompareTextPosEnd(const TextPos * pos,  FileDiffSection * const *sec, int)
{
	if (*pos < (*sec)->m_End)
	{
		return -1;
	}
	return 1;
}

bool FilePair::NextDifference(TextPos PosFrom, BOOL IgnoreWhitespaces,
							TextPos * DiffPos, TextPos * EndPos)
{
	if (m_DiffSections.empty())
	{
		return FALSE;
	}
	FileDiffSection diff;
	diff.m_Begin = PosFrom;
	vector<FileDiffSection *>::iterator pFound = upper_bound(m_DiffSections.begin(),
															m_DiffSections.end(), & diff, less<FileDiffSection *>());

	if (pFound >= m_DiffSections.end())
	{
		return FALSE;
	}
	const FileDiffSection * pSection = *pFound;

	if (IgnoreWhitespaces)
	{
		while (pSection->m_Flags & pSection->FlagWhitespace)
		{
			pFound++;
			if (pFound >= m_DiffSections.end())
			{
				return FALSE;
			}
			pSection = *pFound;
		}
	}
	if (NULL != DiffPos)
	{
		*DiffPos = pSection->m_Begin;
	}
	if (NULL != EndPos)
	{
		*EndPos = pSection->m_End;
	}
	return TRUE;
}

bool FilePair::PrevDifference(TextPos PosFrom, BOOL IgnoreWhitespaces,
							TextPos * DiffPos, TextPos * EndPos)
{
	if (m_DiffSections.empty())
	{
		return FALSE;
	}
	FileDiffSection diff;
	diff.m_Begin = PosFrom;
	vector<FileDiffSection *>::iterator pFound = lower_bound(m_DiffSections.begin(),
															m_DiffSections.end(), & diff, less<FileDiffSection *>());
	if (pFound == m_DiffSections.begin())
	{
		return FALSE;
	}
	pFound--;
	const FileDiffSection * pSection = *pFound;
	if (IgnoreWhitespaces)
	{
		while (pSection->m_Flags & pSection->FlagWhitespace)
		{
			if (pFound == m_DiffSections.begin())
			{
				return FALSE;
			}
			pFound--;
			pSection = *pFound;
		}
	}
	if (NULL != DiffPos)
	{
		*DiffPos = pSection->m_Begin;
	}
	if (NULL != EndPos)
	{
		*EndPos = pSection->m_End;
	}
	return TRUE;
}

struct ModifyFlagsStruct
{
	int Set;
	int Reset;
};
void FilePair::ModifyAcceptDeclineFlagsFunc(StringSection * pSection, void * Param)
{
	ModifyFlagsStruct * pmfs = (ModifyFlagsStruct *) Param;
	pSection->Attr &= ~pmfs->Reset;
	pSection->Attr |= pmfs->Set;
}

BOOL FilePair::ModifyAcceptDeclineFlags(TextPos & PosFrom, TextPos & PosTo, int Set, int Reset)
{
	ModifyFlagsStruct mfs;
	mfs.Set = Set;
	mfs.Reset = Reset;
	return EnumStringDiffSections(PosFrom, PosTo, ModifyAcceptDeclineFlagsFunc, & mfs);
}

struct GetFlagsStruct
{
	int Set;
	int Reset;
	BOOL bIgnoreWhitespace;
};

void FilePair::GetAcceptDeclineFlagsFunc(StringSection * pSection, void * Param)
{
	GetFlagsStruct * gfs = (GetFlagsStruct *) Param;
	if (pSection->IsWhitespace() && gfs->bIgnoreWhitespace)
	{
		return;
	}
	gfs->Set |= pSection->Attr;
	gfs->Reset &= pSection->Attr;
}

int FilePair::GetAcceptDeclineFlags(TextPos PosFrom, TextPos PosTo, bool bIgnoreWhitespaces)
{
	GetFlagsStruct gfs;
	gfs.bIgnoreWhitespace = bIgnoreWhitespaces;
	gfs.Set = 0;
	gfs.Reset = ~0; // all ones
	EnumStringDiffSections(PosFrom, PosTo, GetAcceptDeclineFlagsFunc, & gfs);
	if (~0 == gfs.Reset)
	{
		return StringSection::NoDifference;
	}
	return gfs.Set;
}

BOOL FilePair::EnumStringDiffSections(TextPos & PosFrom, TextPos & PosTo,
									void (* Func)(StringSection * pSection, void * Param), void * pParam)
{
	TextPos begin = PosFrom, end = PosTo;
	if (begin == end)
	{
		// if the range is of zero length, then modify all neighbor sections
		if (begin.line >= (int)m_LinePairs.size())
		{
			return FALSE;
		}
		LinePair * pPair = m_LinePairs[begin.line];
		// find first inclusive section

		int pos = 0;
		StringSection * pSection = pPair->pFirstSection;
		if (pSection != NULL && NULL == pSection->pNext)
		{
			// it's the only section
			if (0 == (pSection->Attr & (pSection->Inserted | pSection->Erased)))
			{
				pSection = NULL;
			}
		}
		else
		{
			for ( ; pSection != NULL; pos += pSection->Length, pSection = pSection->pNext)
			{
				if (pos > begin.pos)
				{
					return FALSE;
				}
				if (0 == (pSection->Attr & (pSection->Inserted | pSection->Erased)))
				{
					continue;
				}
				if (pos + pSection->Length > begin.pos
					|| (NULL != pSection->pNext
						&& 0 != (pSection->pNext->Attr & (pSection->Inserted | pSection->Erased))
						&& pos + pSection->Length + pSection->pNext->Length > begin.pos))
				{
					// section found
					break;
				}
			}
		}
		if (NULL == pSection)
		{
			return FALSE;
		}
		// if the found section is the only section of the line, check if the previous lines are same way
		if (pSection == pPair->pFirstSection
			&& NULL == pSection->pNext)
		{
			begin.pos = 0;
			end.pos = 0;
			end.line = begin.line + 1;

			Func(pSection, pParam);

			while (begin.line > 0)
			{
				pSection = m_LinePairs[begin.line - 1]->pFirstSection;
				if (NULL == pSection
					|| NULL != pSection->pNext
					|| 0 == (pSection->Attr & (pSection->Inserted | pSection->Erased)))
				{
					break;
				}
				Func(pSection, pParam);

				begin.line--;
			}
			// check if it can be expanded down
			while (end.line < (int)m_LinePairs.size())
			{
				pSection = m_LinePairs[end.line]->pFirstSection;
				if (NULL == pSection
					|| NULL != pSection->pNext
					|| 0 == (pSection->Attr & (pSection->Inserted | pSection->Erased)))
				{
					break;
				}
				Func(pSection, pParam);

				end.line++;
			}
		}
		else
		{
			begin.pos = pos;
			end.pos = pos + pSection->Length;

			Func(pSection, pParam);

			pSection = pSection->pNext;
			if (NULL != pSection
				&& (pSection->Attr & (pSection->Inserted | pSection->Erased)))
			{
				end.pos += pSection->Length;

				Func(pSection, pParam);
			}
		}
		PosFrom = begin;
		PosTo = end;
		return TRUE;
	}
	// if the range is not of zero length, then modify all included
	if (begin > end)
	{
		TextPos tmp = end;
		end = begin;
		begin = tmp;
	}
	if (begin.line >= (int)m_LinePairs.size())
	{
		return FALSE;
	}
	LinePair * pPair = m_LinePairs[begin.line];
	// find first inclusive section

	int pos = 0;
	StringSection * pSection;
	for (pSection = pPair->pFirstSection
		; pSection != NULL; pos += pSection->Length, pSection = pSection->pNext)
	{
		if (end.line == begin.line
			&& pos >= end.pos)
		{
			// no section in this line
			pSection = NULL;
			break;
		}
		if (0 != (pSection->Attr & (pSection->Inserted | pSection->Erased))
			&& (pos + pSection->Length > begin.pos
				|| (pos == begin.pos && 0 == pSection->Length)))
		{
			// section found
			begin.pos = pos;
			break;
		}
	}
	TextPos ChangeEnd = begin;
	for ( ; pSection != NULL; pos += pSection->Length, pSection = pSection->pNext)
	{
		if (end.line == begin.line
			&& pos >= end.pos)
		{
			// no more sections to mark
			break;
		}
		if (0 != (pSection->Attr & (pSection->Inserted | pSection->Erased)))
		{
			Func(pSection, pParam);
			ChangeEnd.pos = pos + pSection->Length;
			if (0 == pSection->Length)
			{
				ChangeEnd.pos++;
			}
		}
	}
	int line;
	for (line = begin.line + 1; line < (int)m_LinePairs.size() && line <= end.line; line++)
	{
		for (pos = 0, pSection = m_LinePairs[line]->pFirstSection
			; pSection != NULL; pSection = pSection->pNext)
		{
			if (line == end.line
				&& pos >= end.pos)
			{
				break;
			}
			pos += pSection->Length;
			if (0 != (pSection->Attr & (pSection->Inserted | pSection->Erased)))
			{
				Func(pSection, pParam);
				ChangeEnd.line = line;
				ChangeEnd.pos = pos;
				if (0 == pSection->Length)
				{
					ChangeEnd.pos++;
				}
			}
		}
	}
	if (begin == ChangeEnd)
	{
		return FALSE;
	}
	PosFrom = begin;
	PosTo = ChangeEnd;
	return TRUE;
}

LPCTSTR LinePair::GetText(LPTSTR buf, const size_t nBufChars, int * pStrLen, BOOL IgnoreWhitespaces)
{
	if (NULL == pFirstLine)
	{
		*pStrLen = pSecondLine->GetLength();
		return pSecondLine->GetText();
	}
	else if (NULL == pSecondLine)
	{
		*pStrLen = pFirstLine->GetLength();
		return pFirstLine->GetText();
	}
	else
	{
		// make a string of string sections
		int StrLen = 0;
		for (StringSection * pSection = pFirstSection
			; pSection != NULL && StrLen + 1u < nBufChars; pSection = pSection->pNext)
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
			_tcsncpy(buf + StrLen, pSection->pBegin, len);
			StrLen += len;
		}
		buf[StrLen] = 0;
		*pStrLen = StrLen;
		return buf;
	}

}

int LinePair::LinePosToDisplayPos(int position, BOOL bIgnoreWhitespaces)
{
	if ( ! bIgnoreWhitespaces)
	{
		return position;
	}
	int pos = 0;
	int adj = 0;

	for (StringSection * pSection = pFirstSection; NULL != pSection; pSection = pSection->pNext)
	{
		pos += pSection->Length;
		if ((pSection->Attr & pSection->Whitespace)
			&& (pSection->Attr & pSection->Erased))
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

int LinePair::DisplayPosToLinePos(int position, BOOL bIgnoreWhitespaces)
{
	if ( ! bIgnoreWhitespaces)
	{
		return position;
	}
	int pos = 0;
	int adj = 0;

	for (StringSection * pSection = pFirstSection; NULL != pSection; pSection = pSection->pNext)
	{
		if ((pSection->Attr & pSection->Whitespace)
			&& (pSection->Attr & pSection->Erased))
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

BOOL FileItem::InitHashCalculation()
{
	DeinitHashCalculation();
	m_HashBuf = (PBYTE)VirtualAlloc(NULL, 0x10000, MEM_COMMIT, PAGE_READWRITE);
	CryptAcquireContext( & m_CryptProvider, NULL, MS_DEF_PROV, PROV_RSA_FULL,
						CRYPT_VERIFYCONTEXT);
	return NULL != m_HashBuf
			&& NULL != m_CryptProvider;
}

void FileItem::DeinitHashCalculation()
{
	if (NULL != m_CryptProvider)
	{
		CryptReleaseContext(m_CryptProvider, 0);
		m_CryptProvider = NULL;
	}
	if (NULL != m_HashBuf)
	{
		VirtualFree(m_HashBuf, 0, MEM_RELEASE);
		m_HashBuf = NULL;
	}
}

BOOL FileItem::CalculateHashes(BOOL volatile & bStopOperation)
{
	if (NULL == m_HashBuf
		|| NULL == m_CryptProvider)
	{
		return FALSE;
	}
	HCRYPTHASH hash = NULL;
	HANDLE hFile = NULL;
	BOOL res = TRUE;
	hFile = CreateFile(GetFullName(), GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	if (CryptCreateHash(m_CryptProvider, CALG_MD5, NULL, NULL, & hash))
	{
		for (ULONGLONG BytesLeft = m_Length; BytesLeft != 0 && ! bStopOperation; )
		{
			DWORD ToRead = 0x10000;
			if (ToRead > BytesLeft)
			{
				ToRead = BytesLeft;
			}
			DWORD BytesRead;
			if ( ! ReadFile(hFile, m_HashBuf, 0x10000, & BytesRead, NULL)
				|| BytesRead != ToRead)
			{
				res = FALSE;
				break;
			}
			CryptHashData(hash, m_HashBuf, BytesRead, 0);
			BytesLeft -= BytesRead;
		}
		DWORD HashLen = 20;
		CryptGetHashParam(hash, HP_HASHVAL, m_Md5, & HashLen, 0);
		CryptDestroyHash(hash);
	}
	CloseHandle(hFile);
	if (res)
	{
		m_bMd5Calculated = true;
	}
	return res;
}

CSmallAllocator FileLine::m_Allocator(sizeof FileLine);
CSmallAllocator StringSection::m_Allocator(sizeof StringSection);
CSmallAllocator LinePair::m_Allocator(sizeof LinePair);
CSmallAllocator FileDiffSection::m_Allocator(sizeof FileDiffSection);

//CSimpleCriticalSection FileItem::m_Cs;
BYTE * FileItem::m_HashBuf = NULL;
HCRYPTPROV FileItem::m_CryptProvider = NULL;
