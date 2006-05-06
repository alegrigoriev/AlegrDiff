// FileListSupport.cpp
#include "stdafx.h"
#include "FileListSupport.h"
#include "AlegrDiff.h"
#include "FileLine.h"
#include <io.h>
#include <fcntl.h>
#include "SmallAllocator.h"
#include <algorithm>
#include "ProgressDialog.h"
#include "LastError.h"

#ifdef _DEBUG
#include <mmsystem.h>
#endif

#undef tolower
#undef toupper
static DWORD CalculateHash(void const * data, int len);

bool less<FileDiffSection *>::operator()
	(FileDiffSection * const & pS1, FileDiffSection * const & pS2) const
{
	return pS1->m_Begin < pS2->m_Begin;
}

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
	// find string length
	int len;
	// limit the string length to 64K
	for (len = 0; ('\0' != pMsz[len] || '\0' != pMsz[len + 1]) && len < 65536; len++)
	{
	}
	return CString(pMsz, len + 1);
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
	// check if 'name' contains a '.' character
	// if not, add one in a temporary buffer
	TCHAR TmpBuf[MAX_PATH + 2];
	if (name[0] != 0
		&& NULL == _tcschr(name, '.'))
	{
		_tcsncpy(TmpBuf, name, MAX_PATH);
		TmpBuf[MAX_PATH] = 0;
		_tcscat(TmpBuf, _T("."));
		name = TmpBuf;
	}

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

FileItem::FileItem(const WIN32_FIND_DATA * pWfd,
					const CString & BaseDir, const CString & Dir, FileItem * pParentDir)
	:m_Name(pWfd->cFileName)
	, m_Length(pWfd->nFileSizeLow + (LONGLONG(pWfd->nFileSizeHigh) << 32))
	, m_BaseDir(BaseDir)
	, m_Subdir(Dir)
	, m_C_Cpp(false)
	, m_IsBinary(false)
	, m_IsUnicode(false)
	, m_IsUnicodeBigEndian(false)
	, m_bMd5Calculated(false)
	, m_bIsPhantomFile(false)
	, m_bIsAlone(false)
	, m_pFileReadBuf(NULL)
	, m_FileReadBufSize(0)
	, m_FileReadPos(0)
	, m_FileReadFilled(0)
	, m_Attributes(pWfd->dwFileAttributes)
	, m_hFile(NULL)
	, m_pNext(NULL)
	, m_pParentDir(pParentDir)
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
	m_bIsPhantomFile(false),
	m_pFileReadBuf(NULL)
	, m_bIsAlone(false)
	, m_FileReadBufSize(0)
	, m_Attributes(0)
	, m_FileReadPos(0)
	, m_FileReadFilled(0)
	, m_hFile(NULL)
	, m_pNext(NULL)
	, m_pParentDir(NULL)
{
	memzero(m_Md5);
	m_LastWriteTime.dwHighDateTime = 0;
	m_LastWriteTime.dwLowDateTime = 0;
}

FileItem::~FileItem()
{
	Unload();
}

void FileItem::Unload()
{
	if (0) TRACE(_T("FileItem %s Unloaded\n"), LPCTSTR(GetFullName()));

	CSimpleCriticalSectionLock lock(m_Cs);

	for (unsigned i = 0; i < m_Lines.size(); i++)
	{
		delete m_Lines[i];
	}
	m_Lines.clear();
	m_NonBlankLines.clear();

	m_NormalizedHashSortedLines.clear();
	m_NormalizedHashSortedLineGroups.clear();

	if (NULL != m_hFile)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
	}

	if (NULL != m_pFileReadBuf)
	{
		VirtualFree(m_pFileReadBuf, 0, MEM_RELEASE);
		m_pFileReadBuf = NULL;
		m_FileReadBufSize = 0;
		m_FileReadFilled = 0;
	}
}

void FileItem::AddLine(LPCTSTR pLine)
{
	FileLine * pFileLine = new FileLine(pLine, _tcslen(pLine), true, m_C_Cpp);
	if (pLine)
	{
		pFileLine->SetLineNumber(m_Lines.size());
		m_Lines.insert(m_Lines.end(), pFileLine);
	}
}

int fngets(char *string, int count, FILE *str)
{
	int i;
	if (0 == count)
	{
		return 0;
	}
	for (i = 0; i < count - 1; i++)
	{
		int ch = getc(str);
		if (ch == EOF)
		{
			break;
		}

		if (0 == ch)
		{
			ch = 0x01;
		}

		string[i] = char(ch);

		if (ch == '\n')
		{
			i++;
			break;
		}
	}

	string[i] = 0;
	return i;
}

int fngetws(wchar_t *string, int count, FILE *str, BOOL bBigEndian)
{
	int i;
	if (0 == count)
	{
		return 0;
	}

	for (i = 0; i < count - 1; i++)
	{
		int ch = getwc(str);
		if (ch == WEOF)
		{
			break;
		}

		if (bBigEndian)
		{
			ch = ((ch & 0xFF) << 8) | ((ch >> 8) & 0xFF);
		}

		if (0 == ch)
		{
			ch = 0x0001;
		}

		string[i] = wchar_t(ch);

		if (ch == '\n')
		{
			i++;
			break;
		}
	}

	string[i] = 0;
	return i;
}

static int NormalizedHashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2)
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

static int NormalizedGroupHashAndLineNumberCompareFunc(FileLine const * pLine1, FileLine const * pLine2)
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
	FILE * file = _tfopen(LPCTSTR(GetFullName()), _T("rb"));
	if (NULL == file)
	{
		return false;
	}
	char lineA[4096];
	wchar_t lineW[4096];
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
	TCHAR TabExpandedLine[4096];

	FileLine * pLineList = NULL;
	// peek two first bytes, to see if it is UNICODE file
	wchar_t FirstChar = fgetwc(file);

	clearerr(file);

	if ((FirstChar & 0xFFFF) == 0xFEFF)
	{
		m_IsUnicode = true;
	}
	else if ((FirstChar & 0xFFFF) == 0xFFFE)
	{
		m_IsUnicode = true;
		m_IsUnicodeBigEndian = true;
	}
	else
	{
		rewind(file);
		_setmode(_fileno(file), _O_TEXT);
	}

	for (unsigned LinNum =0; ; LinNum++)
	{
		if (m_IsUnicode)
		{
			if (0 == fngetws(lineW, countof(lineW), file, m_IsUnicodeBigEndian))
			{
				break;
			}
#ifndef _UNICODE
			wcstombs(lineA, lineW, countof(lineA)  - 1);
#endif
		}
		else
		{
			if (0 == fngets(lineA, countof(lineA), file))
			{
				break;
			}
#ifdef _UNICODE
			mbstowcs(lineW, lineA, countof(lineW) - 1);
#endif
		}
		// expand tabs
		for (int i = 0, pos = 0; line[i] != 0 && pos < countof(TabExpandedLine) - 1; pos++)
		{
			if (line[i] == '\t')
			{
				TabExpandedLine[pos] = ' ';
				if ((pos + 1) % pApp->m_TabIndent == 0)
				{
					i++;
				}
			}
			else if (line[i] == '\n' || line[i] == '\r')
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
		FileLine * pLine = new FileLine(TabExpandedLine, pos, true, m_C_Cpp);
		if (pLine)
		{
			pLine->SetNext(pLineList);
			pLineList = pLine;
		}
	}
	fclose(file);
#ifdef _DEBUG
	TRACE(_T("File %s loaded in %d ms\n"), LPCTSTR(GetFullName()), timeGetTime() - BeginTime);
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

		m_NonBlankLines[i]->CalculateGroupHash(GroupHash, j * sizeof GroupHash[0]);
		m_NonBlankLines[i]->CalculateNormalizedGroupHash(NormGroupHash, j * sizeof NormGroupHash[0]);
	}

	m_NormalizedHashSortedLines = m_NonBlankLines;
	std::sort(m_NormalizedHashSortedLines.begin(), m_NormalizedHashSortedLines.end(),
			NormalizedHashAndLineNumberCompareFunc);

	m_NormalizedHashSortedLineGroups = m_NonBlankLines;
	std::sort(m_NormalizedHashSortedLineGroups.begin(), m_NormalizedHashSortedLineGroups.end(),
			NormalizedGroupHashAndLineNumberCompareFunc);
