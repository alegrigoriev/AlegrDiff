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

bool CAlegrDiffDoc::BuildFilePairList(LPCTSTR dir1, LPCTSTR dir2, bool bRecurseSubdirs)
{
	// look through all files in the directory and subdirs

	m_bRecurseSubdirs = bRecurseSubdirs;
	FileList FileList1;
	FileList FileList2;

	// make full names from the directories
	LPTSTR pFilePart;
	TCHAR buf[MAX_PATH];

	GetFullPathName(dir1, MAX_PATH, buf, & pFilePart);
	m_sFirstDir = buf;

	GetFullPathName(dir2, MAX_PATH, buf, & pFilePart);
	m_sSecondDir = buf;

	if (! FileList1.LoadFolder(m_sFirstDir, bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern, m_sCFilesPattern))
	{
		DWORD error = GetLastError();
		FreeFilePairList();
		CString s;
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, LPCTSTR(m_sFirstDir));
		AfxMessageBox(s);
		return false;
	}
	if (! FileList2.LoadFolder(m_sSecondDir, bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern, m_sCFilesPattern))
	{
		FreeFilePairList();
		CString s;
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, LPCTSTR(m_sSecondDir));
		AfxMessageBox(s);
		return false;
	}
	{
		CString title = m_sFirstDir;
		title += _T(" - ");
		title += m_sSecondDir;
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
			pPair->m_ComparisionResult = FilePair::OnlySecondFile;
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
			pPair->m_ComparisionResult = FilePair::OnlyFirstFile;
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
			pPair->m_ComparisionResult = FilePair::OnlySecondFile;
			idx2++;

			if (0) TRACE("File \"%s\" exists only in dir \"%s\"\n",
						pPair->pSecondFile->GetName(),
						FileList2.m_BaseDir + pPair->pSecondFile->GetSubdir());
		}
		else if (comparision > 0)
		{
			pPair->pSecondFile = NULL;
			pPair->pFirstFile = Files1[idx1];
			pPair->m_ComparisionResult = FilePair::OnlyFirstFile;
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

			pPair->m_ComparisionResult = pPair->PreCompareFiles();
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
	m_pFilePair(NULL),
	m_CaretPos(0, 0)
{
	m_bIgnoreWhitespaces = GetApp()->m_bIgnoreWhitespaces;
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
	TextPos NewPos = m_pFilePair->NextDifference(m_CaretPos, m_bIgnoreWhitespaces);
	if (NewPos == TextPos(-1, -1))
	{
		return;
	}
	NewPos = LinePosToDisplayPos(NewPos);
	SetCaretPosition(NewPos.pos, NewPos.line, SetPositionCancelSelection);
}

void CFilePairDoc::OnEditGotoprevdiff()
{
	TextPos NewPos = m_pFilePair->PrevDifference(m_CaretPos, m_bIgnoreWhitespaces);
	if (NewPos == TextPos(-1, -1))
	{
		return;
	}
	NewPos = LinePosToDisplayPos(NewPos);
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
			if ((pSection->Attr & pSection->Whitespace)
				&& (pSection->Attr & pSection->Erased)
				&& m_bIgnoreWhitespaces)
			{
				continue;   // don't show the section
			}
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
			if ((pSection->Attr & pSection->Whitespace)
				&& (pSection->Attr & pSection->Erased)
				&& m_bIgnoreWhitespaces)
			{
				continue;   // don't show the section
			}
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
	ON_COMMAND(ID_EDIT_ACCEPT, OnEditAccept)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ACCEPT, OnUpdateEditAccept)
	ON_COMMAND(ID_EDIT_DECLINE, OnEditDecline)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DECLINE, OnUpdateEditDecline)
	ON_COMMAND(ID_FILE_MERGE_SAVE, OnFileMergeSave)
	ON_COMMAND(ID_VIEW_IGNORE_WHITESPACES, OnViewIgnoreWhitespaces)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IGNORE_WHITESPACES, OnUpdateViewIgnoreWhitespaces)
	ON_UPDATE_COMMAND_UI(ID_FILE_COPY_FIRST_DIR_FILE, OnUpdateFileCopyFirstDirFile)
	ON_COMMAND(ID_FILE_COPY_FIRST_DIR_FILE, OnFileCopyFirstDirFile)
	ON_UPDATE_COMMAND_UI(ID_FILE_COPY_SECOND_DIR_FILE, OnUpdateFileCopySecondDirFile)
	ON_COMMAND(ID_FILE_COPY_SECOND_DIR_FILE, OnFileCopySecondDirFile)
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
// CFilePairDoc commands

