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
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc construction/destruction

CAlegrDiffDoc::CAlegrDiffDoc()
	: m_nFilePairs(0),
	m_bRecurseSubdirs(false),
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

	m_bRecurseSubdirs = bRecurseSubdirs;
	FileList FileList1;
	FileList FileList2;
	if (! FileList1.LoadFolder(dir1, bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern))
	{
		DWORD error = GetLastError();
		FreeFilePairList();
		return false;
	}
	if (! FileList2.LoadFolder(dir2, bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern))
	{
		FreeFilePairList();
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


	FreeFilePairList();
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

			CString s;
			s.Format(_T("Comparing %s - %s"),
					LPCTSTR(pPair->pFirstFile->GetFullName()),
					LPCTSTR(pPair->pSecondFile->GetFullName()));
			((CFrameWnd*)AfxGetMainWnd())->SetMessageText(s);

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
	((CFrameWnd*)AfxGetMainWnd())->SetMessageText(AFX_IDS_IDLEMESSAGE);
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
		if (NULL != pPair->pFirstFile)
		{
			CString title(pPair->pFirstFile->GetFullName());
			if (NULL != pPair->pSecondFile)
			{
				title += " - ";
				title += pPair->pSecondFile->GetFullName();
			}
			SetTitle(title);
		}
		else if (NULL != pPair->pSecondFile)
		{
			CString title(pPair->pSecondFile->GetFullName());
			SetTitle(title);
		}
		else
		{
			SetTitle("");
		}

		if (0 == pPair->m_LinePairs.GetSize())
		{
			//UpdateAllViews(NULL, 0);    // erase the views
			((CFrameWnd*)AfxGetMainWnd())->SetMessageText(_T("Loading and comparing files..."));
			pPair->CompareFiles();
			((CFrameWnd*)AfxGetMainWnd())->SetMessageText(AFX_IDS_IDLEMESSAGE);
		}

		m_UseLinePairArray = true;
		m_TotalLines = pPair->m_LinePairs.GetSize();
	}
	UpdateAllViews(NULL, FileLoaded);
	SetCaretPosition(0, 0, SetPositionCancelSelection);
}

void CFilePairDoc::SetSelection(TextPos CaretPos, TextPos AnchorPos, int flags)
{
	m_SelectionAnchor = AnchorPos;
	SetCaretPosition(CaretPos.pos, CaretPos.line, flags & ~SetPositionCancelSelection);
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
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_UPDATE_COMMAND_UI(ID_FILE_EDIT_FIRST, OnUpdateFileEditFirst)
	ON_COMMAND(ID_FILE_EDIT_FIRST, OnFileEditFirst)
	ON_UPDATE_COMMAND_UI(ID_FILE_EDIT_SECOND, OnUpdateFileEditSecond)
	ON_COMMAND(ID_FILE_EDIT_SECOND, OnFileEditSecond)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CARET_POS, OnUpdateCaretPosIndicator)
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

void CFilePairDoc::OnFileSave()
{
	// TODO: Add your command handler code here

}

void CAlegrDiffDoc::OnFileSave()
{
	// TODO: Add your command handler code here

}

void CFilePairDoc::OnUpdateCaretPosIndicator(CCmdUI* pCmdUI)
{
	CString s;
	s.Format("Ln %d, Col %d", m_CaretPos.line, m_CaretPos.pos);
	pCmdUI->SetText(s);
}


void CAlegrDiffDoc::OnViewRefresh()
{
	// TODO: Add your command handler code here
	// rescan the directories again
}

void CFilePairDoc::OnViewRefresh()
{
	if (NULL == m_pFilePair)
	{
		return;
	}
	PairCheckResult res1 = m_pFilePair->ReloadIfChanged();
	if (FilesDeleted == res1)
	{
		// close this document
		OnCloseDocument();
		return;
	}
	if (FilesUnchanged != res1)
	{
		TRACE("Reloading the files\n");
		TextPos caretpos = m_CaretPos;
		m_pFilePair->Reference();
		SetFilePair(m_pFilePair);
		m_pFilePair->Dereference();
		SetCaretPosition(caretpos.pos, caretpos.line, SetPositionCancelSelection);
		//UpdateAllViews(NULL, FileLoaded);
	}
}

