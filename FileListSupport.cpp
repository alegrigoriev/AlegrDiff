// FileListSupport.cpp
#include "stdafx.h"
#include "FileListSupport.h"
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
}

FileItem::~FileItem()
{
	Unload();
}

// sort directories first then names
int _cdecl FileItem::FileItemDirNameSortFunc(const void * p1, const void * p2)
{
	return FileItemDirNameCompare(*(FileItem * const *) p1, *(FileItem * const *) p2);
}

int _cdecl FileItem::FileItemDirNameSortBackwardsFunc(const void * p1, const void * p2)
{
	return FileItemDirNameCompare(*(FileItem * const *) p2, *(FileItem * const *) p1);
}

int FileItem::FileItemDirNameCompare(FileItem * Item1, FileItem * Item2)
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
int _cdecl FileItem::FileItemNameSortFunc(const void * p1, const void * p2)
{
	return FileItemNameCompare(*(FileItem * const *) p1, *(FileItem * const *) p2);
}

int _cdecl FileItem::FileItemNameSortBackwardsFunc(const void * p1, const void * p2)
{
	return FileItemNameCompare(*(FileItem * const *) p2, *(FileItem * const *) p1);
}

int FileItem::FileItemNameCompare(FileItem * Item1, FileItem * Item2)
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


int _cdecl FileItem::FileItemTimeSortFunc(const void * p1, const void * p2)
{
	return FileItemTimeCompare(*(FileItem * const *) p1, *(FileItem * const *) p2);
}

int _cdecl FileItem::FileItemTimeSortBackwardsFunc(const void * p1, const void * p2)
{
	return FileItemTimeCompare(*(FileItem * const *) p2, *(FileItem * const *) p1);
}

int FileItem::FileItemTimeCompare(FileItem * Item1, FileItem * Item2)
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
		return FileItemNameCompare(Item1, Item2);
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
		SortFunc = FileItem::FileItemNameSortFunc;
		break;
	default:
	case SortDirFirst:
		SortFunc = FileItem::FileItemDirNameSortFunc;
		break;
	case SortDataModified:
		SortFunc = FileItem::FileItemTimeSortFunc;
		break;
	case SortNameFirst | SortBackwards:
		SortFunc = FileItem::FileItemNameSortBackwardsFunc;
		break;
	case SortDirFirst | SortBackwards:
		SortFunc = FileItem::FileItemDirNameSortBackwardsFunc;
		break;
	case SortDataModified | SortBackwards:
		SortFunc = FileItem::FileItemTimeSortBackwardsFunc;
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

FileLine::FileLine(const char * src, size_t length, int OrdNum)
	: m_HashCode(0), m_Flags(0), m_Number(OrdNum)
{
	m_Length = length;
	memcpy(m_Data, src, length);
	m_Data[length] = 0;
	m_HashCode = CalculateHash(m_Data, length);
}

CString FilePair::GetComparisionResult()
{
	if (NULL == pFirstFile)
	{
		if (NULL == pSecondFile)
		{
			return CString();
		}
		CString s;
		s.Format("File exists only in \"%s%s\"",
				pSecondFile->GetBasedir(), pSecondFile->GetSubdir());
		return s;
	}
	if (NULL == pSecondFile)
	{
		CString s;
		s.Format("File exists only in \"%s%s\"",
				pFirstFile->GetBasedir(), pFirstFile->GetSubdir());
		return s;
	}
	return CString();
}

#undef new
bool FileItem::Load()
{
	FILE * file = fopen(m_Subdir + m_Name, "rt");
	if (NULL == file)
	{
		return false;
	}
	char line[2048];
	for (int LinNum =0; NULL != fgets(line, sizeof line, file); LinNum++)
	{
		size_t length = strlen(line);
		FileLine * pLine = new (length) FileLine(line, length, LinNum);
		m_Lines.Add(pLine);
	}
	fclose(file);
	return true;
}