void CFilePairDoc::OnUpdateEditGotonextdiff(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pFilePair->NextDifference(m_CaretPos, m_bIgnoreWhitespaces) != TextPos(-1, -1));
}

void CFilePairDoc::OnUpdateEditGotoprevdiff(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pFilePair->PrevDifference(m_CaretPos, m_bIgnoreWhitespaces) != TextPos(-1, -1));
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
			if ((pSection->Attr & pSection->Whitespace)
				&& (pSection->Attr & pSection->Erased)
				&& m_bIgnoreWhitespaces)
			{
				continue;   // don't show the section
			}
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
				// tabs are replaced with spaces
				//if (pSection->pBegin[i] == '\t')
				//{
				//pos += pApp->m_TabIndent - (pos % pApp->m_TabIndent + 1);
				//}
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
	PairCheckResult res1 = m_pFilePair->CheckForFilesChanged();
	if (FilesUnchanged == res1)
	{
		return;
	}
	// check if there are any changes labeled
	int flags = GetAcceptDeclineFlags(TextPos(0, 0), TextPos(GetTotalLines(), 0));
	if (0 != (flags & (FileDiffSection::FlagDecline | FileDiffSection::FlagAccept)))
	{
		CString s;
		s.Format(IDS_QUERY_RELOAD_FILES,
				m_pFilePair->pSecondFile->GetSubdir(), m_pFilePair->pSecondFile->GetName());
		if (IDYES != AfxMessageBox(s, MB_YESNO))
		{
			return;
		}
	}
	res1 = m_pFilePair->ReloadIfChanged();
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
		UpdateAllViews(NULL);
	}
}

void CFilePairDoc::OnUpdateFileEditFirst(CCmdUI* pCmdUI)
{
	FileItem * pFile = NULL;
	if (m_pFilePair != NULL)
	{
		pFile = m_pFilePair->pFirstFile;
	}
	ModifyOpenFileMenu(pCmdUI, pFile,
						IDS_OPEN_FIRST_FILE_MENU, IDS_OPEN_FIRST_FILE_MENU_DISABLED);
}

void CFilePairDoc::OnFileEditFirst()
{
	if (m_pFilePair != NULL)
	{
		OpenFileForEditing(m_pFilePair->pFirstFile);
	}
}

void CFilePairDoc::OnUpdateFileEditSecond(CCmdUI* pCmdUI)
{
	FileItem * pFile = NULL;
	if (m_pFilePair != NULL)
	{
		pFile = m_pFilePair->pSecondFile;
	}
	ModifyOpenFileMenu(pCmdUI, pFile,
						IDS_OPEN_SECOND_FILE_MENU, IDS_OPEN_SECOND_FILE_MENU_DISABLED);
}