#ifdef _DEBUG
	// check if the array is sorted correctly
	{
		unsigned i;
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
bool FileItem::DirNameSortFunc(FileItem const * Item1, FileItem const * Item2)
{
	return DirNameCompare(Item1, Item2) < 0;
}

bool FileItem::DirNameSortBackwardsFunc(FileItem const * Item1, FileItem const * Item2)
{
	return DirNameCompare(Item1, Item2) > 0;
}

int FileItem::DirNameCompare(FileItem const * Item1, FileItem const * Item2)
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
	if (Item1->GetSubdir() != Item2->GetSubdir())
	{
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
		int result = Item1->m_Subdir.CollateNoCase(Item2->m_Subdir);
		if (0 != result)
		{
			return result;
		}
	}
	// directory name is the same
	// compare file name. Subdirectories go first
	if (Item1->IsFolder() == Item2->IsFolder())
	{
		return Item1->m_Name.CollateNoCase(Item2->m_Name);
	}
	if (Item1->IsFolder())
	{
		return -1;
	}
	return 1;
}

// sort names first then directories
bool FileItem::NameSortFunc(FileItem const * Item1, FileItem const * Item2)
{
	return NameCompare(Item1, Item2) > 0;
}

bool FileItem::NameSortBackwardsFunc(FileItem const * Item1, FileItem const * Item2)
{
	return NameCompare(Item1, Item2) < 0;
}

