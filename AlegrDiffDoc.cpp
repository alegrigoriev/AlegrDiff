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
	CThisApp * pApp = GetApp();
	m_sInclusionPattern = PatternToMultiCString(pApp->m_sFilenameFilter);
	if (m_sInclusionPattern.IsEmpty())
	{
		m_sInclusionPattern = "*";
	}
	if (pApp->m_bUseIgnoreFilter)
	{
		m_sExclusionPattern = PatternToMultiCString(pApp->m_sIgnoreFilesFilter);
	}
	if (pApp->m_bUseCppFilter)
	{
		m_sCFilesPattern = PatternToMultiCString(pApp->m_sCppFilesFilter);
	}
	if (pApp->m_bUseBinaryFilesFilter)
	{
		m_sBinaryFilesPattern = PatternToMultiCString(pApp->m_sBinaryFilesFilter);
	}
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
	m_BaseOnFirstFile(true),
	m_CaretPos(0, 0)
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
		if (NULL != pPair->pFirstFile)
		{
			CString title(pPair->pFirstFile->GetBasedir());
			title += pPair->pFirstFile->GetSubdir();
			title += pPair->pFirstFile->GetName();
			if (NULL != pPair->pSecondFile)
			{
				title += " - ";
				title += pPair->pSecondFile->GetBasedir();
				title += pPair->pSecondFile->GetSubdir();
				title += pPair->pSecondFile->GetName();
			}
			SetTitle(title);
		}
		else if (NULL != pPair->pSecondFile)
		{
			CString title(pPair->pSecondFile->GetBasedir());
			title += pPair->pSecondFile->GetSubdir();
			title += pPair->pSecondFile->GetName();
			SetTitle(title);
		}
		else
		{
			SetTitle("");
		}
	}
	UpdateAllViews(NULL, FileLoaded);
	SetCaretPosition(0, 0, SetPositionCancelSelection);
}

void CFilePairDoc::SetCaretPosition(int pos, int line, int flags)
{
	if (line > GetTotalLines())
	{
		line = GetTotalLines();
	}
	if (line < 0)
	{
		line = 0;
	}
	m_CaretPos.line = line;

	if (pos < 0)
	{
		pos = 0;
	}
	if (pos > 2048)
	{
		pos = 2048;
	}
	m_CaretPos.pos = pos;

	if (0 != (flags & SetPositionCancelSelection))
	{
		m_SelectionAnchor = m_CaretPos;
	}
	UpdateAllViews(NULL, CaretPositionChanged, NULL);
}

void CFilePairDoc::OnEditGotonextdiff()
{
	TextPos NewPos = m_pFilePair->NextDifference(m_CaretPos);
	if (NewPos == TextPos(-1, -1))
	{
		return;
	}
	SetCaretPosition(NewPos.pos, NewPos.line, SetPositionCancelSelection);
}

void CFilePairDoc::OnEditGotoprevdiff()
{
	TextPos NewPos = m_pFilePair->PrevDifference(m_CaretPos);
	if (NewPos == TextPos(-1, -1))
	{
		return;
	}
	SetCaretPosition(NewPos.pos, NewPos.line, SetPositionCancelSelection);
}

void CFilePairDoc::CaretToHome(int flags)
{
	// find the first non-space position
	// if the cursor is on this position, go to pos 0, otherwise to this pos.
	if (m_CaretPos.line >= m_TotalLines)
	{
		SetCaretPosition(0, m_CaretPos.line, flags);
		return;
	}
	LinePair * pLine = m_pFilePair->m_LinePairs[m_CaretPos.line];
	if (NULL != pLine)
	{
		int pos = 0;

		for (StringSection * pSection = pLine->pFirstSection; pSection != NULL; pSection = pSection->pNext)
		{
			for (int i = 0; i < pSection->Length; pos++)
			{
				if (pSection->pBegin[i] == '\t')
				{
					if (0 != (pos + 1) % GetApp()->m_TabIndent)
					{
						continue;
					}
					// otherwise i is incremented
				}
				else if (pSection->pBegin[i] != ' ')
				{
					// non-space character
					int SetToPos = pos;
					if (m_CaretPos.pos == pos)
					{
						SetToPos = 0;
					}
					SetCaretPosition(SetToPos, m_CaretPos.line, flags);
					return;
				}
				i++;
			}
		}
	}
	SetCaretPosition(0, m_CaretPos.line, flags);
}