void CFilePairDoc::OnFileEditSecond()
{
	if (m_pFilePair != NULL)
	{
		OpenFileForEditing(m_pFilePair->pSecondFile);
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
		; pSection != NULL; pSection = pSection->pNext)
	{
		if ((pSection->Attr & pSection->Whitespace)
			&& (pSection->Attr & pSection->Erased)
			&& m_bIgnoreWhitespaces)
		{
			continue;   // don't show the section
		}
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
		nPos += pSection->Length;
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
	if (NULL == m_pFilePair
		|| nLineNum >= m_pFilePair->m_LinePairs.GetSize())
	{
		buf[0] = 0;
		* pStrLen = 0;
		return buf;
	}
	LinePair * pPair = m_pFilePair->m_LinePairs[nLineNum];
	if (NULL == pPair)
	{
		return NULL;
	}
	return pPair->GetText(buf, BufChars, pStrLen, m_bIgnoreWhitespaces);
}

void CFilePairDoc::CaretLeftToWord(int SelectionFlags)
{
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
		CaretPos = 0;
		// get previous line
		if (CaretLine > 0)
		{
			CaretLine--;
			pLine = GetLineText(CaretLine, linebuf, 2048, & StrLen);
			CaretPos = StrLen;
		}
	}
	// go to previous word in the line, or to the begin of line
	// check the char before caret.
	// if it's space, skip all of them
	while (CaretPos > 0 && ' ' == pLine[CaretPos - 1])
	{
		CaretPos--;
	}

	// check previous char type, and skip all of that type
	if (CaretPos > 0)
	{
		TCHAR c = pLine[CaretPos - 1];
		if (_istalnum(c) || '_' == c)
		{
			while (CaretPos--, CaretPos > 0
					&& (_istalnum(pLine[CaretPos - 1])
						|| '_' == pLine[CaretPos - 1]));
		}
		else
		{
			while (CaretPos--, CaretPos > 0
					&& ! (' ' == pLine[CaretPos - 1]
						|| _istalnum(pLine[CaretPos - 1])
						|| '_' == pLine[CaretPos - 1]));
		}
	}

	SetCaretPosition(CaretPos, CaretLine, SelectionFlags);
}

void CFilePairDoc::CaretRightToWord(int SelectionFlags)
{
	// if the caret is on the end of the line, go to the nextline
	TCHAR linebuf[2048];
	int StrLen;
	int CaretPos = m_CaretPos.pos;
	int CaretLine = m_CaretPos.line;

	LPCTSTR pLine = NULL;

	if (CaretLine >= GetTotalLines())
	{
		SetCaretPosition(0, CaretLine, SelectionFlags);
		return;
	}
	pLine = GetLineText(CaretLine, linebuf, 2048, & StrLen);
	if (CaretPos > StrLen)
	{
		CaretPos = StrLen;
	}

	// check the char type, and skip all of that type
	TCHAR c = pLine[CaretPos];
	if (' ' == c || 0 == c)
	{
		// just skip all spaces
		while (' ' == pLine[CaretPos])
		{
			CaretPos++;
		}
		if (0 == pLine[CaretPos])
		{
			// go to the next line and skip the spaces
			CaretLine++;
			CaretPos = 0;
			pLine = GetLineText(CaretLine, linebuf, 2048, & StrLen);
			while (' ' == pLine[CaretPos])
			{
				CaretPos++;
			}
		}
	}
	else if (_istalnum(c) || '_' == c)
	{
		while (_istalnum(pLine[CaretPos])
				|| '_' == pLine[CaretPos])
		{
			CaretPos++;
		}
	}
	else
	{
		while (' ' != pLine[CaretPos]
				&& 0 != pLine[CaretPos]
				&& ! _istalnum(pLine[CaretPos])
				&& '_' != pLine[CaretPos])
		{
			CaretPos++;
		}
	}

	SetCaretPosition(CaretPos, CaretLine, SelectionFlags);
}

void CFilePairDoc::OnEditAccept()
{
	if (NULL != m_pFilePair)
	{
		int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);

		int SetFlags = FileDiffSection::FlagAccept;
		int ResetFlags = FileDiffSection::FlagDecline;
		if (flags & FileDiffSection::FlagAccept)
		{
			SetFlags = 0;
			ResetFlags = FileDiffSection::FlagAccept | FileDiffSection::FlagDecline;
		}
		int NumSections = 0;
		FileDiffSection *const * ppSection = NULL;

		m_pFilePair->ModifyAcceptDeclineFlags(
											DisplayPosToLinePos(m_SelectionAnchor), DisplayPosToLinePos(m_CaretPos),
											SetFlags, ResetFlags, & ppSection, & NumSections);
		for (int i = 0; i < NumSections; i++)
		{
			InvalidatedRange ir;
			ir.begin = ppSection[i]->m_Begin;
			ir.end = ppSection[i]->m_End;
			UpdateAllViews(NULL, InvalidateRange, & ir);
		}
		if (0 != NumSections)
		{
			SetModifiedFlag(TRUE);
		}
	}
}