int FileItem::NameCompare(FileItem const * Item1, FileItem const * Item2)
{
	if (NULL == Item1)
	{
		return NULL != Item2;
	}
	if (NULL == Item2)
	{
		return -1;
	}
	if (Item1->IsFolder() != Item2->IsFolder())
	{
		if (Item1->IsFolder())
		{
			return -1;
		}
		return 1;
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


bool FileItem::TimeSortFunc(FileItem const * Item1, FileItem const * Item2)
{
	return TimeCompare(Item1, Item2) > 0;
}

bool FileItem::TimeSortBackwardsFunc(FileItem const * Item1, FileItem const * Item2)
{
	return TimeCompare(Item1, Item2) < 0;
}

int FileItem::TimeCompare(FileItem const * Item1, FileItem const * Item2)
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

int FileItem::LengthCompare(FileItem const * Item1, FileItem const * Item2)
{
	if (NULL == Item1)
	{
		return NULL != Item2;
	}
	if (NULL == Item2)
	{
		return -1;
	}
	if (Item1->m_Length > Item2->m_Length)
	{
		return 1;
	}
	if (Item1->m_Length < Item2->m_Length)
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
		m_Length = wfd.nFileSizeLow | ((ULONGLONG) wfd.nFileSizeHigh << 32);
		m_bMd5Calculated = false;
		if ((m_Attributes & FILE_ATTRIBUTE_DIRECTORY) !=
			(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			m_Attributes = wfd.dwFileAttributes;
			return FileDeleted;
		}
		m_Attributes = wfd.dwFileAttributes;

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

CSimpleCriticalSection FileItem::m_Cs;

size_t FileItem::GetFileData(LONGLONG FileOffset, void * pBuf, size_t bytes)
{
	CLastError err;
	err.Set(0);

	CSimpleCriticalSectionLock lock(m_Cs);

	if (NULL == m_hFile)
	{
		// Open the file with FILE_SHARE_DELETE, to allow other applications
		// to delete or move the file
		DWORD ShareMode = FILE_SHARE_READ;
		if ((GetVersion() & 0xFF) >= 5)
		{
			ShareMode = FILE_SHARE_READ | FILE_SHARE_DELETE;
		}

		m_hFile = CreateFile(GetFullName(), GENERIC_READ,
							ShareMode, NULL,
							OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
		if (NULL == m_hFile
			|| INVALID_HANDLE_VALUE == m_hFile)
		{
			m_hFile = NULL;
			err.Get();
			return 0;
		}
		DWORD nFileSizeLow, nFileSizeHigh;
		nFileSizeLow = GetFileSize(m_hFile, & nFileSizeHigh);

		if (nFileSizeLow != INVALID_FILE_SIZE
			|| GetLastError() == NO_ERROR)
		{
			m_Length = nFileSizeLow | (ULONGLONG(nFileSizeHigh) << 32);
		}
	}

	if (FileOffset >= m_Length)
	{
		return 0;
	}
	if (NULL == m_pFileReadBuf)
	{
		m_pFileReadBuf = (BYTE *) VirtualAlloc(NULL, 0x10000, MEM_COMMIT, PAGE_READWRITE);
		if (NULL == m_pFileReadBuf)
		{
			err.Get();
			return 0;
		}
		m_FileReadBufSize = 0x10000;
		m_FileReadFilled = 0;
		m_FileReadPos = 0;
	}

	if (FileOffset + bytes > m_Length)
	{
		bytes = size_t(m_Length - FileOffset);
	}
	// beginning address rounded on the page boundary
	LONGLONG NeedBegin = FileOffset & ~0xFFFi64;
	// end of data wanted
	LONGLONG NeedEnd = FileOffset + bytes;
	// end of data rounded on the page boundary:
	LONGLONG NeedEndBuffer = (NeedEnd + 0xFFF) & ~0xFFFi64;

	DWORD NeedBeginLow = DWORD(NeedBegin);
	LONG NeedBeginHigh = LONG(NeedBegin >> 32);

	if (DWORD(NeedEnd - NeedBegin) > m_FileReadBufSize)
	{
		NeedEnd = NeedBegin + m_FileReadBufSize;
		NeedEndBuffer = NeedEnd;
		bytes = size_t(NeedEnd - FileOffset);
	}

	DWORD BytesRead;
	if (NeedBegin < m_FileReadPos)
	{
		// need data before the buffer begin, but some of the data is in the buffer
		if (NeedEnd > m_FileReadPos && NeedEnd <= m_FileReadPos + m_FileReadFilled)
		{
			TRACE("move data up, read some to the end of buffer\n");
			ULONG_PTR MoveBy = ULONG_PTR(m_FileReadPos - NeedBegin);
			ULONG_PTR NewFilled = m_FileReadFilled + MoveBy;
			ULONG_PTR ToMove = m_FileReadFilled;

			if (NewFilled > m_FileReadBufSize)
			{
				ToMove -= NewFilled - m_FileReadBufSize;
				NewFilled = m_FileReadBufSize;
			}
			if (0 != MoveBy)
			{
				memmove(m_pFileReadBuf + MoveBy,
						m_pFileReadBuf, ToMove);

				SetFilePointer(m_hFile, NeedBeginLow, & NeedBeginHigh, FILE_BEGIN);
				TRACE("Reading %d bytes at %X\n", MoveBy, NeedBeginLow);
				if ( ! ReadFile(m_hFile, m_pFileReadBuf, MoveBy, & BytesRead, NULL))
				{
					err.Get();
					m_FileReadFilled = 0;
					return 0;
				}
				m_FileReadFilled = NewFilled;
			}
			m_FileReadPos = NeedBegin;
		}
		else
		{
			SetFilePointer(m_hFile, NeedBeginLow, & NeedBeginHigh, FILE_BEGIN);
			TRACE("Reading %d bytes at %X\n", m_FileReadBufSize, NeedBeginLow);
			if ( ! ReadFile(m_hFile, m_pFileReadBuf, m_FileReadBufSize, & BytesRead, NULL))
			{
				err.Get();
				m_FileReadFilled = 0;
				return 0;
			}
			m_FileReadPos = NeedBegin;
			m_FileReadFilled = BytesRead;

		}
	}
	else if (NeedBegin < m_FileReadPos + m_FileReadFilled)
	{
		// may need data after the buffer end, but some of the data is in the buffer
		if (NeedEnd > m_FileReadPos + m_FileReadFilled)
		{
			TRACE("move data down, read some more data\n");
			ULONG_PTR MoveBy = ULONG_PTR(NeedEndBuffer - (m_FileReadPos + m_FileReadFilled));
			if (0 != MoveBy)
			{
				if (m_FileReadFilled > MoveBy)
				{
					ULONG_PTR NewFilled = m_FileReadFilled - MoveBy;
					ULONG_PTR ToMove = NewFilled;
					memmove(m_pFileReadBuf, m_pFileReadBuf + MoveBy, ToMove);

					m_FileReadFilled = NewFilled;
					m_FileReadPos += MoveBy;
					NeedBegin = m_FileReadPos + NewFilled;

					NeedBeginLow = DWORD(NeedBegin);
					NeedBeginHigh = DWORD(NeedBegin >> 32);
				}
				else
				{
					m_FileReadFilled = 0;
					m_FileReadPos = NeedBegin;
				}
			}

			SetFilePointer(m_hFile, NeedBeginLow, & NeedBeginHigh, FILE_BEGIN);

			TRACE("Reading %d bytes at %X\n", m_FileReadBufSize - m_FileReadFilled, NeedBeginLow);
			if ( ! ReadFile(m_hFile, m_pFileReadBuf, m_FileReadBufSize - m_FileReadFilled, & BytesRead, NULL))
			{
				err.Get();
				m_FileReadFilled = 0;
				return 0;
			}
			m_FileReadFilled += BytesRead;
		}
	}
	else
	{
		SetFilePointer(m_hFile, NeedBeginLow, & NeedBeginHigh, FILE_BEGIN);
		TRACE("Reading %d bytes at %X\n", m_FileReadBufSize, NeedBeginLow);
		if ( ! ReadFile(m_hFile, m_pFileReadBuf, m_FileReadBufSize, & BytesRead, NULL))
		{
			err.Get();
			m_FileReadFilled = 0;
			return 0;
		}
		m_FileReadPos = NeedBegin;
		m_FileReadFilled = BytesRead;
	}
	// now get data from the buffer and return
	ASSERT(FileOffset >= m_FileReadPos);
	size_t Index = size_t(FileOffset - m_FileReadPos);

	ASSERT(m_FileReadFilled <= m_FileReadBufSize);
	ASSERT(Index + bytes <= m_FileReadBufSize);
	ASSERT(Index + bytes <= m_FileReadFilled);

	memcpy(pBuf, m_pFileReadBuf + Index, bytes);
	return bytes;
}

PairCheckResult FilePair::CheckForFilesChanged()
{
	if (MemoryFile == m_ComparisonResult)
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
	if (MemoryFile == m_ComparisonResult)
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
			m_ComparisonResult = OnlySecondFile;
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
			m_ComparisonResult = OnlyFirstFile;
		}
	}
	if (FileDeleted == res1
		&& FileDeleted == res2)
	{
		m_ComparisonResult = FileUnaccessible;
		return FilesDeleted;
	}
	return FilesTimeChanged;
}

FileList::FileList()
	: m_pList(NULL), m_NumFiles(0)
{
}

FileList::~FileList()
{
	FreeFileList();
}

void FileList::GetSortedList(vector<FileItem *> & ItemArray, DWORD SortFlags)
{
	ItemArray.resize(m_NumFiles);
	if (0 == m_NumFiles)
	{
		return;
	}
	// sort the file lists by subdirs and names

	FileItem * pCurItem = m_pList;
	for (int i = 0; i < m_NumFiles; i++)
	{
		ItemArray[i] = pCurItem;
		if (NULL != pCurItem)
		{
			pCurItem = pCurItem->m_pNext;
		}
	}
	bool (* SortFunc)(FileItem const * Item1, FileItem const * Item2);
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
	std::sort(ItemArray.begin(), ItemArray.end(), SortFunc);
#ifdef _DEBUG
	if (1) for (i = 0; i < m_NumFiles && NULL != ItemArray[i]; i++)
		{
			TRACE(_T("Sorted file item: subdir=%s, Name=\"%s\"\n"),
				ItemArray[i]->GetSubdir(), ItemArray[i]->GetName());
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
						LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask, LPCTSTR sIgnoreDirs)
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

	if ( ! bRecurseSubdirs)
	{
		sIgnoreDirs = _T("*");
	}

	return LoadSubFolder(_T(""), sInclusionMask, sExclusionMask,
						sC_CPPMask, sBinaryMask, sIgnoreDirs, NULL);
}

bool ReadSubfolder(FileItem ** ppFiles, FileItem ** ppDirs,
					CString const & SubDirectory, CString const & BaseDirectory,
					LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
					LPCTSTR sIgnoreDirs, FileItem * pParentDir)
{
	if (0) TRACE(_T("LoadSubFolder: scanning %s\n"), LPCTSTR(BaseDirectory + SubDirectory));

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(BaseDirectory + SubDirectory + _T("*"), & wfd);

	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		return false;
	}

	do
	{
		FileItem * pFile = NULL;

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (0) TRACE(_T("Found the subdirectory %s\n"), wfd.cFileName);
			if (0 == _tcscmp(wfd.cFileName, _T("."))
				|| 0 == _tcscmp(wfd.cFileName, _T(".."))
				|| MultiPatternMatches(wfd.cFileName, sIgnoreDirs)
				)
			{
				continue;
			}
			// add the directory item
			// scan the subdirectory

			wfd.cFileName[countof(wfd.cFileName) - 1] = 0;

			pFile = new FileItem( & wfd, BaseDirectory, SubDirectory, pParentDir);
			if (NULL == pFile)
			{
				continue;
			}

			// add to the list
			pFile->m_pNext = *ppDirs;
			*ppDirs = pFile;
		}
		else
		{
			// filter the file and add it to the list, if it matches the pattern.
			if (0) TRACE(_T("File %s found\n"), wfd.cFileName);

			if (! MultiPatternMatches(wfd.cFileName, sInclusionMask)
				|| MultiPatternMatches(wfd.cFileName, sExclusionMask))
			{
				TRACE("File name does not match\n");
				continue;
			}

			if (0) TRACE(_T("New file item: Name=\"%s\", base dir=%s, subdir=%s\n"),
						wfd.cFileName, BaseDirectory, SubDirectory);

			pFile = new FileItem( & wfd, BaseDirectory, SubDirectory, pParentDir);
			if (NULL == pFile)
			{
				continue;
			}
			// add to the list
			pFile->m_pNext = *ppFiles;
			*ppFiles = pFile;
		}
	}
	while (FindNextFile(hFind, & wfd));
	FindClose(hFind);
	return true;
}

bool FileList::LoadSubFolder(LPCTSTR Subdir,
							LPCTSTR sInclusionMask, LPCTSTR sExclusionMask,
							LPCTSTR sC_CPPMask, LPCTSTR sBinaryMask,
							LPCTSTR sIgnoreDirs, FileItem * pParentDir)
{
	if (0) TRACE(_T("LoadSubFolder: scanning %s\n"), LPCTSTR(Subdir));
	FileItem * pFilesList = NULL;
	FileItem * pDirsList = NULL;

	CString SubDirectory(Subdir);
	// make sure the name contains '\'.
	if ( ! SubDirectory.IsEmpty())
	{
		SubDirectory += _T("\\");
	}

	if (! ReadSubfolder(& pFilesList, & pDirsList,
						SubDirectory, m_BaseDir,
						sInclusionMask, sExclusionMask, sIgnoreDirs, pParentDir))
	{
		return false;
	}


	// files first
	while (pFilesList)
	{
		FileItem * pFile = pFilesList;
		pFilesList = pFile->m_pNext;

		if (MultiPatternMatches(pFile->GetName(), sBinaryMask))
		{
			pFile->m_IsBinary = true;
		}
		else
		{
			pFile->m_C_Cpp = MultiPatternMatches(pFile->GetName(), sC_CPPMask);
		}
		// add to the list
		pFile->m_pNext = m_pList;
		m_pList = pFile;
		m_NumFiles++;
	}

	while (pDirsList)
	{
		FileItem * pDir = pDirsList;
		pDirsList = pDir->m_pNext;

		// add to the list
		pDir->m_pNext = m_pList;
		m_pList = pDir;
		m_NumFiles++;

		// scan the subdirectory
		// but don't recurse down reparse point

		if ( ! pDir->IsReparsePoint())
		{
			LoadSubFolder(SubDirectory + pDir->GetName(),
						sInclusionMask, sExclusionMask, sC_CPPMask, sBinaryMask, sIgnoreDirs, pDir);
		}
	}

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

FilePair::FilePair()
	: pFirstFile(NULL),
	pSecondFile(NULL),
	m_RefCount(1),
	m_LoadedCount(0),
	m_bChanged(false),
	m_bHideFromListView(false),
	m_bSelected(false),
	m_bFocused(false),
	m_bDeleted(false),

	m_FilenameSortOrder(0),
	m_DirectorySortOrder(0),

	m_ComparisonResult(ResultUnknown)
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

int FilePairComparePredicate::Time1SortFunc(const FilePair * Pair1, const FilePair * Pair2)
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

int FilePairComparePredicate::Time1SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2)
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

int FilePairComparePredicate::Time2SortFunc(const FilePair * Pair1, const FilePair * Pair2)
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

int FilePairComparePredicate::Time2SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2)
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

