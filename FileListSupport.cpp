// FileListSupport.cpp
#include "stdafx.h"
#include "FileListSupport.h"
#include "AlegrDiff.h"

#undef tolower
#undef toupper

void MiltiSzToCString(CString & str, LPCTSTR pMsz)
{
	// find string length
	int l;
	// limit the string length to 64K
	for (l = 0; ('\0' != pMsz[l] || '\0' != pMsz[l + 1]) && l < 65536; l++)
	{
	}
	l += 2;
	LPTSTR pBuf = str.GetBuffer(l);
	if (NULL != pBuf)
	{
		memcpy(pBuf, pMsz, l);
		str.ReleaseBuffer(l - 1);
	}
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
			if (*name != * pattern
				&& *name != toupper(*pattern)
				&& *name != tolower(*pattern))
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

FileItem::FileItem(const CString & Name, const CString & BaseDir, const CString & Dir)
	:m_Name(Name),
	m_BaseDir(BaseDir),
	m_Subdir(Dir),
	m_pNext(NULL)
{
	m_LastWriteTime.dwHighDateTime = 0;
	m_LastWriteTime.dwLowDateTime = 0;
}
FileItem::FileItem(const WIN32_FIND_DATA * pWfd, const CString & BaseDir, const CString & Dir)
	:m_Name(pWfd->cFileName),
	m_BaseDir(BaseDir),
	m_Subdir(Dir),
	m_pNext(NULL)
{
	m_LastWriteTime = pWfd->ftLastWriteTime;
}

void FileItem::Unload()
{
	for (int i = 0; i < m_Lines.GetSize(); i++)
	{
		delete m_Lines[i];
	}
	m_Lines.RemoveAll();
	m_HashSortedLines.RemoveAll();
}

FileItem::~FileItem()
{
	Unload();
}

int _cdecl FileLine::HashCompareFunc(const void * p1, const void * p2)
{
	FileLine const * pLine1 = *(FileLine **) p1;
	FileLine const * pLine2 = *(FileLine **) p2;
	if (pLine1->GetHash() > pLine2->GetHash())
	{
		return -1;
	}
	if (pLine1->GetHash() < pLine2->GetHash())
	{
		return 1;
	}
	return 0;
}

int _cdecl FileLine::HashAndLineNumberCompareFunc(const void * p1, const void * p2)
{
	FileLine const * pLine1 = *(FileLine **) p1;
	FileLine const * pLine2 = *(FileLine **) p2;
	if (pLine1->GetHash() > pLine2->GetHash())
	{
		return 1;
	}
	if (pLine1->GetHash() < pLine2->GetHash())
	{
		return -1;
	}
	// if hash is the same, compare line numbers
	if (pLine1->GetLineNumber() > pLine2->GetLineNumber())
	{
		return 1;
	}
	if (pLine1->GetLineNumber() < pLine2->GetLineNumber())
	{
		return -1;
	}
	return 0;
}

int _cdecl FileLine::NormalizedHashAndLineNumberCompareFunc(const void * p1, const void * p2)
{
	FileLine const * pLine1 = *(FileLine **) p1;
	FileLine const * pLine2 = *(FileLine **) p2;
	if (pLine1->GetNormalizedHash() > pLine2->GetNormalizedHash())
	{
		return 1;
	}
	if (pLine1->GetNormalizedHash() < pLine2->GetNormalizedHash())
	{
		return -1;
	}
	// if hash is the same, compare line numbers
	if (pLine1->GetLineNumber() > pLine2->GetLineNumber())
	{
		return 1;
	}
	if (pLine1->GetLineNumber() < pLine2->GetLineNumber())
	{
		return -1;
	}
	return 0;
}

#undef new
bool FileItem::Load()
{
	FILE * file = fopen(m_BaseDir + m_Subdir + m_Name, "rt");
	if (NULL == file)
	{
		return false;
	}
	char line[2048];
	for (int LinNum =0; NULL != fgets(line, sizeof line, file); LinNum++)
	{
		FileLine * pLine = new FileLine(line, true);
		if (pLine)
		{
			pLine->SetLineNumber(LinNum);
			m_Lines.Add(pLine);
		}
	}
	fclose(file);

	int i, j;
	// make sorted array of the string hash values
	m_HashSortedLines.SetSize(LinNum);
	for (i = 0, j = 0; i < LinNum; i++)
	{
		if ( ! m_Lines[i]->IsBlank())
		{
			m_HashSortedLines[j] = m_Lines[i];
			j++;
		}
	}
	if (j != m_HashSortedLines.GetSize())
	{
		m_HashSortedLines.SetSize(j);
	}
	m_NormalizedHashSortedLines.Copy(m_HashSortedLines);

	qsort(m_HashSortedLines.GetData(), LinNum,
		sizeof m_HashSortedLines[0], FileLine::HashAndLineNumberCompareFunc);
	qsort(m_NormalizedHashSortedLines.GetData(), LinNum,
		sizeof m_NormalizedHashSortedLines[0], FileLine::NormalizedHashAndLineNumberCompareFunc);
#ifdef _DEBUG
	// check if the array is sorted correctly
	{
		int i;
		for (i = 1; i < m_HashSortedLines.GetSize(); i++)
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
		for (i = 1; i < m_NormalizedHashSortedLines.GetSize(); i++)
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
	}
#endif
	// make sorted array of the normalized string hash values

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
	int result = Item1->m_Name.CompareNoCase(Item2->m_Name);
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

	return Item1->m_Subdir.CompareNoCase(Item2->m_Subdir);
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

void * BinLookup(
				const void *key,
				const void *base,
				size_t num,
				size_t width,
				int (__cdecl *compare)(const void *, const void *)
				)
{
	char *lo = (char *)base;
	char *hi = (char *)base + (num - 1) * width;
	char *mid;
	unsigned int half;
	unsigned int uphalf;
	int result;

	while (lo <= hi)
	{
		half = num / 2;
		uphalf = num - half - 1;
		if (0 != half)
		{
			mid = lo + uphalf * width;
			result = compare(key,mid);
			if (0 == result)
			{
				return(mid);
			}
			else if (result < 0)
			{
				hi = mid - width;

				num = uphalf;
			}
			else
			{
				lo = mid + width;
				num = half;
			}
		}
		else if (num)
		{
			result = compare(key,lo);
			if (0 == result)
			{
				return lo;
			}
			else
				return NULL;
		}
		else
		{
			break;
		}
	}
	return(NULL);
}
// find equal or the next greater
template<class T, class A> const T * BinLookupAbout(
													const T *key,
													A ComparisionContext,
													const T *base,
													size_t num,
													int (*compare)(const T * item1, const T * item2, A ComparisionContext)
													)
{
	const T *lo = base;
	const T *hi = base + (num - 1);
	const T *mid;
	unsigned int half;
	unsigned int uphalf;
	int result;

	while (lo <= hi)
	{
		half = num / 2;
		uphalf = num - half - 1;
		if (0 != half)
		{
			mid = lo + uphalf;
			result = compare(key, mid, ComparisionContext);
			if (0 == result)
			{
				return mid;
			}
			else if (result < 0)
			{
				// choose lower half
				hi = mid - 1;

				num = uphalf;
			}
			else
			{
				// choose upper half
				lo = mid + 1;
				num = half;
			}
		}
		else if (num)
		{
			result = compare(key, lo, ComparisionContext);
			if (result <= 0)
			{
				return lo;
			}
			else
			{
				return hi;
			}
		}
		else
		{
			break;
		}
	}
	return lo;
}

// find the line with the same hash and same and greater number, return -1 if not found
int FileItem::FindMatchingLineIndex(const FileLine * pLine, int nStartLineIndex)
{
	void const * key = pLine;
	void * pFound = bsearch( & key, m_HashSortedLines.GetData(), m_HashSortedLines.GetSize(),
							sizeof m_HashSortedLines[0], FileLine::HashCompareFunc);
	if (NULL == pFound)
	{
		return -1;
	}
	if (pFound < m_HashSortedLines.GetData()
		|| pFound >= m_HashSortedLines.GetData() + m_HashSortedLines.GetSize())
	{
		TRACE("Inconsistent address returned from bsearch\n");
		return -1;
	}
	//int SortedIndex =
	FileLine * pFoundLine = *(FileLine **) pFound;
	// todo: there can be several lines with the same number
	return pFoundLine->GetLineNumber();
}


bool FileList::LoadFolder(const CString & BaseDir, bool bRecurseSubdirs,
						LPCTSTR sInclusionMask, LPCTSTR sExclusionMask)
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
	return LoadSubFolder(CString(), bRecurseSubdirs, sInclusionMask, sExclusionMask);
}

bool FileList::LoadSubFolder(const CString & Subdir, bool bRecurseSubdirs,
							LPCTSTR sInclusionMask, LPCTSTR sExclusionMask)
{
	TRACE("LoadSubFolder: scanning %s\n", LPCTSTR(Subdir));
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
							sInclusionMask, sExclusionMask);
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

DWORD CalculateHash(const char * data, int len)
{
	// CRC32
	DWORD	crc32_val = 0xFFFFFFFF;

	// Calculate a CRC32 value
	for (int i = 0 ; i < len; i++)
	{
		char c = data[i];
		// TODO: ignore whitespaces
		crc32_val = ( crc32_val << 8 ) ^ CRC32_Table[(( crc32_val >> 24) ^ c) & 0xff];
	}
	return crc32_val;
}
// remove the unnecessary whitespaces from the line (based on C, C++ syntax)
// return string length
static int RemoveExtraWhitespaces(LPTSTR pDst, LPCTSTR Src, int DstLen)
{
	// pDst buffer size must be greater or equal strlen(pSrc)
	int SrcIdx = 0;
	int DstIdx = 0;
	bool CanRemoveWhitespace = true;
	TCHAR cPrevChar = 0;
	bool PrevCharAlpha = false;

	TCHAR c;
	// remove whitespaces at the begin
	while ((c = Src[SrcIdx]) == ' '
			|| '\t' == c)
	{
		SrcIdx++;
	}
	while (Src[SrcIdx] && DstIdx < DstLen - 1)
	{
		// it's OK to remove extra spaces between non-alpha bytes,
		// unless these are the following pairs:
		// /*, */, //, ++, --, &&, ||, ##, ->*, ->, >>, << >=, <=, ==, ::
		// that is most two-char operators
		// strings also must be kept intact.
		// Can remove extra spaces between alpha and non-alpha
		c = Src[SrcIdx];
		bool c_IsAlpha = (0 != isalpha(c));
		if(c_IsAlpha)
		{
			if (PrevCharAlpha)
			{
				if(DstIdx >= DstLen - 1)
				{
					break;
				}
				// insert one whitespace to the output string
				pDst[DstIdx] = ' ';
				DstIdx++;
			}
			// move all alpha chars till non-alpha
			do
			{
				if(DstIdx >= DstLen - 1)
				{
					break;
				}
				pDst[DstIdx] = c;
				SrcIdx++;
				DstIdx++;
				c = Src[SrcIdx];
			} while (isalpha(c));

			PrevCharAlpha = true;
			CanRemoveWhitespace = true;
		}
		else
		{
			if( ! PrevCharAlpha)
			{
				// TODO: need to check for a space between #define name and (arguments)

				// check if we need to insert a whitespace
				static int ReservedPairs[] =
				{
					// /*, */, //, ++, --, &&, ||, ##, ->*, ->, >>, << >=, <=, ==, ::
					// TODO: process L' and L"
					'//', '/*', '*/', '++', '--', '!=', '##', '%=',
					'^=', '&=', '&&', '-=', '==', '::', '<<',
					'>>', '||', '|=', '<=', '>=', '/=', '\'\'', '""',
				};
				// may be non-portable to big-endian
				int pair = ((cPrevChar & 0xFF) << 8) | (c & 0xFF);
				for (int i = 0; i < sizeof ReservedPairs / sizeof ReservedPairs[0]; i++)
				{
					if (pair == ReservedPairs[i])
					{
						if(DstIdx < DstLen - 1)
						{
							pDst[DstIdx] = ' ';
							DstIdx++;
						}
						break;
					}
				}
			}
			// move all non-alpha non whitespace chars
			// check for a string or character constant
			if ('\'' == c)
			{
				// character constant
				// skip everything till the next '. Process \'
				cPrevChar = c;
				do
				{
					if(DstIdx >= DstLen - 1)
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
						if(DstIdx >= DstLen - 1)
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
						if(DstIdx >= DstLen - 1)
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
			else if ('"' == c)
			{
				// char string
				cPrevChar = c;
				do
				{
					if(DstIdx >= DstLen - 1)
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
						if(DstIdx >= DstLen - 1)
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
						if(DstIdx >= DstLen - 1)
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
				if(DstIdx >= DstLen - 1)
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
					&& ! isalpha(c));
			PrevCharAlpha = false;
		}
		// remove whitespaces
		while ((c = Src[SrcIdx]) == ' '
				|| '\t' == c)
		{
			SrcIdx++;
		}
	}
	pDst[DstIdx] = 0;
	return DstIdx;
}

static void RemoveExtraWhitespaces(CString & Dst, const CString & Src)
{
	// pDst buffer size must be grater or equal strlen(pSrc)
	int SrcIdx = 0;
	int DstIdx = 0;
	bool CanRemoveWhitespace = true;
	TCHAR cPrevChar = 0;
	bool PrevCharAlpha = false;
	LPTSTR pDst = Dst.GetBuffer(Src.GetLength());

	TCHAR c;
	// remove whitespaces at the begin
	while ((c = Src[SrcIdx]) == ' '
			|| '\t' == c)
	{
		SrcIdx++;
	}
	while (Src[SrcIdx])
	{
		// it's OK to remove extra spaces between non-alpha bytes,
		// unless these are the following pairs:
		// /*, */, //, ++, --, &&, ||, ##, ->*, ->, >>, << >=, <=, ==, ::
		// that is most two-char operators
		// strings also must be kept intact.
		// Can remove extra spaces between alpha and non-alpha
		c = Src[SrcIdx];
		bool c_IsAlpha = (0 != isalpha(c));
		if(c_IsAlpha)
		{
			if (PrevCharAlpha)
			{
				// insert one whitespace to the output string
				pDst[DstIdx] = ' ';
				DstIdx++;
			}
			// move all alpha chars till non-alpha
			do
			{
				pDst[DstIdx] = c;
				SrcIdx++;
				DstIdx++;
				c = Src[SrcIdx];
			} while (isalpha(c));

			PrevCharAlpha = true;
			CanRemoveWhitespace = true;
		}
		else
		{
			if( ! PrevCharAlpha)
			{
				// TODO: need to check for a space between #define name and (arguments)

				// check if we need to insert a whitespace
				static int ReservedPairs[] =
				{
					// /*, */, //, ++, --, &&, ||, ##, ->*, ->, >>, << >=, <=, ==, ::
					// TODO: process L' and L"
					'//', '/*', '*/', '++', '--', '!=', '##', '%=',
					'^=', '&=', '&&', '-=', '==', '::', '<<',
					'>>', '||', '|=', '<=', '>=', '/=', '\'\'', '""',
				};
				// may be non-portable to big-endian
				int pair = ((cPrevChar & 0xFF) << 8) | (c & 0xFF);
				for (int i = 0; i < sizeof ReservedPairs / sizeof ReservedPairs[0]; i++)
				{
					if (pair == ReservedPairs[i])
					{
						pDst[DstIdx] = ' ';
						DstIdx++;
						break;
					}
				}
			}
			// move all non-alpha non whitespace chars
			// check for a string or character constant
			if ('\'' == c)
			{
				// character constant
				// skip everything till the next '. Process \'
				cPrevChar = c;
				do
				{
					pDst[DstIdx] = c;
					SrcIdx++;
					DstIdx++;
					c = Src[SrcIdx];
					if ('\'' == c)
					{
						// if the next char is double quote,
						// skip both
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
						pDst[DstIdx] = c;
						SrcIdx++;
						DstIdx++;
						c = Src[SrcIdx];
					}
				} while (c != 0);
			}
			else if ('"' == c)
			{
				// char string
				cPrevChar = c;
				do
				{
					pDst[DstIdx] = c;
					SrcIdx++;
					DstIdx++;
					c = Src[SrcIdx];
					if ('"' == c)
					{
						// if the next char is double quote,
						// skip both
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
						pDst[DstIdx] = c;
						SrcIdx++;
						DstIdx++;
						c = Src[SrcIdx];
					}
				} while (c != 0);
			}
			else do
			{
				cPrevChar = c;
				pDst[DstIdx] = c;
				SrcIdx++;
				DstIdx++;
				c = Src[SrcIdx];
			} while (c != 0
					&& c != ' '
					&& c != '\t'
					&& ! isalpha(c));
			PrevCharAlpha = false;
		}
		// remove whitespaces
		while ((c = Src[SrcIdx]) == ' '
				|| '\t' == c)
		{
			SrcIdx++;
		}
	}
	Dst.ReleaseBuffer(DstIdx);
}

// find difference in the strings, and build the array of inserts and
// and deleted chars
void MatchStrings(LPCTSTR str1, LPCTSTR str2, DWORD Results[])
{
}

FileLine::FileLine(const char * src, bool MakeNormalizedString)
	: m_Flags(0), m_Number(-1), m_Link(NULL)
{
	m_Length = strlen(src);
	char TmpBuf[2048];
	m_NormalizedStringLength = RemoveExtraWhitespaces(TmpBuf, src, sizeof TmpBuf);

	m_pAllocatedBuf = new char[m_Length + m_NormalizedStringLength + 2];
	if (NULL != m_pAllocatedBuf)
	{
		m_pString = m_pAllocatedBuf;
		memcpy(m_pAllocatedBuf, src, m_Length + 1);
		m_HashCode = CalculateHash(m_pString, m_Length);

		m_pNormalizedString = m_pAllocatedBuf + m_Length + 1;
		memcpy(m_pAllocatedBuf + m_Length + 1, TmpBuf, m_NormalizedStringLength + 1);
		m_NormalizedHashCode = CalculateHash(m_pNormalizedString, m_NormalizedStringLength);

		if (0 == m_NormalizedStringLength)
		{
			m_Flags |= BlankString;
		}
	}
	else
	{
		m_pString = NULL;
		m_pNormalizedString = NULL;
		m_Length = 0;
		m_NormalizedStringLength = 0;

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
	return false;
}

int _cdecl FilePair::Time1SortFunc(const void * p1, const void * p2)
{
	FilePair * Pair1 = *(FilePair * const *) p1;
	FilePair * Pair2 = *(FilePair * const *) p2;

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

int _cdecl FilePair::Time1SortBackwardsFunc(const void * p1, const void * p2)
{
	FilePair * Pair1 = *(FilePair * const *) p1;
	FilePair * Pair2 = *(FilePair * const *) p2;

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

int _cdecl FilePair::Time2SortFunc(const void * p1, const void * p2)
{
	FilePair * Pair1 = *(FilePair * const *) p1;
	FilePair * Pair2 = *(FilePair * const *) p2;

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

int _cdecl FilePair::Time2SortBackwardsFunc(const void * p1, const void * p2)
{
	FilePair * Pair1 = *(FilePair * const *) p1;
	FilePair * Pair2 = *(FilePair * const *) p2;

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

int _cdecl FilePair::NameSortFunc(const void * p1, const void * p2)
{
	return NameCompare(*(FilePair * const *) p1, *(FilePair * const *) p2);
}

int _cdecl FilePair::NameSortBackwardsFunc(const void * p1, const void * p2)
{
	return NameCompare(*(FilePair * const *) p2, *(FilePair * const *) p1);
}

int FilePair::NameCompare(FilePair * Pair1, FilePair * Pair2)
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

int _cdecl FilePair::DirNameSortFunc(const void * p1, const void * p2)
{
	return DirNameCompare(*(FilePair * const *) p1, *(FilePair * const *) p2);
}

int _cdecl FilePair::DirNameSortBackwardsFunc(const void * p1, const void * p2)
{
	return DirNameCompare(*(FilePair * const *) p2, *(FilePair * const *) p1);
}

int FilePair::DirNameCompare(FilePair * Pair1, FilePair * Pair2)
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

int _cdecl FilePair::ComparisionSortFunc(const void * p1, const void * p2)
{
	return 0; (*(FilePair * const *) p2, *(FilePair * const *) p1);
}
int _cdecl FilePair::ComparisionSortBackwardsFunc(const void * p1, const void * p2)
{
	return 0; (*(FilePair * const *) p2, *(FilePair * const *) p1);
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

DWORD FilePair::CompareFiles(bool bCompareAll)
{
	if (NULL == pFirstFile
		|| NULL == pSecondFile)
	{
		return 0;
	}
	// TODO: different function for binary comparision
	if (! pFirstFile->Load()
		|| ! pSecondFile->Load())
	{
		pFirstFile->Unload();
		return 0;
	}
	// different comparision for different modes
	//return CompareTextFiles(bCompareAll);
	pFirstFile->Unload();
	pSecondFile->Unload();
	return 1;
}

int LineHashComparisionFunc(const FileLine * const * ppKeyLine,
							const FileLine * const * ppLine2, int LineNum)
{
	// use hash from *ppKeyLine, and LineNum as line number
	if ((*ppKeyLine)->GetNormalizedHash() > (*ppLine2)->GetNormalizedHash())
	{
		return 1;
	}
	if ((*ppKeyLine)->GetNormalizedHash() < (*ppLine2)->GetNormalizedHash())
	{
		return -1;
	}
	if (LineNum > (*ppLine2)->GetLineNumber())
	{
		return 1;
	}
	if (LineNum < (*ppLine2)->GetLineNumber())
	{
		return -1;
	}
	return 0;
}
DWORD FilePair::CompareTextFiles(bool bCompareAll)
{
	// find similar lines
	CThisApp * pApp = GetApp();
	int nLine1 = 0;
	int nLine2 = 0;
	int NumLines1 = pFirstFile->GetNumLines();
	int NumLines2 = pSecondFile->GetNumLines();
	// build list of equal sections
	while (nLine1 < NumLines1
			&& nLine2 < NumLines2)
	{
		// find the beginning of the section
		// find a few identical lines
		while (nLine1 < NumLines1
				&& nLine2 < NumLines2)
		{
			for (int dist = 0; dist < pApp->m_MaxSearchDistance; dist++)
			{
				const FileLine * Line1 = pFirstFile->GetLine(nLine1+dist);
				if ( ! Line1->IsBlank())
				{
					// check the lines in file2 in range Line2 to Line2+dist
#if 0
					for (int i = 0; i < dist && i + nLine2 < NumLines2; i++)
					{
						const FileLine * Line2 = pSecondFile->GetLine(nLine2 + i);
						if (Line1->IsEqual(Line2))
						{
							int n1 = nLine1 + 1;
							int n2 = nLine2 + i + 1;
							int NumEqual = 1;
							// check if a few non-blank lines more are the same
							while(n1 < NumLines1 && n2 < NumLines2
								&& NumEqual < pApp->m_MinIdenticalLines
								&& n1 - nLine1 < pApp->m_MaxSearchDistance)
							{
								const FileLine * L1 = pFirstFile->GetLine(n1);
								if (L1->IsBlank())
								{
									n1++;
									continue;
								}
								const FileLine * L2 = pSecondFile->GetLine(n2);
								if (L2->IsBlank())
								{
									n2++;
									continue;
								}
								if ( ! L1->IsEqual(L2))
								{
									break;
								}
								n1++;
								n2++;
								NumEqual++;
							}
							if (NumEqual >= pApp->m_MinIdenticalLines)
							{
								break;
							}
						}
					}
#else
					// find the similar string in the normalized string array
					// the line should go on or after nLine2
					FileLine const * const* ppLine = BinLookupAbout<const FileLine *, int>( & Line1, nLine2,
						(FileLine *const *) pSecondFile->m_NormalizedHashSortedLines.GetData(),
						pSecondFile->m_NormalizedHashSortedLines.GetSize(),
						LineHashComparisionFunc);
#ifdef _DEBUG
					{
						// verify that the correct position found
						int nIndex = ppLine - pSecondFile->m_NormalizedHashSortedLines.GetData();
						if (nIndex < 0 || nIndex >= pSecondFile->m_NormalizedHashSortedLines.GetSize())
						{
							TRACE("Wrong pointer in m_NormalizedHashSortedLines array\n");
						}
						else
						{
							//if (
						}
					}
#endif
#endif
				}
			}
		}
		int Line1Begin = nLine1;
		int Line2Begin = nLine2;

		while (nLine1 < NumLines1
				&& nLine2 < NumLines2)
		{
			const FileLine * Line1 = pFirstFile->GetLine(nLine1);
			const FileLine * Line2 = pSecondFile->GetLine(nLine2);
			if (Line1->IsEqual(Line2))
			{
				nLine1++;
				nLine2++;
			}
			else
			{
				// the lines are different
				if (! bCompareAll)
				{
					// if we don't need to compare the while file (just scanning)
					// return now
					return 1;
				}
				// check if the lines are similar enough
				// the lines can be considered similar if < 1/4 of the characters is different,
				// or the only difference is in whitespaces
				if (Line1->LooksLike(Line2, 25))
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
		FileSection * pSection = new FileSection;
		pSection->File1LineBegin = Line1Begin;

		pSection->File2LineBegin = Line2Begin;
		pSection->File1LineEnd = nLine1 - 1;
		pSection->File2LineEnd = nLine2 - 1;

	}
	return 1;
}