void CFilePairDoc::OnUpdateEditAccept(CCmdUI* pCmdUI)
{
	int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);
	pCmdUI->Enable(0 == (flags & FileDiffSection::FlagNoDifference));
	pCmdUI->SetCheck(0 != (flags & FileDiffSection::FlagAccept));
}

void CFilePairDoc::OnEditDecline()
{
	if (NULL != m_pFilePair)
	{
		int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);
		int SetFlags = FileDiffSection::FlagDecline;
		int ResetFlags = FileDiffSection::FlagAccept;
		if (flags & FileDiffSection::FlagDecline)
		{
			SetFlags = 0;
			ResetFlags = FileDiffSection::FlagAccept | FileDiffSection::FlagDecline;
		}
		int NumSections = 0;
		FileDiffSection *const * ppSection = NULL;

		m_pFilePair->ModifyAcceptDeclineFlags(
											DisplayPosToLinePos(m_SelectionAnchor), DisplayPosToLinePos(m_CaretPos),
											SetFlags, ResetFlags, & ppSection, & NumSections);
		for (int i = 0; i < NumSections; i++)
		{
			InvalidatedRange ir;
			ir.begin = ppSection[i]->m_Begin;
			ir.end = ppSection[i]->m_End;
			UpdateAllViews(NULL, InvalidateRange, & ir);
		}
		if (0 != NumSections)
		{
			SetModifiedFlag(TRUE);
		}
	}
}

void CFilePairDoc::OnUpdateEditDecline(CCmdUI* pCmdUI)
{
	int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);
	pCmdUI->Enable(0 == (flags & FileDiffSection::FlagNoDifference));
	pCmdUI->SetCheck(0 != (flags & FileDiffSection::FlagDecline));
}

BOOL CFilePairDoc::SaveModified()
{
	if ( ! IsModified())
	{
		return TRUE;
	}
	int flags = GetAcceptDeclineFlags(TextPos(0, 0),
									TextPos(GetTotalLines(), 0));
	if (0 != (flags & (FileDiffSection::FlagDecline | FileDiffSection::FlagAccept)))
	{
		CString s;
		s.Format(IDS_QUERY_SAVE_MERGED_FILE,
				m_pFilePair->pSecondFile->GetSubdir(), m_pFilePair->pSecondFile->GetName());
		int answer = AfxMessageBox(s, MB_YESNOCANCEL);
		if (IDYES == answer)
		{
			return DoSaveMerged(FALSE);
		}
		if (IDNO == answer)
		{
			return TRUE;    // don't save merged
		}
		return FALSE;
	}
	SetModifiedFlag(FALSE);
	return TRUE;
}