int FilePairComparePredicate::Length1SortFunc(const FilePair * Pair1, const FilePair * Pair2)
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
	return FileItem::LengthCompare(Item2, Item1);
}

int FilePairComparePredicate::Length1SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2)
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
	return FileItem::LengthCompare(Item1, Item2);
}

int FilePairComparePredicate::Length2SortFunc(const FilePair * Pair1, const FilePair * Pair2)
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
	return FileItem::LengthCompare(Item2, Item1);
}

int FilePairComparePredicate::Length2SortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2)
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
	return FileItem::LengthCompare(Item1, Item2);
}

int FilePairComparePredicate::NameCompare(const FilePair * Pair1, const FilePair * Pair2)
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
	return -FileItem::NameCompare(Item1, Item2);
}

int FilePairComparePredicate::NameCompareBackward(const FilePair * Pair1, const FilePair * Pair2)
{
	return NameCompare(Pair2, Pair1);
}

int FilePairComparePredicate::DirNameCompare(const FilePair * Pair1, const FilePair * Pair2)
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
	return -FileItem::DirNameCompare(Item1, Item2);
}

int FilePairComparePredicate::DirNameCompareBackward(const FilePair * Pair1, const FilePair * Pair2)
{
	return DirNameCompare(Pair2, Pair1);
}

