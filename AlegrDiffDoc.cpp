// AlegrDiffDoc.cpp : implementation of the CAlegrDiffDoc class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "AlegrDiffDoc.h"
#include "CompareDirsDialog.h"
#include "DiffFileView.h"

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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc construction/destruction

CAlegrDiffDoc::CAlegrDiffDoc()
	: m_nFilePairs(0),
	m_pPairList(NULL)
{
	// TODO: add one-time construction code here
	m_sInclusionPattern = MiltiSzToCString("*\0");
	m_sExclusionPattern = MiltiSzToCString("*.ncb\0");
	m_sCFilesPattern = MiltiSzToCString("*.C\0*.cpp\0*.h\0*.hpp\0*.inl\0*.rc\0*.h++");
	m_sBinaryFilesPattern = MiltiSzToCString("*.exe\0*.dll\0*.sys\0*.obj\0*.pdb\0");
}

CAlegrDiffDoc::~CAlegrDiffDoc()
{
	FreeFilePairList();
}

BOOL CAlegrDiffDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

bool CAlegrDiffDoc::BuildFilePairList(LPCTSTR dir1, LPCTSTR dir2, bool bRecurseSubdirs)
{
	// look through all files in the directory and subdirs

	FreeFilePairList();
	FileList FileList1;
	FileList FileList2;
	if (! FileList1.LoadFolder(dir1, bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern))
	{
		DWORD error = GetLastError();
		return false;
	}
	if (! FileList2.LoadFolder(dir2, bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern))
	{
		return false;
	}
	{
		CString title = dir1;
		title += _T(" - ");
		title += dir2;
		SetTitle(title);
	}

	CArray<FileItem *, FileItem *> Files1;
	CArray<FileItem *, FileItem *> Files2;

	FileList1.GetSortedList(Files1, FileList::SortDirFirst | FileList::SortBackwards);
	FileList2.GetSortedList(Files2, FileList::SortDirFirst | FileList::SortBackwards);


	for (int idx1 = 0, idx2 = 0; idx1 < Files1.GetSize() || idx2 < Files2.GetSize(); )
	{
		FilePair * pPair = new FilePair;
		pPair->pNext = m_pPairList;
		m_pPairList = pPair;
		m_nFilePairs++;
		if (idx1 >= Files1.GetSize())
		{
			pPair->pFirstFile = NULL;
			pPair->pSecondFile = Files2[idx2];
			pPair->ComparisionResult = FilePair::OnlySecondFile;
			idx2++;

			if (0) TRACE("File \"%s\" exists only in dir \"%s\"\n",
						pPair->pSecondFile->GetName(),
						FileList2.m_BaseDir + pPair->pSecondFile->GetSubdir());
			continue;
		}
		if (idx2 >= Files2.GetSize())
		{
			pPair->pSecondFile = NULL;
			pPair->pFirstFile = Files1[idx1];
			pPair->ComparisionResult = FilePair::OnlyFirstFile;
			idx1++;

			if (0) TRACE("File \"%s\" exists only in dir \"%s\"\n",
						pPair->pFirstFile->GetName(),
						FileList1.m_BaseDir + pPair->pFirstFile->GetSubdir());
			continue;
		}
		int comparision = FileItem::DirNameCompare(Files1[idx1], Files2[idx2]);
		if (comparision < 0)
		{
			pPair->pFirstFile = NULL;
			pPair->pSecondFile = Files2[idx2];
			pPair->ComparisionResult = FilePair::OnlySecondFile;
			idx2++;

			if (0) TRACE("File \"%s\" exists only in dir \"%s\"\n",
						pPair->pSecondFile->GetName(),
						FileList2.m_BaseDir + pPair->pSecondFile->GetSubdir());
		}
		else if (comparision > 0)
		{
			pPair->pSecondFile = NULL;
			pPair->pFirstFile = Files1[idx1];
			pPair->ComparisionResult = FilePair::OnlyFirstFile;
			idx1++;

			if (0) TRACE("File \"%s\" exists only in dir \"%s\"\n",
						pPair->pFirstFile->GetName(),
						FileList1.m_BaseDir + pPair->pFirstFile->GetSubdir());
		}
		else
		{
			pPair->pFirstFile = Files1[idx1];
			idx1++;
			pPair->pSecondFile = Files2[idx2];
			idx2++;
			pPair->ComparisionResult = pPair->PreCompareFiles();
			if (0) TRACE("File \"%s\" exists in both \"%s\" and \"%s\"\n",
						pPair->pFirstFile->GetName(),
						FileList1.m_BaseDir + pPair->pFirstFile->GetSubdir(),
						FileList2.m_BaseDir + pPair->pSecondFile->GetSubdir());
		}
	}
	// all files are referenced in FilePair list
	FileList1.Detach();
	FileList2.Detach();
	return true;
}

void CAlegrDiffDoc::FreeFilePairList()
{
	FilePair * tmp;
	while (NULL != m_pPairList)
	{
		tmp = m_pPairList;
		m_pPairList = tmp->pNext;
		tmp->Dereference();
	}
	m_nFilePairs = 0;
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

/////////////////////////////////////////////////////////////////////////////
// CFilePairDoc

IMPLEMENT_DYNCREATE(CFilePairDoc, CDocument)

CFilePairDoc::CFilePairDoc()
	: m_TotalLines(0),
	m_UseLinePairArray(false),
	m_pFilePair(NULL),
	m_BaseOnFirstFile(true)
{
}

BOOL CFilePairDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CFilePairDoc::~CFilePairDoc()
{
	if (NULL != m_pFilePair)
	{
		m_pFilePair->UnloadFiles();
		m_pFilePair->Dereference();
		m_pFilePair = NULL;
	}
}

void CFilePairDoc::SetFilePair(FilePair * pPair)
{
	if (NULL != m_pFilePair)
	{
		m_pFilePair->Dereference();
	}
	m_pFilePair = pPair;
	if (NULL != pPair)
	{
		pPair->Reference();
		if (0 == pPair->m_LinePairs.GetSize())
		{
			pPair->CompareFiles();
		}
		// build the line pair array
		if (pPair->m_LinePairs.GetSize() != 0)
		{
			m_UseLinePairArray = true;
			m_TotalLines = pPair->m_LinePairs.GetSize();
		}
		else
		{
			m_UseLinePairArray = false;
			FileItem * pFile = pPair->pFirstFile;
			if (NULL == pFile
				|| (! m_BaseOnFirstFile && NULL != pPair->pSecondFile))
			{
				pFile = pPair->pSecondFile;
			}

			m_TotalLines = pFile->GetNumLines();
		}
	}
	UpdateAllViews(NULL);
}

BEGIN_MESSAGE_MAP(CFilePairDoc, CDocument)
	//{{AFX_MSG_MAP(CFilePairDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilePairDoc diagnostics

#ifdef _DEBUG
void CFilePairDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFilePairDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFilePairDoc serialization

void CFilePairDoc::Serialize(CArchive& ar)
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
// CFilePairDoc commands