void CFilePairDoc::OnUpdateFileEditFirst(CCmdUI* pCmdUI)
{
	if (m_pFilePair != NULL && m_pFilePair->pFirstFile != NULL)
	{
		CString s(_T("&1 Open "));
		CString name(m_pFilePair->pFirstFile->GetFullName());
		// duplicate all '&' in name
		for (int i = 0; i < name.GetLength(); i++)
		{
			if (name[i] == '&')
			{
				name.Insert(i, '&');
				i++;
			}
		}

		s += name;
		pCmdUI->SetText(s);
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CFilePairDoc::OnFileEditFirst()
{
	if (m_pFilePair != NULL && m_pFilePair->pFirstFile != NULL)
	{
		CString name = m_pFilePair->pFirstFile->GetFullName();
		SHELLEXECUTEINFO shex;
		memset( & shex, 0, sizeof shex);
		shex.cbSize = sizeof shex;
		shex.hwnd = AfxGetMainWnd()->m_hWnd;
		//shex.lpVerb = _T("Open");
		shex.lpFile = name;
		shex.nShow = SW_SHOWDEFAULT;
		ShellExecuteEx( & shex);
	}
}

void CFilePairDoc::OnUpdateFileEditSecond(CCmdUI* pCmdUI)
{
	if (m_pFilePair != NULL && m_pFilePair->pSecondFile != NULL)
	{
		CString s(_T("&2 Open "));
		CString name(m_pFilePair->pSecondFile->GetFullName());
		// duplicate all '&' in name
		for (int i = 0; i < name.GetLength(); i++)
		{
			if (name[i] == '&')
			{
				name.Insert(i, '&');
				i++;
			}
		}

		s += name;
		pCmdUI->SetText(s);
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CFilePairDoc::OnFileEditSecond()
{
	if (m_pFilePair != NULL && m_pFilePair->pSecondFile != NULL)
	{
		CString name = m_pFilePair->pSecondFile->GetFullName();
		SHELLEXECUTEINFO shex;
		memset( & shex, 0, sizeof shex);
		shex.cbSize = sizeof shex;
		shex.hwnd = AfxGetMainWnd()->m_hWnd;
		//shex.lpVerb = _T("Open");
		shex.lpFile = name;
		shex.nShow = SW_SHOWDEFAULT;
		ShellExecuteEx( & shex);
	}
}

bool CFilePairDoc::FindTextString(LPCTSTR pStrToFind, bool bBackward, bool bCaseSensitive)
{
	// find from the current position
	if (NULL == m_pFilePair
		|| NULL == pStrToFind
		|| 0 == pStrToFind[0]
		|| 0 == m_pFilePair->m_LinePairs.GetSize())
	{
		return false;
	}
	TCHAR line[2048];
	int nSearchPos;
	int nSearchLine;
	if (bBackward)
	{
		if (m_CaretPos > m_SelectionAnchor)
		{
			nSearchPos = m_CaretPos.pos - 1;
			nSearchLine = m_CaretPos.line;
		}
		else if (m_CaretPos < m_SelectionAnchor)
		{
			nSearchPos = m_SelectionAnchor.pos - 1;
			nSearchLine = m_SelectionAnchor.line;
		}
		else
		{
			nSearchPos = m_CaretPos.pos;
			nSearchLine = m_CaretPos.line;
		}
		if (nSearchLine >= m_pFilePair->m_LinePairs.GetSize())
		{
			nSearchLine--;
			nSearchPos = INT_MAX;
		}
	}
	else
	{
		if (m_CaretPos < m_SelectionAnchor)
		{
			nSearchPos = m_CaretPos.pos + 1;
			nSearchLine = m_CaretPos.line;
		}
		else if (m_CaretPos > m_SelectionAnchor)
		{
			nSearchPos = m_SelectionAnchor.pos + 1;
			nSearchLine = m_SelectionAnchor.line;
		}
		else
		{
			nSearchPos = m_CaretPos.pos;
			nSearchLine = m_CaretPos.line;
		}
		if (nSearchLine >= m_pFilePair->m_LinePairs.GetSize())
		{
			nSearchLine = 0;
			nSearchPos = 0;
		}
	}

	int nStartSearchPos = nSearchPos;
	int nStartSearchLine = nSearchLine;

	while(1)
	{
		LPCTSTR pStr = NULL;
		int StrLen = 0;
		pStr = GetLineText(nSearchLine, line, sizeof line / sizeof line[0], & StrLen);

		int nPatternLen = strlen(pStrToFind);
		if ( ! bBackward)
		{
			for ( ;nSearchPos < StrLen - nPatternLen; nSearchPos++)
			{
				if (bCaseSensitive)
				{
					if (0 == _tcsncmp(pStr + nSearchPos, pStrToFind, nPatternLen))
					{
						// found
						SetSelection(TextPos(nSearchLine, nSearchPos + nPatternLen), TextPos(nSearchLine, nSearchPos));
						return true;
					}
				}
				else
				{
					if (0 ==_tcsnicmp(pStr + nSearchPos, pStrToFind, nPatternLen))
					{
						// found
						SetSelection(TextPos(nSearchLine, nSearchPos + nPatternLen), TextPos(nSearchLine, nSearchPos));
						return true;
					}
				}
			}
			if (nSearchLine == nStartSearchLine)
			{
				if (-2 == nStartSearchPos)
				{
					break;
				}
				nStartSearchPos = -2;
			}
			nSearchLine++;
			nSearchPos = 0;
			if (nSearchLine >= m_pFilePair->m_LinePairs.GetSize())
			{
				// wraparound
				nSearchLine = 0;
			}
		}
		else
		{
			// search the line backwards
			if (nSearchPos > StrLen)
			{
				nSearchPos = StrLen;
			}
			for (nSearchPos -= nPatternLen; nSearchPos >= 0; nSearchPos--)
			{
				if (bCaseSensitive)
				{
					if (0 == _tcsncmp(pStr + nSearchPos, pStrToFind, nPatternLen))
					{
						// found
						SetSelection(TextPos(nSearchLine, nSearchPos + nPatternLen), TextPos(nSearchLine, nSearchPos));
						return true;
					}
				}
				else
				{
					if (0 ==_tcsnicmp(pStr + nSearchPos, pStrToFind, nPatternLen))
					{
						// found
						SetSelection(TextPos(nSearchLine, nSearchPos + nPatternLen), TextPos(nSearchLine, nSearchPos));
						return true;
					}
				}
			}
			if (nSearchLine == nStartSearchLine)
			{
				if (-2 == nStartSearchPos)
				{
					break;
				}
				nStartSearchPos = -2;
			}
			nSearchPos = INT_MAX;
			if (nSearchLine == 0)
			{
				nSearchLine = m_pFilePair->m_LinePairs.GetSize();
			}
			nSearchLine--;
		}
	}
	// end of file
	return false;
}

bool CFilePairDoc::GetWordUnderCaret(TextPos &Start, TextPos &End)
{
	if (m_CaretPos.line >= m_pFilePair->m_LinePairs.GetSize())
	{
		return false;
	}
	LinePair * pPair = m_pFilePair->m_LinePairs[m_CaretPos.line];
	if (NULL == pPair)
	{
		return false;
	}

	Start.line = m_CaretPos.line;
	End.line = m_CaretPos.line;

	int nPos = 0;
	int CaretPos = m_CaretPos.pos;
	for (StringSection * pSection = pPair->pFirstSection
		; pSection != NULL; nPos += pSection->Length, pSection = pSection->pNext)
	{
		if (CaretPos < nPos + pSection->Length)
		{
			// get a word under the caret and to the right, or take a single non-alpha char
			TCHAR c = pSection->pBegin[CaretPos - nPos];
			if (_istalnum(c) || '_' == c)
			{
				for (Start.pos = CaretPos; Start.pos > nPos; Start.pos--)
				{
					c = pSection->pBegin[Start.pos - nPos - 1];
					if (! _istalnum(c) && '_' != c)
					{
						break;
					}
				}
				for (End.pos = CaretPos + 1; End.pos < nPos + pSection->Length; End.pos++)
				{
					c = pSection->pBegin[End.pos - nPos];
					if (! _istalnum(c) && '_' != c)
					{
						break;
					}
				}
			}
			else
			{
				if (' ' == c)
				{
					// todo: look for the next non-space char
					return false;
				}
				// get one char under the cursor
				Start.pos = CaretPos;
				End.pos = CaretPos + 1;
			}
			return true;
		}
	}

	return false;
}

bool CFilePairDoc::OnEditFind()
{
	// TODO: Add your command handler code here
	return false;
}

bool CFilePairDoc::OnEditFindNext()
{
	CThisApp * pApp = GetApp();
	return FindTextString(pApp->m_FindString, false, pApp->m_bCaseSensitive);
}

bool CFilePairDoc::OnEditFindPrev()
{
	CThisApp * pApp = GetApp();
	return FindTextString(pApp->m_FindString, true, pApp->m_bCaseSensitive);
}

bool CFilePairDoc::OnEditFindWordNext()
{
	return FindWordOrSelection(false);
}

bool CFilePairDoc::OnEditFindWordPrev()
{
	return FindWordOrSelection(true);
}

bool CFilePairDoc::FindWordOrSelection(bool bBackwards)
{
	CThisApp * pApp = GetApp();
	int nBeginOffset;
	int nLength;
	if (m_CaretPos == m_SelectionAnchor
		|| m_CaretPos.line != m_SelectionAnchor.line)
	{
		TextPos Begin, End;
		if ( ! GetWordUnderCaret(Begin, End))
		{
			return false;
		}
		nBeginOffset = Begin.pos;
		nLength = End.pos - Begin.pos;
	}
	else
	{
		if (m_CaretPos.pos < m_SelectionAnchor.pos)
		{
			nBeginOffset = m_CaretPos.pos;
			nLength = m_SelectionAnchor.pos - m_CaretPos.pos;
		}
		else
		{
			nBeginOffset = m_SelectionAnchor.pos;
			nLength = m_CaretPos.pos - m_SelectionAnchor.pos;
		}
	}
	TCHAR line[2048];
	int StrLen;
	LPCTSTR pStr = GetLineText(m_CaretPos.line, line, sizeof line / sizeof line[0], & StrLen);
	if (NULL == pStr)
	{
		return false;
	}
	if (nBeginOffset >= StrLen)
	{
		return false;
	}
	if (nBeginOffset + nLength > StrLen)
	{
		nLength = StrLen - nBeginOffset;
	}
	LPTSTR StrBuf = pApp->m_FindString.GetBuffer(nLength);
	if (NULL == StrBuf)
	{
		return false;
	}
	_tcsncpy(StrBuf, pStr + nBeginOffset, nLength);
	pApp->m_FindString.ReleaseBuffer(nLength);
	return FindTextString(pApp->m_FindString, bBackwards, pApp->m_bCaseSensitive);
}

// returns a pointer to a line text
// buf is used to assembly the string if it is fragmented
LPCTSTR CFilePairDoc::GetLineText(int nLineNum, LPTSTR buf, size_t BufChars, int *pStrLen)
{
	LinePair * pPair = m_pFilePair->m_LinePairs[nLineNum];
	if (NULL == pPair)
	{
		return NULL;
	}
	return pPair->GetText(buf, BufChars, pStrLen);
}

void CFilePairDoc::CaretLeftToWord(bool bCancelSelection)
{
	return;
	TextPos Begin, End;
	// if the caret is on the begin of the line, go to the previous line
	TCHAR linebuf[2048];
	int StrLen;
	int CaretPos = m_CaretPos.pos;
	int CaretLine = m_CaretPos.line;

	LPCTSTR pLine = NULL;
	do
	{
		if (0 == CaretPos)
		{
			break;
		}
		pLine = GetLineText(CaretLine, linebuf, 2048, & StrLen);
		if (NULL == pLine)
		{
			break;
		}
		if (CaretPos > StrLen)
		{
			CaretPos = StrLen;
		}
		// check if there are any non-space characters before caret
		for ( ; CaretPos > 0 && ' ' == pLine[CaretPos - 1]; CaretPos--)
		{
		}
		if (0 == CaretPos)
		{
			pLine = NULL;
			break;
		}
	}
	while (0);
	if (NULL == pLine)
	{
		// get previous line
		if (CaretLine > 0)
		{
			CaretLine--;
			pLine = GetLineText(CaretLine, linebuf, 2048, & StrLen);
			if (CaretPos > StrLen)
			{
				CaretPos = StrLen;
			}
		}
	}
}
void CFilePairDoc::CaretRightToWord(bool bCancelSelection)
{
}