int FilePair::ComparisionResultPriority() const
{
	// the following order:
	// ResultUnknown, FilesDifferent, VersionInfoDifferent, DifferentInSpaces, FilesIdentical,
	// OnlyFirstFile, OnlySecondFile
	switch (m_ComparisonResult)
	{
	case ErrorReadingFirstFile:
		return 0;
	case ErrorReadingSecondFile:
		return 1;
	case FileUnaccessible:
		return 2;
	case FilesDifferent:
		return 3;
	case FirstFileLonger:
		return 4;
	case SecondFileLonger:
		return 5;
	case DifferentInSpaces:
		return 6;
	case VersionInfoDifferent:
		return 7;
	case FilesIdentical:
		return 8;
	case DirectoryInFingerprintFileOnly:
		return 9;
	case FileInFingerprintFileOnly:
		return 10;
	case OnlyFirstFile:
		return 11;
	case OnlySecondFile:
		return 12;
	case OnlyFirstDirectory:
		return 13;
	case OnlySecondDirectory:
		return 14;
	case FileFromSubdirInFirstDirOnly:
		return 15;
	case FileFromSubdirInSecondDirOnly:
		return 16;
	case ReadingFirstFile:
		return 17;
	case ReadingSecondFile:
		return 18;
	default:
	case ResultUnknown:
		return 100;
	}
}

bool FilePairComparePredicate::operator ()(const FilePair * Pair1, const FilePair * Pair2)
{
	for (int i = 0; i < countof (Functions); i++)
	{
		int result = Functions[i](Pair1, Pair2);
		if (result != 0)
		{
			return result < 0;
		}
	}
	return 0;
}

FilePairComparePredicate::FilePairComparePredicate(enum eColumns Sort[], bool Ascending[], int SortNumber)
{
	for (int i = 0; i < countof (Functions); i++)
	{
		if (i >= SortNumber)
		{
			Functions[i] = NoOp;
			continue;
		}

		if (Ascending[i])
		{
			switch (Sort[i])
			{
			case ColumnName:
				Functions[i] = NameCompare;
				break;
			case ColumnSubdir:
				Functions[i] = DirNameCompare;
				break;
			case ColumnDate1:
				Functions[i] = Time1SortFunc;
				break;
			case ColumnDate2:
				Functions[i] = Time2SortFunc;
				break;
			case ColumnLength1:
				Functions[i] = Length1SortFunc;
				break;
			case ColumnLength2:
				Functions[i] = Length2SortFunc;
				break;
			case ColumnComparisionResult:
				Functions[i] = ComparisionSortFunc;
				break;
			default:
				Functions[i] = NoOp;
				break;
			}
		}
		else
		{
			switch (Sort[i])
			{
			case ColumnName:
				Functions[i] = NameCompareBackward;
				break;
			case ColumnSubdir:
				Functions[i] = DirNameCompareBackward;
				break;
			case ColumnDate1:
				Functions[i] = Time1SortBackwardsFunc;
				break;
			case ColumnDate2:
				Functions[i] = Time2SortBackwardsFunc;
				break;
			case ColumnLength1:
				Functions[i] = Length1SortBackwardsFunc;
				break;
			case ColumnLength2:
				Functions[i] = Length2SortBackwardsFunc;
				break;
			case ColumnComparisionResult:
				Functions[i] = ComparisionSortBackwardsFunc;
				break;
			default:
				Functions[i] = NoOp;
				break;
			}
		}
	}
}

int FilePairComparePredicate::ComparisionSortFunc(const FilePair * Pair1, const FilePair * Pair2)
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

int FilePairComparePredicate::ComparisionSortBackwardsFunc(const FilePair * Pair1, const FilePair * Pair2)
{
	int priority1 = Pair1->ComparisionResultPriority();
	int priority2 = Pair2->ComparisionResultPriority();

	if (priority1 > priority2)
	{
		return -1;
	}
	if (priority1 < priority2)
	{
		return 1;
	}

	return 0;
}