void CFilePairDoc::CaretToEnd(int flags)
{
	if (m_CaretPos.line >= m_TotalLines)
	{
		SetCaretPosition(0, m_CaretPos.line, flags);
		return;
	}
	LinePair * pLine = m_pFilePair->m_LinePairs[m_CaretPos.line];
	int pos = 0;
	if (NULL != pLine)
	{
		for (StringSection * pSection = pLine->pFirstSection; pSection != NULL; pSection = pSection->pNext)
		{
			for (int i = 0; i < pSection->Length; pos++)
			{
				if (pSection->pBegin[i] != '\t'
					|| 0 == (pos + 1) % GetApp()->m_TabIndent)
				{
					i++;
				}
			}
		}
	}
	SetCaretPosition(pos, m_CaretPos.line, flags);
}

BEGIN_MESSAGE_MAP(CFilePairDoc, CDocument)
	//{{AFX_MSG_MAP(CFilePairDoc)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTONEXTDIFF, OnUpdateEditGotonextdiff)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTOPREVDIFF, OnUpdateEditGotoprevdiff)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
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

void CFilePairDoc::OnUpdateEditGotonextdiff(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pFilePair->NextDifference(m_CaretPos) != TextPos(-1, -1));
}

void CFilePairDoc::OnUpdateEditGotoprevdiff(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pFilePair->PrevDifference(m_CaretPos) != TextPos(-1, -1));
}

void CFilePairDoc::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_CaretPos != m_SelectionAnchor);
}

ULONG CFilePairDoc::CopyTextToMemory(PUCHAR pBuf, ULONG BufLen, TextPos pFrom, TextPos pTo)
{
	ULONG TotalChars = 0;
	CThisApp * pApp = GetApp();
	TextPos begin, end;
	if (pFrom < pTo)
	{
		begin = pFrom;
		end = pTo;
	}
	else
	{
		begin = pTo;
		end = pFrom;
	}
	if (begin.line < 0)
	{
		begin.line = 0;
	}
	if (end.line < 0)
	{
		end.line = 0;
	}
	if (begin.line > m_TotalLines - 1)
	{
		begin.line = m_TotalLines - 1;
	}
	if (end.line > m_TotalLines - 1)
	{
		end.line = m_TotalLines - 1;
	}

	for (int line = begin.line; line <= end.line; line++)
	{
		int startpos = 0;
		if (line == begin.line)
		{
			startpos = begin.pos;
		}
		int pos = 0;
		LinePair * pPair = m_pFilePair->m_LinePairs[line];

		StringSection * pSection = pPair->pFirstSection;
		for ( ; pSection != NULL; pSection = pSection->pNext)
		{
			for (int i = 0; i < pSection->Length; pos++, i++)
			{
				if (pos >= end.pos && line == end.line)
				{
					break;
				}
				if (pos >= startpos)
				{
					// store in the buffer, if there is room
					if (pBuf != NULL && BufLen != 0)
					{
						*pBuf = pSection->pBegin[i];
						BufLen--;
						pBuf++;
					}
					TotalChars++;
				}
				if (pSection->pBegin[i] == '\t')
				{
					pos += pApp->m_TabIndent - (pos % pApp->m_TabIndent + 1);
				}
			}
		}
		// add CR LF
		if (line < end.line)
		{
			if (pBuf != NULL
				&& BufLen > 1)
			{
				pBuf[0] = '\r';
				pBuf[1] = '\n';
				BufLen -= 2;
				pBuf += 2;
			}
			TotalChars += 2;
		}
	}
	if (pBuf != NULL
		&& BufLen > 0)
	{
		pBuf[0] = 0;
		BufLen --;
		pBuf ++;
	}
	TotalChars += 2;
	return TotalChars;
}

void CFilePairDoc::OnEditCopy()
{
	// calculate length of the selection
	ULONG Len = CopyTextToMemory(NULL, 0, m_SelectionAnchor, m_CaretPos);
	if (0 == Len)
	{
		return;
	}
	//	allocate memory
	HGLOBAL hMem = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, Len + 1);
	if (NULL == hMem)
	{
		return;
	}
	PUCHAR pMem = PUCHAR(GlobalLock(hMem));
	// Open and erase clipboard
	if (NULL != pMem
		&& AfxGetMainWnd()->OpenClipboard())
	{
		EmptyClipboard();
		CopyTextToMemory(pMem, Len, m_SelectionAnchor, m_CaretPos);
		GlobalUnlock(hMem);

		SetClipboardData(CF_TEXT, hMem);
		// set text to clipboard
		CloseClipboard();
	}
	else
	{
		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}
}
