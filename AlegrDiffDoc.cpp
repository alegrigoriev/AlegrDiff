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
	: m_bRecurseSubdirs(true),
	m_pPairList(NULL)
{
	// TODO: add one-time construction code here
	MiltiSzToCString(m_sInclusionPattern, "*\0");
	MiltiSzToCString(m_sExclusionPattern, "*.ncb\0");
	MiltiSzToCString(m_sCFilesPattern, "*.C\0*.cpp\0*.h\0*.hpp\0*.inl\0*.rc\0*.h++");
	MiltiSzToCString(m_sBinaryFilesPattern, "*.exe\0*.dll\0*.sys\0*.obj\0*.pdb\0");
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

bool CAlegrDiffDoc::BuildFilePairList(LPCTSTR dir1, LPCTSTR dir2)
{
	// look through all files in the directory and subdirs
	bool res1, res2;

	m_FileList1.FreeFileList();
	m_FileList2.FreeFileList();
	res1 = m_FileList1.LoadFolder(dir1, m_bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern);
	res2 = m_FileList2.LoadFolder(dir1, m_bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern);

	if (res1 && res2)
	{
		CArray<FileItem *, FileItem *> Files1;
		CArray<FileItem *, FileItem *> Files2;

		m_FileList1.GetSortedList(Files1, FileList::SortDirFirst | FileList::SortBackwards);
		m_FileList2.GetSortedList(Files2, FileList::SortDirFirst | FileList::SortBackwards);

		//m_pPairList = NULL;

		for (int idx1 = 0, idx2 = 0; idx1 < Files1.GetSize() || idx2 < Files2.GetSize(); )
		{
			FilePair * pPair = new FilePair;
			pPair->pNext = m_pPairList;
			m_pPairList = pPair;
			if (idx1 >= Files1.GetSize())
			{
				pPair->pFirstFile = NULL;
				pPair->pSecondFile = Files2[idx2];
				idx2++;
				continue;
			}
			if (idx2 >= Files2.GetSize())
			{
				pPair->pSecondFile = NULL;
				pPair->pFirstFile = Files1[idx1];
				idx1++;
				continue;
			}
			int comparision = FileItem::FileItemDirNameCompare(Files1[idx1], Files2[idx2]);
			if (comparision < 0)
			{
				pPair->pFirstFile = NULL;
				pPair->pSecondFile = Files2[idx2];
				idx2++;
			}
			else if (comparision > 0)
			{
				pPair->pSecondFile = NULL;
				pPair->pFirstFile = Files1[idx1];
				idx1++;
			}
			else
			{
				pPair->pFirstFile = Files1[idx1];
				idx1++;
				pPair->pSecondFile = Files2[idx2];
				idx2++;
			}
		}
	}
	return true;
}

void CAlegrDiffDoc::FreeFilePairList()
{
	FilePair * tmp;
	while (NULL != m_pPairList)
	{
		tmp = m_pPairList;
		m_pPairList = tmp->pNext;
		delete tmp;
	}
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
	dlg.m_sFirstDir = m_sFirstDir;
	dlg.m_sSecondDir = m_sSecondDir;
	dlg.m_bIncludeSubdirs = m_bRecurseSubdirs;
	if (IDOK == dlg.DoModal())
	{
		m_sFirstDir = dlg.m_sFirstDir;
		m_sSecondDir = dlg.m_sSecondDir;
		m_bRecurseSubdirs = (1 == dlg.m_bIncludeSubdirs);
		FreeFilePairList();
		BuildFilePairList(dlg.m_sFirstDir, dlg.m_sSecondDir);
//        CompareFileLists();
	}
}