CString FilePair::GetComparisonResultStr() const
{
	static CString sFilesUnaccessible(MAKEINTRESOURCE(IDS_STRING_FILES_UNACCESSIBLE));
	static CString sFilesIdentical(MAKEINTRESOURCE(IDS_STRING_FILES_IDENTICAL));
	static CString sVersionInfoDifferent(MAKEINTRESOURCE(IDS_VERSION_INFO_DIFFERENT));
	static CString sDifferentInSpaces(MAKEINTRESOURCE(IDS_DIFFERENT_IN_SPACES));
	static CString sFilesDifferent(MAKEINTRESOURCE(IDS_FILES_DIFFERENT));
	static CString sOnlyOneExists(MAKEINTRESOURCE(IDS_STRING_ONLY_ONE_EXISTS));
	static CString sOnlyFingerprintExists(MAKEINTRESOURCE(IDS_STRING_ONLY_FINGERPRINT_EXISTS));
	static CString sOnlyOneSubdirExists(MAKEINTRESOURCE(IDS_STRING_ONE_SUBDIR_EXISTS));
	static CString sOnlyFingerprintSubdirExists(MAKEINTRESOURCE(IDS_STRING_ONLY_FINGERPRINT_SUBDIR_EXISTS));
	static CString sOnlyFingerprintFilesSubdirExists(MAKEINTRESOURCE(IDS_STRING_ONLY_FINGERPRINT_FILES_SUBDIR_EXISTS));
	static CString sOneFileLonger(MAKEINTRESOURCE(IDS_STRING_FILE_IS_LONGER));
	static CString sReadingFile(MAKEINTRESOURCE(IDS_STRING_READING_FILE));
	static CString sErrorReadingFile(MAKEINTRESOURCE(IDS_STRING_ERROR_READING_FILE));
	static CString sCalculatingFingerprint(MAKEINTRESOURCE(IDS_STRING_CALC_FINGERPRINT));
	static CString sComparingFiles(MAKEINTRESOURCE(IDS_STRING_COMPARING));
	static CString sOnlyOneFilesSubdirExists(MAKEINTRESOURCE(IDS_STRING_FILES_ONE_SUBDIR_EXISTS));
	static CString sOnlyOneSubdirsParentExists(MAKEINTRESOURCE(IDS_STRING_FILES_ONE_SUBDIR_PARENT_EXISTS));

	CString s;
	TCHAR buf1[MAX_PATH], buf2[MAX_PATH];

	switch(m_ComparisonResult)
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
	case FileInFingerprintFileOnly:
		return sOnlyFingerprintExists;
		break;
	case OnlyFirstFile:
		s.Format(sOnlyOneExists,
				pFirstFile->GetBasedir(), pFirstFile->GetSubdir());
		break;
	case FileFromSubdirInFirstDirOnly:
		s.Format(sOnlyOneFilesSubdirExists,
				pFirstFile->GetBasedir(), pFirstFile->GetSubdir());
		break;
	case SubdirsParentInFirstDirOnly:
		s.Format(sOnlyOneSubdirsParentExists,
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
	case DirectoryInFingerprintFileOnly:
		return sOnlyFingerprintSubdirExists;
		break;
	case FilesDirectoryInFingerprintFileOnly:
		return sOnlyFingerprintFilesSubdirExists;
		break;
	case OnlySecondDirectory:
		s.Format(sOnlyOneSubdirExists,
				pSecondFile->GetBasedir(), pSecondFile->GetSubdir());
		break;
	case FileFromSubdirInSecondDirOnly:
		s.Format(sOnlyOneFilesSubdirExists,
				pSecondFile->GetBasedir(), pSecondFile->GetSubdir());
		break;
	case SubdirsParentInSecondDirOnly:
		s.Format(sOnlyOneSubdirsParentExists,
				pSecondFile->GetBasedir(), pSecondFile->GetSubdir());
		break;

	case FirstFileLonger:
		s.Format(sOneFileLonger,
				pFirstFile->GetBasedir(), pFirstFile->GetSubdir(),
				pFirstFile->GetFileLength() - pSecondFile->GetFileLength());
		break;
	case SecondFileLonger:
		s.Format(sOneFileLonger,
				pSecondFile->GetBasedir(), pSecondFile->GetSubdir(),
				pSecondFile->GetFileLength() - pFirstFile->GetFileLength());
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
		break;
	case CalculatingSecondFingerprint:
		s.Format(sCalculatingFingerprint, LPCTSTR(pSecondFile->GetFullName()));
		break;
	case ComparingFiles:
		_tcsncpy(buf1, pFirstFile->GetFullName(), countof(buf1));
		buf1[countof(buf1) - 1] = 0;
		AbbreviateName(buf1, 50, TRUE);

		_tcsncpy(buf2, pSecondFile->GetFullName(), countof(buf2));
		buf2[countof(buf2) - 1] = 0;
		AbbreviateName(buf2, 50, TRUE);

		s.Format(sComparingFiles, buf1, buf2);
		break;
	}
	return s;
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
		result = pFirstFile->m_bIsPhantomFile || (pFirstFile->Load() && result);
	}
	if (NULL != pSecondFile)
	{
		result = pSecondFile->Load() && result;
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
	if (0) TRACE("Unloading file pair\n");
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
		while( ! pPair->StrSections.IsEmpty())
		{
			delete pPair->StrSections.RemoveHead();
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

FilePair::eFileComparisionResult FilePair::PreCompareFiles(CMd5HashCalculator * pMd5Calc,
															class CProgressDialog * pProgressDialog)
{
	// different comparision for different modes
	if (NeedBinaryComparison())
	{
		return PreCompareBinaryFiles(pMd5Calc, pProgressDialog);
	}

	if (NULL != pProgressDialog)
	{
		m_ComparisonResult = ComparingFiles;

		pProgressDialog->SetNextItem(GetComparisonResultStr(),
									2 * (pFirstFile->GetFileLength() + pSecondFile->GetFileLength()),
									FILE_OPEN_OVERHEAD * 2);
	}

	eFileComparisionResult result = FileUnaccessible;

	if (LoadFiles())
	{
		if (NULL != pProgressDialog)
		{
			pProgressDialog->SetCurrentItemDone(pFirstFile->GetFileLength() + pSecondFile->GetFileLength());
		}
#ifdef _DEBUG
		DWORD BeginTime = timeGetTime();
#endif
		result = ResultUnknown;
		if (NULL != pFirstFile
			&& NULL != pSecondFile)
		{
			result = PreCompareTextFiles(pProgressDialog);
		}
#ifdef _DEBUG
		TRACE("Files compared in %d ms\n", timeGetTime() - BeginTime);
		BeginTime = timeGetTime();
#endif
		UnloadFiles();
	}
	if (NULL != pProgressDialog)
	{
		pProgressDialog->AddDoneItem(2 * (pFirstFile->GetFileLength()
										+ pSecondFile->GetFileLength()));
	}

	return result;
}

FilePair::eFileComparisionResult FilePair::CompareFiles(class CProgressDialog * pProgressDialog)
{
	// TODO: different function for binary comparision
	if (! LoadFiles())
	{
		return ResultUnknown;
	}
	eFileComparisionResult result = ResultUnknown;
	if (NULL != pFirstFile && ! pFirstFile->m_bIsPhantomFile
		&& NULL != pSecondFile && ! pFirstFile->m_bIsPhantomFile)
	{
		result = CompareTextFiles(pProgressDialog);
	}
	else
	{
		// just build the line array
		FileItem * pFile = pFirstFile;
		result = OnlyFirstFile;
		if (NULL == pFile)
		{
			pFile = pSecondFile;
			result = OnlySecondFile;
		}
		else if (pFile->m_bIsPhantomFile)
		{
			pFile = pSecondFile;
			// keep previous result
			result = m_ComparisonResult;
		}

		if (NULL != pFile)
		{
			m_LinePairs.resize(pFile->GetNumLines());
			for (unsigned i = 0; i < m_LinePairs.size(); i++)
			{
				m_LinePairs[i] = new LinePair(pFile->GetLine(i));
			}
		}
	}

	if (MemoryFile == m_ComparisonResult)
	{
		return MemoryFile;
	}
	// different comparision for different modes
	return result;
}

FilePair::eFileComparisionResult FilePair::PreCompareTextFiles(class CProgressDialog * /*pProgressDialog*/)
{
	int nLine1 = 0;
	int nLine2 = 0;

	int NumLines1 = pFirstFile->GetNumLines();
//    int NumNonBlankLines1 = pFirstFile->m_NormalizedHashSortedLines.size();

	int NumLines2 = pSecondFile->GetNumLines();
//    int NumNonBlankLines2 = pSecondFile->m_NormalizedHashSortedLines.size();

	bool SpacesDifferent = false;
	bool OnlyVersionInfoDifferent = false;

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
					// check if the lines differs only in the version control information
					LPCTSTR str1 = Line1->GetNormalizedText();
					LPCTSTR pVersionStart1 = _tcschr(str1, '$');
					LPCTSTR pVersionEnd1 = _tcsrchr(str1, '$');

					LPCTSTR str2 = Line2->GetNormalizedText();
					LPCTSTR pVersionStart2 = _tcschr(str2, '$');
					LPCTSTR pVersionEnd2 = _tcsrchr(str2, '$');

					if (NULL == pVersionStart1
						|| NULL == pVersionEnd1
						|| NULL == pVersionStart2
						|| NULL == pVersionEnd2
						|| pVersionStart1 == pVersionEnd1
						|| pVersionStart2 == pVersionEnd2
						|| pVersionStart1 - str1 != pVersionStart2 - str2
						|| 0 != memcmp(str1, str2, (pVersionStart1 - str1) * sizeof (TCHAR))
						|| 0 != _tcscmp(pVersionEnd1 + 1, pVersionEnd2 + 1))
					{
						return FilesDifferent;
					}
					OnlyVersionInfoDifferent = true;
				}
			}
			nLine1++;
			nLine2++;
		}
	}

	if (OnlyVersionInfoDifferent)
	{
		return VersionInfoDifferent;
	}
	else if (SpacesDifferent)
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
				if (0 && LooksLike(Line1, Line2, 15))
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
FilePair::eFileComparisionResult FilePair::CompareBinaryFiles(class CProgressDialog * /*pProgressDialog*/)
{
//    CThisApp * pApp = GetApp();
	return ResultUnknown;
}

