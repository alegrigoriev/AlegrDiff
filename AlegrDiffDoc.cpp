// AlegrDiffDoc.cpp : implementation of the CAlegrDiffDoc class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "AlegrDiffDoc.h"
#include "CompareDirsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc

IMPLEMENT_DYNCREATE(CAlegrDiffDoc, CDocument)

BEGIN_MESSAGE_MAP(CAlegrDiffDoc, CDocument)
	//{{AFX_MSG_MAP(CAlegrDiffDoc)
	ON_COMMAND(ID_FILE_COMPAREDIRECTORIES, OnFileComparedirectories)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc construction/destruction

CAlegrDiffDoc::CAlegrDiffDoc()
{
	// TODO: add one-time construction code here

}

CAlegrDiffDoc::~CAlegrDiffDoc()
{
}

BOOL CAlegrDiffDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

#undef tolower
#undef toupper

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

bool MultiPatternMatches(LPCTSTR name, const CString & sPattern)
{
	// sPattern is MULTI_SZ

}

bool CAlegrDiffDoc::BuildFileList(LPCTSTR dir)
{
	WIN32_FIND_DATA wfd;
	CString Directory(dir);
	// make sure the name contains '\'.
	TCHAR c;
	if (Directory.IsEmpty())
	{
		c = 0;
	}
	else
	{
		c = Directory[Directory.GetLength() - 1];
	}
	if (':' != c
		&& '\\' != c
		&& '/' != c)
	{
		Directory += _T("\\");
	}
	CString name = Directory + '*';
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
			if ( ! m_bCompareSubdirectories)
			{
				continue;
			}
			// scan the subdirectory
		}
		else
		{
			// filter the file and add it to the list, if it matches the pattern.
			if (! MultiPatternMatches(wfd.cFileName, m_sInclusionPattern)
				|| MultiPatternMatches(wfd.cFileName, m_sExclusionPattern))
			{
				continue;
			}
			FileItem * pFile = new FileItem(wfd.cFileName, Directory);
			if (NULL == pFile)
			{
				continue;
			}
		}
	}
	while (FindNextFile(hFind, & wfd));
	FindClose(hFind);
	return true;
}

bool CAlegrDiffDoc::BuildFilePairList(LPCTSTR dir1, LPCTSTR dir2)
{
	// look through all files in the directory and subdirs
	bool res;
	res = BuildFileList(dir1);
	res = BuildFileList(dir2);
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc serialization

void CAlegrDiffDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc diagnostics

#ifdef _DEBUG
void CAlegrDiffDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAlegrDiffDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc commands
void CAlegrDiffDoc::OnFileComparedirectories()
{
	CCompareDirsDialog dlg;
	if (IDOK == dlg.DoModal())
	{
		BuildFilePairList(dlg.m_sFirstDirCombo, dlg.m_sSecondDirCombo);
//        CompareFileLists();
	}
}