BOOL CFilePairDoc::DoSaveMerged(BOOL bOpenResultFile)
{
	// check if there are unmarked differences
	int flags = GetAcceptDeclineFlags(TextPos(0, 0),
									TextPos(GetTotalLines(), 0));
	int DefaultFlags = 0;
	if (flags & FileDiffSection::FlagUndefined)
	{
		CDialog dlg(IDD_DIALOG_ACCEPT_OR_DECLINE_ALL);
		int result = dlg.DoModal();
		if (IDOK == result)
		{
			DefaultFlags = FileDiffSection::FlagAccept;
		}
		else if (IDNO == result)
		{
			DefaultFlags = FileDiffSection::FlagDecline;
		}
		else
		{
			// IDCANCEL or error
			return FALSE;
		}
	}
	CThisApp * pApp = GetApp();
	LPCTSTR FileExt = NULL;
	LPCTSTR FileName = m_pFilePair->pSecondFile->GetName();
	for (int pos = strlen(FileName); pos > 0; pos--)
	{
		if ('.' == FileName[pos - 1])
		{
			FileExt = & FileName[pos - 1];
			break;
		}
	}
	// create the filter
	CString filter;
	if (NULL != FileExt)
	{
		filter = '*';
		filter = FileExt;
		filter += _T(" Files|*");
		filter += FileExt;
	}
	CString AllFilter;
	AllFilter.LoadString(IDS_ALL_FILES);
	filter += AllFilter;

	CFileDialog dlg(FALSE,
					FileExt,
					FileName,
					OFN_HIDEREADONLY
					| OFN_EXPLORER
					| OFN_ENABLESIZING
					| OFN_NONETWORKBUTTON
					| OFN_OVERWRITEPROMPT,
					filter,
					NULL);
	dlg.m_ofn.lpstrInitialDir = pApp->m_LastSaveMergedDir;
	CString DlgTitle;
	DlgTitle.LoadString(IDS_SAVE_MERGED_DIALOG_TITLE);
	dlg.m_ofn.lpstrTitle = DlgTitle;

	if (IDOK == dlg.DoModal())
	{
		LPTSTR DirBuf = pApp->m_LastSaveMergedDir.GetBuffer(MAX_PATH + 1);
		if (DirBuf)
		{
			GetCurrentDirectory(MAX_PATH + 1, DirBuf);
			pApp->m_LastSaveMergedDir.ReleaseBuffer();
		}
		CString FileName = dlg.GetPathName();
		if (! SaveMergedFile(FileName, DefaultFlags))
		{
			AfxMessageBox(IDS_STRING_COULDNT_SAVE_MERGED);
			return FALSE;
		}
		SetModifiedFlag(FALSE);
		pApp->OpenSingleFile(FileName);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CFilePairDoc::SaveMergedFile(LPCTSTR Name, int DefaultFlags)
{
	FILE * file = fopen(Name, "wt");
	if (NULL == file)
	{
		return FALSE;
	}
	BOOL result = TRUE;
	for (int LineNum = 0, DiffSectionIdx = 0; LineNum < GetTotalLines(); LineNum++)
	{
		LinePair * pPair = m_pFilePair->m_LinePairs[LineNum];
		if (NULL == pPair)
		{
			continue;
		}

		FileDiffSection * pDiffSection = NULL;
		while (DiffSectionIdx < m_pFilePair->m_DiffSections.GetSize())
		{
			pDiffSection = m_pFilePair->m_DiffSections[DiffSectionIdx];
			if (pDiffSection->m_End > TextPos(LineNum, 0))
			{
				break;
			}
			pDiffSection = NULL;
			DiffSectionIdx++;
		}

		StringSection * pSection = pPair->pFirstSection;

		// check if the line is all removed
		if (NULL != pDiffSection
			&& pDiffSection->m_Begin <= TextPos(LineNum, 0)
			&& pDiffSection->m_End >= TextPos(LineNum + 1, 0))
		{
			// check if the block is accepted
			if ((pDiffSection->IsAccepted()
					|| ! pDiffSection->IsDeclined()
					&& (DefaultFlags & FileDiffSection::FlagAccept))
				&& (pSection->Attr & pSection->Erased))
			{
				// skip the line
				continue;
			}
			if ((pDiffSection->IsDeclined()
					|| ! pDiffSection->IsAccepted()
					&& (DefaultFlags & FileDiffSection::FlagDecline))
				&& (pSection->Attr & pSection->Inserted))
			{
				// skip the line
				continue;
			}
		}


		for (int LinePos = 0; NULL != pSection; LinePos += pSection->Length, pSection = pSection->pNext)
		{
			pDiffSection = NULL;
			while (DiffSectionIdx < m_pFilePair->m_DiffSections.GetSize())
			{
				pDiffSection = m_pFilePair->m_DiffSections[DiffSectionIdx];
				if (pDiffSection->m_End > TextPos(LineNum, LinePos))
				{
					break;
				}
				pDiffSection = NULL;
				DiffSectionIdx++;
			}

			// check if the string section is all removed
			if (NULL != pDiffSection
				&& pDiffSection->m_Begin <= TextPos(LineNum, LinePos)
				&& pDiffSection->m_End >= TextPos(LineNum, LinePos + pSection->Length))
			{
				// check if the block is accepted
				if ((pDiffSection->IsAccepted()
						|| ! pDiffSection->IsDeclined()
						&& (DefaultFlags & FileDiffSection::FlagAccept))
					&& (pSection->Attr & pSection->Erased))
				{
					// skip the string section
					continue;
				}
				if ((pDiffSection->IsDeclined()
						|| ! pDiffSection->IsAccepted()
						&& (DefaultFlags & FileDiffSection::FlagDecline))
					&& (pSection->Attr & pSection->Inserted))
				{
					// skip the string section
					continue;
				}
			}
			if (pSection->Length != 0
				&& 1 != fwrite(pSection->pBegin, pSection->Length, 1, file))
			{
				fclose(file);
				return FALSE;
			}
		}
		if (EOF == fputc('\n', file))
		{
			fclose(file);
			return FALSE;
		}
	}
	if (fflush(file))
	{
		result = FALSE;
	}
	fclose(file);
	return result;
}

void CFilePairDoc::OnFileMergeSave()
{
	DoSaveMerged(TRUE); // open it in new window
}

void CFilePairDoc::OnViewIgnoreWhitespaces()
{
	TextPos CaretLinePos = DisplayPosToLinePos(m_CaretPos);
	TextPos AnchorLinePos = DisplayPosToLinePos(m_SelectionAnchor);

	m_bIgnoreWhitespaces = ! m_bIgnoreWhitespaces;
	GetApp()->m_bIgnoreWhitespaces = m_bIgnoreWhitespaces;

	m_CaretPos = LinePosToDisplayPos(CaretLinePos);
	m_SelectionAnchor = LinePosToDisplayPos(AnchorLinePos);

	UpdateAllViews(NULL);
}

void CFilePairDoc::OnUpdateViewIgnoreWhitespaces(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bIgnoreWhitespaces);
}

TextPos CFilePairDoc::LinePosToDisplayPos(TextPos position)
{
	LinePair * pPair = GetLinePair(position.line);
	if (NULL == pPair)
	{
		return position;
	}
	return TextPos(position.line,
					pPair->LinePosToDisplayPos(position.pos, m_bIgnoreWhitespaces));
}
// recalculates offset in the line with or without whitespaces shown to offset in the raw line
TextPos CFilePairDoc::DisplayPosToLinePos(TextPos position)
{
	LinePair * pPair = GetLinePair(position.line);
	if (NULL == pPair)
	{
		return position;
	}
	return TextPos(position.line,
					pPair->DisplayPosToLinePos(position.pos, m_bIgnoreWhitespaces));
}

LinePair * CFilePairDoc::GetLinePair(int line) const
{
	if (NULL == m_pFilePair
		|| line >= m_pFilePair->m_LinePairs.GetSize())
	{
		return NULL;
	}
	return m_pFilePair->m_LinePairs[line];
}

int CFilePairDoc::GetAcceptDeclineFlags(TextPos begin, TextPos end)
{
	if (NULL == m_pFilePair)
	{
		return FileDiffSection::FlagNoDifference;
	}
	return m_pFilePair->GetAcceptDeclineFlags(
											DisplayPosToLinePos(begin), DisplayPosToLinePos(end), m_bIgnoreWhitespaces);

}

void CFilePairDoc::OnUpdateFileCopyFirstDirFile(CCmdUI* pCmdUI)
{
	FileItem * pFile = NULL;
	if (m_pFilePair != NULL)
	{
		pFile = m_pFilePair->pFirstFile;
	}
	ModifyOpenFileMenu(pCmdUI, pFile,
						IDS_COPY_FILE_TO_FOLDER, IDS_COPY_FIRST_FILE_MENU_DISABLED);
}

void CFilePairDoc::OnFileCopyFirstDirFile()
{
	if (m_pFilePair != NULL && m_pFilePair->pFirstFile != NULL)
	{
		CopyFilesToFolder( & m_pFilePair->pFirstFile, 1, false);
	}
}

void CFilePairDoc::OnUpdateFileCopySecondDirFile(CCmdUI* pCmdUI)
{
	FileItem * pFile = NULL;
	if (m_pFilePair != NULL)
	{
		pFile = m_pFilePair->pSecondFile;
	}
	ModifyOpenFileMenu(pCmdUI, pFile,
						IDS_COPY_FILE_TO_FOLDER, IDS_COPY_SECOND_FILE_MENU_DISABLED);

}

void CFilePairDoc::OnFileCopySecondDirFile()
{
	if (m_pFilePair != NULL && m_pFilePair->pSecondFile != NULL)
	{
		CopyFilesToFolder( & m_pFilePair->pSecondFile, 1, false);
	}
}