FilePair::eFileComparisionResult FilePair::PreCompareBinaryFiles(CMd5HashCalculator * pMd5Calc,
																class CProgressDialog * pProgressDialog)
{
	CThisApp * pApp = GetApp();
	// comparison can be done through CRC, or direct comparison
	// if length is different, return it:
	if (pFirstFile->GetFileLength() != pSecondFile->GetFileLength())
	{
		if (pFirstFile->GetFileLength() > pSecondFile->GetFileLength())
		{
			return m_ComparisonResult = FirstFileLonger;
		}
		else
		{
			return m_ComparisonResult = SecondFileLonger;
		}
	}
	if (pApp->m_bUseMd5)
	{
		if ( ! pFirstFile->m_bMd5Calculated)
		{
			m_ComparisonResult = CalculatingFirstFingerprint;

			if (NULL != pProgressDialog)
			{
				pProgressDialog->SetNextItem(GetComparisonResultStr(),
											pFirstFile->GetFileLength(), FILE_OPEN_OVERHEAD);
			}

			if (! pFirstFile->CalculateHashes(pMd5Calc, pProgressDialog)
				&& (NULL == pProgressDialog || ! pProgressDialog->m_StopRunThread))
			{
				if (NULL != pProgressDialog)
				{
					pProgressDialog->AddDoneItem(pFirstFile->GetFileLength());
				}
				return m_ComparisonResult = FilePair::ErrorReadingFirstFile;
			}

			if (NULL != pProgressDialog)
			{
				pProgressDialog->AddDoneItem(pFirstFile->GetFileLength());
			}
		}

		if ( ! pSecondFile->m_bMd5Calculated)
		{
			m_ComparisonResult = CalculatingSecondFingerprint;

			if (NULL != pProgressDialog)
			{
				pProgressDialog->SetNextItem(GetComparisonResultStr(),
											pSecondFile->GetFileLength(), FILE_OPEN_OVERHEAD);
			}

			if (! pSecondFile->CalculateHashes(pMd5Calc, pProgressDialog)
				&& (NULL == pProgressDialog || ! pProgressDialog->m_StopRunThread))
			{
				if (NULL != pProgressDialog)
				{
					pProgressDialog->AddDoneItem(pSecondFile->GetFileLength());
				}
				return m_ComparisonResult = FilePair::ErrorReadingSecondFile;
			}

			if (NULL != pProgressDialog)
			{
				pProgressDialog->AddDoneItem(pSecondFile->GetFileLength());
			}
		}
	}
	// if files have MD5 calculated, use it
	if (pFirstFile->m_bMd5Calculated
		&& pSecondFile->m_bMd5Calculated)
	{
		if (memcmp(pFirstFile->m_Md5, pSecondFile->m_Md5,
					sizeof pFirstFile->m_Md5))
		{
			return m_ComparisonResult = FilesDifferent;
		}
		return m_ComparisonResult = FilesIdentical;
	}

	return m_ComparisonResult = ResultUnknown;
}

FilePair::eFileComparisionResult FilePair::CompareTextFiles(CProgressDialog * /*pProgressDialog*/)
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
				&& LooksLike(pFirstFile->GetLine(pSection->File1LineBegin - 1),
							pSecondFile->GetLine(pSection->File2LineBegin - 1),
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
				&& LooksLike(pFirstFile->GetLine(pSection->File1LineEnd),
							pSecondFile->GetLine(pSection->File2LineEnd),
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
				pPair = new LinePair(pFirstFile->GetLine(i), NULL, pDiffSection);
				if (NULL == pPair)
				{
					break;
				}
				if (m_LinePairs.size() <= nLineIndex)
				{
					m_LinePairs.resize(nLineIndex+1);
				}
				m_LinePairs[nLineIndex] = pPair;
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
				pPair = new LinePair(NULL, pSecondFile->GetLine(i), pDiffSection);
				if (NULL == pPair)
				{
					break;
				}
				if (m_LinePairs.size() <= nLineIndex)
				{
					m_LinePairs.resize(nLineIndex+1);
				}
				m_LinePairs[nLineIndex] = pPair;
			}

		}

		int line1 = pSection->File1LineBegin;
		int line2 = pSection->File2LineBegin;
		for ( ; line1 < pSection->File1LineEnd || line2 < pSection->File2LineEnd; nLineIndex++)
		{
			const FileLine * pLine1 = NULL;
			if (line1 < pSection->File1LineEnd)
			{
				pLine1 = pFirstFile->GetLine(line1);
			}

			// if only one of the strings is blank, make a single-line entry from the blank line only
			const FileLine * pLine2 = NULL;
			if (line2 < pSection->File2LineEnd)
			{
				// if only one of the strings is blank, make a single-line entry
				pLine2 = pSecondFile->GetLine(line2);

				if (pLine2->IsBlank())
				{
					if (NULL != pLine1
						&& ! pLine1->IsBlank())
					{
						pLine1 = NULL;
					}
				}
				else if (NULL != pLine1
						&& pLine1->IsBlank())
				{
					pLine2 = NULL;
				}
			}

			if (NULL != pLine1
				&& pLine1->IsBlank())
			{
				if (NULL == pLine2
					|| ! pLine2->IsBlank())
				{
					pLine2 = NULL;
				}
			}

			if (NULL != pLine1)
			{
				line1++;
			}

			if (NULL != pLine2)
			{
				line2++;
			}

			pPair = new LinePair(pLine1, pLine2);
			if (NULL == pPair)
			{
				break;
			}

			if (m_LinePairs.size() <= nLineIndex)
			{
				m_LinePairs.resize(nLineIndex+1);
			}
			m_LinePairs[nLineIndex] = pPair;
			pPair->BuildDiffSectionsList(nLineIndex, m_DiffSections, pApp->m_MinMatchingChars);
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
	// check difference sections, see what is difference
	eFileComparisionResult Result = FilesIdentical;
	for (unsigned i = 0; i < m_DiffSections.size(); i++)
	{
		if (0 != (m_DiffSections[i]->m_Flags & FileDiffSection::FlagVersionInfoDifferent))
		{
			Result = VersionInfoDifferent;
		}
		else if (0 != (m_DiffSections[i]->m_Flags & FileDiffSection::FlagWhitespace))
		{
			if (FilesIdentical == Result)
			{
				Result = DifferentInSpaces;
			}
		}
		else
		{
			Result = FilesDifferent;
			break;
		}
	}
	return Result;
}

bool FilePair::NextDifference(TextPosDisplay PosFrom, BOOL IgnoreWhitespaces,
							TextPosDisplay * DiffPos, TextPosDisplay * EndPos)
{
	if (m_DiffSections.empty())
	{
		return FALSE;
	}
	FileDiffSection diff;
	diff.m_Begin = DisplayPosToLinePos(PosFrom, IgnoreWhitespaces);

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

	int NewFileScope = 0;
	if (0 != PosFrom.scope)
	{
		int flags = GetAcceptDeclineFlags(pSection->m_Begin, pSection->m_End, IgnoreWhitespaces != FALSE);
		if (flags & StringSection::Erased)
		{
			NewFileScope = 1;
		}
		else
		{
			NewFileScope = 2;
		}
	}

	if (NULL != DiffPos)
	{
		*DiffPos = LinePosToDisplayPos(pSection->m_Begin, IgnoreWhitespaces, NewFileScope);
	}
	if (NULL != EndPos)
	{
		*EndPos = LinePosToDisplayPos(pSection->m_End, IgnoreWhitespaces, NewFileScope);
	}
	return TRUE;
}

bool FilePair::PrevDifference(TextPosDisplay PosFrom, BOOL IgnoreWhitespaces,
							TextPosDisplay * DiffPos, TextPosDisplay * EndPos)
{
	if (m_DiffSections.empty())
	{
		return FALSE;
	}
	FileDiffSection diff;
	diff.m_Begin = DisplayPosToLinePos(PosFrom, IgnoreWhitespaces);

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

	int NewFileScope = 0;
	if (0 != PosFrom.scope)
	{
		int flags = GetAcceptDeclineFlags(pSection->m_Begin, pSection->m_End, IgnoreWhitespaces != FALSE);
		if (flags & StringSection::Erased)
		{
			NewFileScope = 1;
		}
		else
		{
			NewFileScope = 2;
		}
	}

	if (NULL != DiffPos)
	{
		*DiffPos = LinePosToDisplayPos(pSection->m_Begin, IgnoreWhitespaces, NewFileScope);
	}
	if (NULL != EndPos)
	{
		*EndPos = LinePosToDisplayPos(pSection->m_End, IgnoreWhitespaces, NewFileScope);
	}
	return TRUE;
}

TextPosLine FilePair::DisplayPosToLinePos(TextPosDisplay position, BOOL IgnoreWhitespaces)
{
	if (unsigned(position.line) >= m_LinePairs.size())
	{
		return TextPosLine(position.line, position.pos);
	}
	return TextPosLine(position.line,
						m_LinePairs[position.line]->DisplayPosToLinePos(position.pos, IgnoreWhitespaces, position.scope));
}

TextPosDisplay FilePair::LinePosToDisplayPos(TextPosLine position, BOOL IgnoreWhitespaces, int FileScope)
{
	if (unsigned(position.line) >= m_LinePairs.size())
	{
		return TextPosDisplay(position.line, position.pos, FileScope);
	}
	return TextPosDisplay(position.line,
						(m_LinePairs[position.line]->LinePosToDisplayPos(position.pos, IgnoreWhitespaces, FileScope)),
						FileScope);
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

BOOL FilePair::ModifyAcceptDeclineFlags(TextPosLine & PosFrom, TextPosLine & PosTo, int Set, int Reset)
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

int FilePair::GetAcceptDeclineFlags(TextPosLine PosFrom, TextPosLine PosTo, bool bIgnoreWhitespaces)
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

BOOL FilePair::EnumStringDiffSections(TextPosLine & PosFrom, TextPosLine & PosTo,
									void (* Func)(StringSection * pSection, void * Param), void * pParam)
{
	TextPosLine begin = PosFrom, end = PosTo;
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
		StringSection * pSection = pPair->StrSections.First();
		if (pPair->StrSections.NotEnd(pSection)
			&& pPair->StrSections.Last() == pSection)
		{
			// it's the only section
			if (0 == (pSection->Attr & (pSection->Inserted | pSection->Erased)))
			{
				//pSection = NULL;
				return FALSE;
			}
		}
		else
		{
			for ( ; pPair->StrSections.NotEnd(pSection); pos += pSection->Length, pSection = pSection->Next())
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
					|| (pPair->StrSections.Last() != pSection
						&& 0 != (pSection->Next()->Attr & (pSection->Inserted | pSection->Erased))
						&& pos + pSection->Length + pSection->Next()->Length > begin.pos))
				{
					// section found
					break;
				}
			}
		}
		if (pPair->StrSections.IsEnd(pSection))
		{
			return FALSE;
		}
		// if the found section is the only section of the line, check if the previous lines are same way
		if (pSection == pPair->StrSections.First()
			&& pSection == pPair->StrSections.Last())
		{
			begin.pos = 0;
			end.pos = 0;
			end.line = begin.line + 1;

			Func(pSection, pParam);

			while (begin.line > 0)
			{
				pSection = m_LinePairs[begin.line - 1]->StrSections.First();

				if (m_LinePairs[begin.line - 1]->StrSections.IsEnd(pSection)
					|| m_LinePairs[begin.line - 1]->StrSections.Last() != pSection
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
				pSection = m_LinePairs[end.line]->StrSections.First();
				if (m_LinePairs[end.line]->StrSections.IsEnd(pSection)
					|| m_LinePairs[end.line]->StrSections.Last() != pSection
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

			pSection = pSection->Next();

			if (pPair->StrSections.NotEnd(pSection)
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
		TextPosLine tmp = end;
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
	for (pSection = pPair->StrSections.First();
		pPair->StrSections.NotEnd(pSection); pos += pSection->Length, pSection = pPair->StrSections.Next(pSection))
	{
		if (end.line == begin.line
			&& pos >= end.pos)
		{
			// no section in this line
			pSection = pPair->StrSections.Last()->Next();
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
	TextPosLine ChangeEnd = begin;
	for ( ; pPair->StrSections.NotEnd(pSection); pos += pSection->Length, pSection = pPair->StrSections.Next(pSection))
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
		for (pos = 0, pSection = m_LinePairs[line]->StrSections.First();
			m_LinePairs[line]->StrSections.NotEnd(pSection); pSection = pSection->Next())
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

BOOL FileItem::CalculateHashes(CMd5HashCalculator * pMd5Calc,
								class CProgressDialog * pProgressDialog)
{
	BOOL res = pMd5Calc->CalculateFileMd5Hash(GetFullName(), m_Md5, pProgressDialog);
	if (res)
	{
		m_bMd5Calculated = true;
	}
	return res;
}

void FileItem::SetMD5(BYTE md5[16])
{
	memcpy(m_Md5, md5, sizeof m_Md5);
	m_bMd5Calculated = true;
	m_bIsPhantomFile = true;
	m_IsBinary = true;
}

CSmallAllocator StringSection::m_Allocator(sizeof StringSection);
CSmallAllocator FileDiffSection::m_Allocator(sizeof FileDiffSection);

//CSimpleCriticalSection FileItem::m_Cs;
