// AlegrDiffDoc.cpp : implementation of the CAlegrDiffDoc class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "AlegrDiffDoc.h"
#include "CompareDirsDialog.h"
#include "DiffFileView.h"
#include "FindDialog.h"
#include "FilesPropertiesDialog.h"
#include <process.h>
#include <afxpriv.h>
#include "AcceptDeclineDlg.h"

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
	ON_COMMAND(ID_FILE_CANCEL, OnFileCancel)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_VIEW_REFRESH, OnUpdateViewRefresh)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc construction/destruction

CAlegrDiffDoc::CAlegrDiffDoc()
	: m_nFilePairs(0),
	m_bRecurseSubdirs(false),
	m_hThread(NULL),
	m_hEvent(CreateEvent(NULL, FALSE, FALSE, NULL)),
	m_bStopThread(TRUE),
	m_NextPairToRefresh(NULL),
	m_NextPairToCompare(NULL)
{
}

CAlegrDiffDoc::~CAlegrDiffDoc()
{
	if (NULL != m_hThread)
	{
		m_bStopThread = TRUE;
		SetEvent(m_hEvent);
		if (WAIT_TIMEOUT == WaitForSingleObject(m_hThread, 5000))
		{
			TerminateThread(m_hThread, -1);
		}
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	if (NULL != m_hEvent)
	{
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
	FreeFilePairList();
}

bool CAlegrDiffDoc::BuildFilePairList(LPCTSTR dir1, LPCTSTR dir2,
									bool bRecurseSubdirs, bool BinaryComparison)
{
	// look through all files in the directory and subdirs
	CThisApp * pApp = GetApp();
	m_sInclusionPattern = PatternToMultiCString(pApp->m_sFilenameFilter);
	if (m_sInclusionPattern.IsEmpty())
	{
		m_sInclusionPattern = "*";
	}

	m_sExclusionPattern = PatternToMultiCString(pApp->m_sIgnoreFilesFilter);

	m_sCFilesPattern = PatternToMultiCString(pApp->m_sCppFilesFilter);

	if (BinaryComparison)
	{
		m_sBinaryFilesPattern = PatternToMultiCString(_T("*"));
	}
	else
	{
		m_sBinaryFilesPattern = PatternToMultiCString(pApp->m_sBinaryFilesFilter);
	}

	m_bRecurseSubdirs = bRecurseSubdirs;
	FileList FileList1;
	FileList FileList2;

	UpdateAllViews(NULL);
	// make full names from the directories
	LPTSTR pFilePart;
	TCHAR buf[MAX_PATH];

	GetFullPathName(dir1, MAX_PATH, buf, & pFilePart);
	m_sFirstDir = buf;

	GetFullPathName(dir2, MAX_PATH, buf, & pFilePart);
	m_sSecondDir = buf;

	{
		CString title = m_sFirstDir;
		title += _T(" - ");
		title += m_sSecondDir;
		SetTitle(title);
	}
	if (! FileList1.LoadFolder(m_sFirstDir, bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern, m_sCFilesPattern,
								m_sBinaryFilesPattern))
	{
		DWORD error = GetLastError();
		FreeFilePairList();
		CString s;
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, LPCTSTR(m_sFirstDir));
		AfxMessageBox(s);
		return false;
	}
	if (! FileList2.LoadFolder(m_sSecondDir, bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern,
								m_sCFilesPattern, m_sBinaryFilesPattern))
	{
		FreeFilePairList();
		CString s;
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, LPCTSTR(m_sSecondDir));
		AfxMessageBox(s);
		return false;
	}

	if (BuildFilePairList(FileList1, FileList2))
	{
		UpdateAllViews(NULL);
	}
	((CFrameWnd*)AfxGetMainWnd())->SetMessageText(AFX_IDS_IDLEMESSAGE);
	return true;
}
bool operator ==(FILETIME time1, FILETIME time2)
{
	return time1.dwLowDateTime == time2.dwLowDateTime
			&& time1.dwHighDateTime == time2.dwHighDateTime;
}

bool CAlegrDiffDoc::BuildFilePairList(FileList & FileList1, FileList & FileList2)
{

	vector<FileItem *> Files1;
	vector<FileItem *> Files2;

	FileList1.GetSortedList(Files1, FileList::SortDirFirst | FileList::SortBackwards);
	FileList2.GetSortedList(Files2, FileList::SortDirFirst | FileList::SortBackwards);

	// we don't call FreeFilePairList, because we could be performing file list refrech
	FilePair * pInsertBefore = m_PairList.Next();
	bool NeedUpdateViews = false;

	for (unsigned idx1 = 0, idx2 = 0; idx1 < Files1.size() || idx2 < Files2.size(); )
	{
		FilePair * pPair = new FilePair;

		int comparison;
		if (idx1 >= Files1.size())
		{
			comparison = -1;
		}
		else if (idx2 >= Files2.size())
		{
			comparison = 1;
		}
		else
		{
			comparison = FileItem::DirNameCompare(Files1[idx1], Files2[idx2]);
		}

		if (comparison < 0)
		{
			pPair->pFirstFile = NULL;
			pPair->pSecondFile = Files2[idx2];
			if (pPair->pSecondFile->IsFolder())
			{
				pPair->m_ComparisionResult = FilePair::OnlySecondDirectory;
			}
			else
			{
				pPair->m_ComparisionResult = FilePair::OnlySecondFile;
			}
			idx2++;

			if (0) TRACE(_T("File \"%s\" exists only in dir \"%s\"\n"),
						LPCTSTR(pPair->pSecondFile->GetName()),
						LPCTSTR(FileList2.m_BaseDir + pPair->pSecondFile->GetSubdir()));
		}
		else if (comparison > 0)
		{
			pPair->pSecondFile = NULL;
			pPair->pFirstFile = Files1[idx1];
			if (pPair->pFirstFile->IsFolder())
			{
				pPair->m_ComparisionResult = FilePair::OnlyFirstDirectory;
			}
			else
			{
				pPair->m_ComparisionResult = FilePair::OnlyFirstFile;
			}
			idx1++;

			if (0) TRACE(_T("File \"%s\" exists only in dir \"%s\"\n"),
						LPCTSTR(pPair->pFirstFile->GetName()),
						LPCTSTR(FileList1.m_BaseDir + pPair->pFirstFile->GetSubdir()));
		}
		else
		{
			pPair->pFirstFile = Files1[idx1];
			idx1++;
			pPair->pSecondFile = Files2[idx2];
			idx2++;
			if (pPair->pFirstFile->IsFolder())
			{
				pPair->m_ComparisionResult = pPair->FilesIdentical;
			}
			else
			{
				pPair->m_ComparisionResult = pPair->ResultUnknown;
			}

			if (0) TRACE(_T("File \"%s\" exists in both \"%s\" and \"%s\"\n"),
						LPCTSTR(pPair->pFirstFile->GetName()),
						LPCTSTR(FileList1.m_BaseDir + pPair->pFirstFile->GetSubdir()),
						LPCTSTR(FileList2.m_BaseDir + pPair->pSecondFile->GetSubdir()));
		}
		while (pInsertBefore != m_PairList.Head())
		{
			// check if we insert ir remove items, or the item is duplicate
			FileItem * pItem1 = pPair->pFirstFile;
			if (NULL == pItem1)
			{
				pItem1 = pPair->pSecondFile;
			}

			FileItem * pItem2 = pInsertBefore->pFirstFile;
			if (NULL == pItem2)
			{
				pItem2 = pInsertBefore->pSecondFile;
			}
			comparison = FileItem::DirNameCompare(pItem1, pItem2);
			if (comparison < 0)
			{
				FilePair * tmp = pInsertBefore;
				pInsertBefore = pInsertBefore->Next();
				tmp->RemoveFromList();
				tmp->Dereference();
				m_nFilePairs--;
				NeedUpdateViews = true;
				continue;
			}
			else if (comparison > 0)
			{
				break;
			}
			// name is the same
			// check if file times are the same, and both files exist/not exist
			if ((NULL != pPair->pFirstFile) == (NULL != pInsertBefore->pFirstFile)
				&& (NULL != pPair->pSecondFile) == (NULL != pInsertBefore->pSecondFile)
				// also check to see if file types are the same (binary/text
				&& (pPair->NeedBinaryComparison() == pInsertBefore->NeedBinaryComparison())
				&& (pPair->pFirstFile == NULL
					|| pPair->pFirstFile->GetLastWriteTime() ==
					pInsertBefore->pFirstFile->GetLastWriteTime())
				&& (pPair->pSecondFile == NULL
					|| pPair->pSecondFile->GetLastWriteTime() ==
					pInsertBefore->pSecondFile->GetLastWriteTime()))
			{
				if (pPair->pFirstFile != NULL
					&& pPair->pFirstFile->GetLastWriteTime() ==
					pInsertBefore->pFirstFile->GetLastWriteTime())
				{
				}
				pPair->Dereference();
				pPair = NULL;
				pInsertBefore = pInsertBefore->Next();
			}
			else
			{
				FilePair * tmp = pInsertBefore;
				pInsertBefore = pInsertBefore->Next();
				tmp->RemoveFromList();
				tmp->Dereference();
				m_nFilePairs--;
			}
			break;
		}

		if (pPair != NULL)
		{
			NeedUpdateViews = true;
			pInsertBefore->InsertTail(pPair);
			m_nFilePairs++;
		}
	}

	while (pInsertBefore != m_PairList.Head())
	{
		FilePair * tmp = pInsertBefore;
		pInsertBefore = pInsertBefore->Next();
		tmp->RemoveFromList();
		tmp->Dereference();
		NeedUpdateViews = true;
		m_nFilePairs--;
	}
	// all files are referenced in FilePair list
	FileList1.Detach();
	FileList2.Detach();
	return NeedUpdateViews;
}

void CAlegrDiffDoc::FreeFilePairList()
{
	while (! m_PairList.IsEmpty())
	{
		m_PairList.RemoveHead()->Dereference();
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
	m_CopyDisabled(false),
	m_CaretPos(0, 0)
{
	m_ComparisonResult[0] = 0;
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
		m_pFilePair->UnloadFiles();
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
			SetTitle(_T(""));
		}

		if (pPair->m_LinePairs.empty())
		{
			//UpdateAllViews(NULL, 0);    // erase the views
			BOOL StopOp = FALSE;
			((CFrameWnd*)AfxGetMainWnd())->SetMessageText(_T("Loading and comparing files..."));

			pPair->m_ComparisionResult = pPair->CompareFiles(StopOp);
		}

		m_TotalLines = pPair->m_LinePairs.size();
		_tcsncpy(m_ComparisonResult, pPair->GetComparisionResult(),
				countof(m_ComparisonResult));
		m_ComparisonResult[countof(m_ComparisonResult) - 1] = 0;
		((CFrameWnd*)AfxGetMainWnd())->PostMessage(WM_SETMESSAGESTRING, 0, (LPARAM)m_ComparisonResult);
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
		m_OriginalSelectionAnchor = m_CaretPos;  // for word mode selection
		// if canceling selection, check for word mode reset
		m_WordSelectionMode = (0 != (flags & SetWordSelectionMode));
	}
	// keep selection, but check for word selection mode
	else if (flags & SetWordSelectionMode)
	{
		m_WordSelectionMode = true;
	}
	if (m_WordSelectionMode)
	{
		TextPos AnchorBegin = m_OriginalSelectionAnchor;
		TextPos AnchorEnd = m_OriginalSelectionAnchor;
		GetWordOnPos(m_OriginalSelectionAnchor, AnchorBegin, AnchorEnd);

		TextPos Begin = m_CaretPos;
		TextPos End = m_CaretPos;
		GetWordOnPos(m_CaretPos, Begin, End);

		if (m_CaretPos < m_OriginalSelectionAnchor)
		{
			m_CaretPos = Begin;
			m_SelectionAnchor = AnchorEnd;
		}
		else
		{
			m_CaretPos = End;
			m_SelectionAnchor = AnchorBegin;
		}
	}
	UpdateAllViews(NULL, CaretPositionChanged, NULL);
}

void CFilePairDoc::CaretToHome(int flags)
{
	int NewLine = m_CaretPos.line;
	int NewPos = m_CaretPos.pos;
	if ((flags & SetPositionCancelSelection)
		&& m_CaretPos > m_SelectionAnchor)
	{
		NewLine = m_SelectionAnchor.line;
		NewPos = m_SelectionAnchor.pos;
	}

	// find the first non-space position
	// if the cursor is on this position, go to pos 0, otherwise to this pos.
	if (NewLine >= m_TotalLines)
	{
		SetCaretPosition(0, NewLine, flags);
		return;
	}
	LinePair * pLine = m_pFilePair->m_LinePairs[NewLine];
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
					if (NewPos == pos)
					{
						SetToPos = 0;
					}
					SetCaretPosition(SetToPos, NewLine, flags);
					return;
				}
				i++;
			}
		}
	}
	SetCaretPosition(0, NewLine, flags);
}

void CFilePairDoc::CaretToEnd(int flags)
{
	int NewLine = m_CaretPos.line;
	if ((flags & SetPositionCancelSelection)
		&& m_CaretPos < m_SelectionAnchor)
	{
		NewLine = m_SelectionAnchor.line;
	}

	if (NewLine >= m_TotalLines)
	{
		SetCaretPosition(0, NewLine, flags);
		return;
	}
	LinePair * pLine = m_pFilePair->m_LinePairs[NewLine];
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
	SetCaretPosition(pos, NewLine, flags);
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
	ON_COMMAND(ID_FILE_PROPERTIES, OnFileProperties)
	ON_UPDATE_COMMAND_UI(ID_FILE_MERGE_SAVE, OnUpdateFileMergeSave)
	ON_COMMAND(ID_MERGE_INCLUDE, OnMergeInclude)
	ON_UPDATE_COMMAND_UI(ID_MERGE_INCLUDE, OnUpdateMergeInclude)
	ON_COMMAND(ID_MERGE_EXCLUDE, OnMergeExclude)
	ON_UPDATE_COMMAND_UI(ID_MERGE_EXCLUDE, OnUpdateMergeExclude)
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
	pCmdUI->Enable(m_pFilePair->NextDifference(m_CaretPos, m_bIgnoreWhitespaces, NULL, NULL));
}

void CFilePairDoc::OnUpdateEditGotoprevdiff(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pFilePair->PrevDifference(m_CaretPos, m_bIgnoreWhitespaces, NULL, NULL));
}

void CFilePairDoc::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_CaretPos != m_SelectionAnchor && ! m_CopyDisabled);
}

ULONG CFilePairDoc::CopyTextToMemory(LPTSTR pBuf, ULONG BufLen, TextPos pFrom, TextPos pTo)
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
	if(m_CopyDisabled)
	{
		return;
	}
	// todo: perform UNICODE or ANSI copy?
	// calculate length of the selection
	ULONG Len = CopyTextToMemory(NULL, 0, m_SelectionAnchor, m_CaretPos);
	if (0 == Len)
	{
		return;
	}
	//	allocate memory
	HGLOBAL hMem = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, sizeof (TCHAR) * (Len + 1));
	if (NULL == hMem)
	{
		return;
	}
	LPTSTR pMem = LPTSTR(GlobalLock(hMem));
	// Open and erase clipboard
	if (NULL != pMem
		&& AfxGetMainWnd()->OpenClipboard())
	{
		EmptyClipboard();
		CopyTextToMemory(pMem, Len, m_SelectionAnchor, m_CaretPos);
#ifndef _UNICODE
		GlobalUnlock(hMem);
		SetClipboardData(CF_TEXT, hMem);
#else

		if ( ! GetApp()->IsWindows9x())
		{
			// if UNICODE OS, set CF_UNICODETEXT data
			GlobalUnlock(hMem);
			SetClipboardData(CF_UNICODETEXT, hMem);
		}
		else
		{
			// if non-UNICODE OS, set CF_TEXT data
			size_t MbcsBufSize = WideCharToMultiByte(CP_ACP, 0, pMem, Len, NULL, 0, NULL, 0);
			TRACE("UNICODE chars=%d, Requered MBCS buffer size = %d\n", Len, MbcsBufSize);

			HGLOBAL hMbcsMem = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, MbcsBufSize);
			if (NULL != hMbcsMem)
			{
				LPSTR pMbcsMem = LPSTR(GlobalLock(hMbcsMem));
				if (NULL != pMbcsMem)
				{
					if (0 != WideCharToMultiByte(CP_ACP, 0, pMem, Len,
												pMbcsMem, MbcsBufSize, NULL, 0))
					{
						GlobalUnlock(hMbcsMem);
						SetClipboardData(CF_TEXT, hMbcsMem);
					}
					else
					{
						GlobalUnlock(hMbcsMem);
						GlobalFree(hMbcsMem);
					}
				}
				else
				{
					GlobalFree(hMbcsMem);
				}
			}
			GlobalUnlock(hMem);
			GlobalFree(hMem);
		}
#endif
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
	s.Format(_T("Ln %d, Col %d"), m_CaretPos.line + 1, m_CaretPos.pos + 1);
	pCmdUI->SetText(s);
}

void CAlegrDiffDoc::OnViewRefresh()
{
	if (m_sFirstDir.IsEmpty())
	{
		return;
	}

	// rescan the directories again
	FileList FileList1;
	FileList FileList2;

	if (! FileList1.LoadFolder(m_sFirstDir, m_bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern, m_sCFilesPattern,
								m_sBinaryFilesPattern))
	{
		DWORD error = GetLastError();
		FreeFilePairList();
		CString s;
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, LPCTSTR(m_sFirstDir));
		AfxMessageBox(s);
		return;
	}
	if (! FileList2.LoadFolder(m_sSecondDir, m_bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern,
								m_sCFilesPattern, m_sBinaryFilesPattern))
	{
		FreeFilePairList();
		CString s;
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, LPCTSTR(m_sSecondDir));
		AfxMessageBox(s);
		return;
	}
	BuildFilePairList(FileList1, FileList2);
	RunComparisionThread();
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
	if (0 != (flags &
			(StringSection::Declined
				| StringSection::Accepted
				| StringSection::Included
				| StringSection::Discarded)))
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
	if (FilesUnchanged == res1)
	{
		return;
	}
	if (FilesDeleted == res1)
	{
		// close this document
		OnCloseDocument();
		return;
	}
	TRACE("Reloading the files\n");
	TextPos caretpos = m_CaretPos;

	m_pFilePair->LoadFiles();   // make one more reference
	m_pFilePair->Reference();

	SetFilePair(m_pFilePair);

	m_pFilePair->UnloadFiles(); // decrement extra reference
	m_pFilePair->Dereference();

	SetCaretPosition(caretpos.pos, caretpos.line, SetPositionCancelSelection);
	UpdateAllViews(NULL);
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
		|| m_pFilePair->m_LinePairs.empty())
	{
		return false;
	}
	TCHAR line[2048];
	TextPos LineSelectionAnchor = m_SelectionAnchor; //DisplayPosToLinePos(m_SelectionAnchor);
	TextPos LineCaretPos = m_CaretPos; //DisplayPosToLinePos(m_CaretPos);

	int nSearchPos;
	unsigned nSearchLine;
	if (bBackward)
	{
		if (LineCaretPos > LineSelectionAnchor)
		{
			nSearchPos = LineCaretPos.pos - 1;
			nSearchLine = LineCaretPos.line;
		}
		else if (LineCaretPos < LineSelectionAnchor)
		{
			nSearchPos = LineSelectionAnchor.pos - 1;
			nSearchLine = LineSelectionAnchor.line;
		}
		else
		{
			nSearchPos = LineCaretPos.pos;
			nSearchLine = LineCaretPos.line;
		}
		if (nSearchLine >= m_pFilePair->m_LinePairs.size())
		{
			nSearchLine--;
			nSearchPos = INT_MAX;
		}
	}
	else
	{
		if (LineCaretPos < LineSelectionAnchor)
		{
			nSearchPos = LineCaretPos.pos + 1;
			nSearchLine = LineCaretPos.line;
		}
		else if (LineCaretPos > LineSelectionAnchor)
		{
			nSearchPos = LineSelectionAnchor.pos + 1;
			nSearchLine = LineSelectionAnchor.line;
		}
		else
		{
			nSearchPos = LineCaretPos.pos;
			nSearchLine = LineCaretPos.line;
		}
		if (nSearchLine >= m_pFilePair->m_LinePairs.size())
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
		pStr = GetLineText(nSearchLine, line, countof(line), & StrLen);

		int nPatternLen = _tcslen(pStrToFind);
		if ( ! bBackward)
		{
			for ( ;nSearchPos <= StrLen - nPatternLen; nSearchPos++)
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
			if (nSearchLine >= m_pFilePair->m_LinePairs.size())
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
				nSearchLine = m_pFilePair->m_LinePairs.size();
			}
			nSearchLine--;
		}
	}
	// end of file
	return false;
}

bool CFilePairDoc::GetWordOnPos(TextPos OnPos, TextPos &Start, TextPos &End)
{
	if (OnPos.line >= (int)m_pFilePair->m_LinePairs.size())
	{
		return false;
	}
	LinePair * pPair = m_pFilePair->m_LinePairs[OnPos.line];
	if (NULL == pPair)
	{
		return false;
	}

	Start.line = OnPos.line;
	End.line = OnPos.line;

	int nPos = 0;
	int CaretPos = OnPos.pos;
	for (StringSection * pSection = pPair->pFirstSection
		; pSection != NULL; pSection = pSection->pNext)
	{
		if ((pSection->Attr & pSection->Whitespace)
			&& (pSection->Attr & pSection->Erased)
			&& m_bIgnoreWhitespaces)
		{
			continue;   // don't show the section
		}
		// if position is on the endo of line or on space, and the previous char is alpha, get the word to the left
		if (CaretPos < nPos + pSection->Length
			|| (NULL == pSection->pNext && CaretPos == nPos + pSection->Length))
		{
			// get a word under the position and to the right, or take a single non-alpha char
			TCHAR c;
			if (CaretPos < nPos + pSection->Length)
			{
				c = pSection->pBegin[CaretPos - nPos];
			}
			else
			{
				c = ' ';
			}
			if (' ' == c)
			{
				// if position is on the endo of line or on space, and the previous char is alpha, get the word to the left
				if (CaretPos > nPos)
				{
					c = pSection->pBegin[CaretPos - nPos - 1];
					if (_istalnum(TCHAR_MASK & c) || '_' == c)
					{
						CaretPos --;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			if (_istalnum(TCHAR_MASK & c) || '_' == c)
			{
				for (Start.pos = CaretPos; Start.pos > nPos; Start.pos--)
				{
					c = pSection->pBegin[Start.pos - nPos - 1];
					if (! _istalnum(TCHAR_MASK & c) && '_' != c)
					{
						break;
					}
				}
				for (End.pos = CaretPos + 1; End.pos < nPos + pSection->Length; End.pos++)
				{
					c = pSection->pBegin[End.pos - nPos];
					if (! _istalnum(TCHAR_MASK & c) && '_' != c)
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
	return OnFind(true, false, true);
}

bool CFilePairDoc::OnEditFindNext()
{
	return OnFind(false, false, false);
}

bool CFilePairDoc::OnEditFindPrev()
{
	return OnFind(false, true, false);
}

bool CFilePairDoc::OnEditFindWordNext()
{
	return OnFind(true, false, false);
}

bool CFilePairDoc::OnEditFindWordPrev()
{
	return OnFind(true, true, false);
}

bool CFilePairDoc::OnFind(bool PickWordOrSelection, bool bBackwards, bool bInvokeDialog)
{
	CThisApp * pApp = GetApp();
	int nBeginOffset;
	int nLength = 0;
	if (PickWordOrSelection)
	{
		pApp->m_FindString.Empty();

		if (m_CaretPos == m_SelectionAnchor
			|| m_CaretPos.line != m_SelectionAnchor.line)
		{
			TextPos Begin, End;
			if (GetWordOnPos(m_CaretPos, Begin, End))
			{
				nBeginOffset = Begin.pos;
				nLength = End.pos - Begin.pos;
			}
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
		if (nLength)
		{
			TCHAR line[2048];
			int StrLen;
			LPCTSTR pStr = GetLineText(m_CaretPos.line, line, countof(line), & StrLen);
			if (NULL != pStr
				&& nBeginOffset < StrLen)
			{
				if (nBeginOffset + nLength > StrLen)
				{
					nLength = StrLen - nBeginOffset;
				}
				LPTSTR StrBuf = pApp->m_FindString.GetBuffer(nLength);
				if (NULL != StrBuf)
				{
					_tcsncpy(StrBuf, pStr + nBeginOffset, nLength);
					pApp->m_FindString.ReleaseBuffer(nLength);
				}
			}
		}
	}

	if (bInvokeDialog || pApp->m_FindString.IsEmpty())
	{
		CMyFindDialog dlg;
		dlg.m_sFindCombo = pApp->m_FindString;
		dlg.m_bCaseSensitive = pApp->m_bCaseSensitive;
		dlg.m_FindDown =  ! pApp->m_bFindBackward;

		if (IDOK != dlg.DoModal())
		{
			return FALSE;
		}
		pApp->m_FindString = dlg.m_sFindCombo;
		pApp->m_bCaseSensitive = ( 0 != dlg.m_bCaseSensitive);
		pApp->m_bFindBackward = ! dlg.m_FindDown;
		bBackwards = pApp->m_bFindBackward;
	}
	// update MRU, case sensitive
	AddStringToHistory(pApp->m_FindString, pApp->m_sFindHistory,
						countof(pApp->m_sFindHistory), true);

	return FindTextString(pApp->m_FindString, bBackwards, pApp->m_bCaseSensitive);
}

// returns a pointer to a line text
// buf is used to assembly the string if it is fragmented
LPCTSTR CFilePairDoc::GetLineText(int nLineNum, LPTSTR buf, size_t BufChars, int *pStrLen)
{
	if (NULL == m_pFilePair
		|| nLineNum >= (int)m_pFilePair->m_LinePairs.size())
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
	if ((SelectionFlags & SetPositionCancelSelection)
		&& m_CaretPos > m_SelectionAnchor)
	{
		CaretLine = m_SelectionAnchor.line;
		CaretPos = m_SelectionAnchor.pos;
	}

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
		if (_istalnum(TCHAR_MASK & c) || '_' == c)
		{
			while (CaretPos--, CaretPos > 0
					&& (_istalnum(TCHAR_MASK & pLine[CaretPos - 1])
						|| '_' == pLine[CaretPos - 1]));
		}
		else
		{
			while (CaretPos--, CaretPos > 0
					&& ! (' ' == pLine[CaretPos - 1]
						|| _istalnum(TCHAR_MASK & pLine[CaretPos - 1])
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
	if ((SelectionFlags & SetPositionCancelSelection)
		&& m_CaretPos < m_SelectionAnchor)
	{
		CaretLine = m_SelectionAnchor.line;
		CaretPos = m_SelectionAnchor.pos;
	}

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
	else
	{
		if (_istalnum(TCHAR_MASK & c) || '_' == c)
		{
			while (_istalnum(TCHAR_MASK & pLine[CaretPos])
					|| '_' == pLine[CaretPos])
			{
				CaretPos++;
			}
		}
		else
		{
			while (' ' != pLine[CaretPos]
					&& 0 != pLine[CaretPos]
					&& ! _istalnum(TCHAR_MASK & pLine[CaretPos])
					&& '_' != pLine[CaretPos])
			{
				CaretPos++;
			}
		}
		// skip all spaces
		while (' ' == pLine[CaretPos])
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

		int SetFlags = StringSection::Accepted;
		int ResetFlags = StringSection::Declined
						| StringSection::Undefined
						| StringSection::Included
						| StringSection::Discarded;
		if (flags & StringSection::Accepted)
		{
			SetFlags = StringSection::Undefined;
			ResetFlags = StringSection::Accepted
						| StringSection::Declined
						| StringSection::Included
						| StringSection::Discarded;
		}

		TextPos begin = DisplayPosToLinePos(m_SelectionAnchor);
		TextPos end = DisplayPosToLinePos(m_CaretPos);
		if (m_pFilePair->ModifyAcceptDeclineFlags(begin, end, SetFlags, ResetFlags))
		{
			InvalidatedRange ir;
			ir.begin = LinePosToDisplayPos(begin);
			ir.end = LinePosToDisplayPos(end);
			UpdateAllViews(NULL, InvalidateRange, & ir);
			SetModifiedFlag(TRUE);
		}
		if (GetApp()->m_bCancelSelectionOnMerge)
		{
			SetSelection(m_CaretPos, m_CaretPos, 0);
		}
	}
}

void CFilePairDoc::OnUpdateEditAccept(CCmdUI* pCmdUI)
{
	int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);
	pCmdUI->Enable(0 == (flags & StringSection::NoDifference));
	pCmdUI->SetCheck(0 != (flags & StringSection::Accepted));
}

void CFilePairDoc::OnEditDecline()
{
	if (NULL != m_pFilePair)
	{
		int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);
		int SetFlags = StringSection::Declined;
		int ResetFlags = StringSection::Accepted
						| StringSection::Undefined
						| StringSection::Included
						| StringSection::Discarded;
		if (flags & StringSection::Declined)
		{
			SetFlags = StringSection::Undefined;
			ResetFlags = StringSection::Accepted
						| StringSection::Declined
						| StringSection::Included
						| StringSection::Discarded;
		}
		TextPos begin = DisplayPosToLinePos(m_SelectionAnchor);
		TextPos end = DisplayPosToLinePos(m_CaretPos);
		if (m_pFilePair->ModifyAcceptDeclineFlags(begin, end, SetFlags, ResetFlags))
		{
			InvalidatedRange ir;
			ir.begin = LinePosToDisplayPos(begin);
			ir.end = LinePosToDisplayPos(end);
			UpdateAllViews(NULL, InvalidateRange, & ir);
			SetModifiedFlag(TRUE);
		}
		if (GetApp()->m_bCancelSelectionOnMerge)
		{
			SetSelection(m_CaretPos, m_CaretPos, 0);
		}
	}
}

void CFilePairDoc::OnUpdateEditDecline(CCmdUI* pCmdUI)
{
	int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);
	pCmdUI->Enable(0 == (flags & StringSection::NoDifference));
	pCmdUI->SetCheck(0 != (flags & StringSection::Declined));
}

BOOL CFilePairDoc::SaveModified()
{
	if ( ! IsModified())
	{
		return TRUE;
	}
	int flags = m_pFilePair->GetAcceptDeclineFlags(TextPos(0, 0),
													TextPos(GetTotalLines(), 0), false);
	if (0 != (flags & (StringSection::Declined | StringSection::Accepted
				| StringSection::Included | StringSection::Discarded)))
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

class CMergedFilesSaveDlg : public CFileDialog
{
public:
	CMergedFilesSaveDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
						LPCTSTR lpszDefExt = NULL,
						LPCTSTR lpszFileName = NULL,
						DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						LPCTSTR lpszFilter = NULL,
						CWnd* pParentWnd = NULL,
						DWORD dwSize = sizeof(OPENFILENAME))
		: CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName,
					dwFlags, lpszFilter, pParentWnd, dwSize),
		m_bUnicode(FALSE)
	{
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_SAVE_MERGED_TEMPLATE);
		m_ofn.Flags |= OFN_ENABLETEMPLATE;
	}
	BOOL m_bUnicode;
	virtual void OnInitDone( )
	{
		CheckDlgButton(IDC_CHECK_UNICODE, m_bUnicode);
	}

	void OnCheckUnicode()
	{
		m_bUnicode = IsDlgButtonChecked(IDC_CHECK_UNICODE);
	}
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CMergedFilesSaveDlg, CFileDialog)
	//{{AFX_MSG_MAP(CMergedFilesSaveDlg)
	ON_BN_CLICKED(IDC_CHECK_UNICODE, OnCheckUnicode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CFilePairDoc::DoSaveMerged(BOOL bOpenResultFile)
{
	// check if there are unmarked differences
	int flags = GetAcceptDeclineFlags(TextPos(0, 0),
									TextPos(GetTotalLines(), 0));
	int DefaultFlags = 0;
	CThisApp * pApp = GetApp();
	if (flags & StringSection::Undefined)
	{
		CAcceptDeclineDlg dlg;
		dlg.m_File1 = m_pFilePair->pFirstFile->GetFullName();
		dlg.m_File2 = m_pFilePair->pSecondFile->GetFullName();

		int result = dlg.DoModal();
		if (IDOK == result)
		{
			DefaultFlags = StringSection::Accepted;
		}
		else if (IDNO == result)
		{
			DefaultFlags = StringSection::Declined;
		}
		else
		{
			// IDCANCEL or error
			return FALSE;
		}
	}
	CString FileExt;
	CString FileName;
	BOOL bUnicode;
	if (DefaultFlags == StringSection::Declined)
	{
		FileName = m_pFilePair->pFirstFile->GetName();
		bUnicode = m_pFilePair->pFirstFile->m_IsUnicode;
	}
	else
	{
		FileName = m_pFilePair->pSecondFile->GetName();
		bUnicode = m_pFilePair->pSecondFile->m_IsUnicode;
	}

	for (int pos = FileName.GetLength(); pos > 0; pos--)
	{
		if ('.' == FileName[pos - 1])
		{
			FileExt = LPCTSTR(FileName) + pos - 1;
			break;
		}
	}
	// create the filter
	CString filter;
	if ( ! FileExt.IsEmpty())
	{
		filter += CreateCustomFilter(FileExt);
	}
	CString AllFilter;
	AllFilter.LoadString(IDS_ALL_FILES);
	filter += AllFilter;

	CMergedFilesSaveDlg dlg(FALSE,
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
	dlg.m_bUnicode = bUnicode;

	if (IDOK == dlg.DoModal())
	{
		LPTSTR DirBuf = pApp->m_LastSaveMergedDir.GetBuffer(MAX_PATH + 1);
		if (DirBuf)
		{
			GetCurrentDirectory(MAX_PATH + 1, DirBuf);
			pApp->m_LastSaveMergedDir.ReleaseBuffer();
		}
		CString FileName = dlg.GetPathName();
		if (! SaveMergedFile(FileName, DefaultFlags, dlg.m_bUnicode))
		{
			AfxMessageBox(IDS_STRING_COULDNT_SAVE_MERGED);
			return FALSE;
		}
		SetModifiedFlag(FALSE);
#ifndef DEMO_VERSION
		pApp->OpenSingleFile(FileName);
#else
		AfxMessageBox("DEMO version doesn't save the merged file. You can only view it\n", MB_OK);
#endif
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CFilePairDoc::SaveMergedFile(LPCTSTR Name, int DefaultFlags, BOOL bUnicode)
{
	// save ANSI or UNICODE
#ifndef DEMO_VERSION
	LPCTSTR FileMode = _T("wt");
#ifdef _UNICODE
	if (bUnicode)
	{
		FileMode = _T("wb");
	}
#endif
	FILE * file = _tfopen(Name, FileMode);
	if (NULL == file)
	{
		return FALSE;
	}
#else
	FilePair * pNewFilePair = new FilePair;
	pNewFilePair->pFirstFile = new FileItem(Name);
	pNewFilePair->SetMemoryFile();
#endif

	if (bUnicode)
	{
		fputwc(0xFEFF, file);
	}
	BOOL result = TRUE;
	for (int LineNum = 0; LineNum < GetTotalLines(); LineNum++)
	{
		TCHAR LineBuf[2048 + 3];
		int nUsedChars = 0;

		LinePair * pPair = m_pFilePair->m_LinePairs[LineNum];
		if (NULL == pPair)
		{
			continue;
		}

		StringSection * pSection = pPair->pFirstSection;

		if (NULL == pPair->pFirstLine && NULL != pSection)
		{
			if (pSection->IsDeclined()
				|| pSection->IsDiscarded()
				|| ( ! pSection->IsAccepted() && ! pSection->IsIncluded()
					&& (DefaultFlags & StringSection::Declined)))
			{
				// skip the line completely
				continue;
			}
		}
		else if (NULL == pPair->pSecondLine && NULL != pSection)
		{
			if (pSection->IsAccepted()
				|| pSection->IsDiscarded()
				|| ( ! pSection->IsDeclined() && ! pSection->IsIncluded()
					&& (DefaultFlags & StringSection::Accepted)))
			{
				// skip the line completely
				continue;
			}
		}
		for ( ; NULL != pSection; pSection = pSection->pNext)
		{

			// check if the string section is all removed
			// check if the block is accepted
			if ((pSection->Attr & pSection->Erased)
				&& (pSection->IsAccepted()
					|| pSection->IsDiscarded()
					|| ( ! pSection->IsDeclined() && ! pSection->IsIncluded()
						&& (DefaultFlags & StringSection::Accepted))))
			{
				continue;
			}
			else if ((pSection->Attr & pSection->Inserted)
					&& (pSection->IsDeclined()
						|| pSection->IsDiscarded()
						|| ( ! pSection->IsAccepted() && ! pSection->IsIncluded()
							&& (DefaultFlags & StringSection::Declined))))
			{
				continue;
			}

			if (2048 - nUsedChars >= pSection->Length)
			{
				memcpy(LineBuf + nUsedChars, pSection->pBegin,
						pSection->Length * sizeof (TCHAR));
				nUsedChars += pSection->Length;
			}
			else
			{
				break;
			}
		}
#ifndef DEMO_VERSION
#ifdef _UNICODE
		if (bUnicode)
		{
			LineBuf[nUsedChars++] = '\r';
		}
#endif
		LineBuf[nUsedChars ++] = '\n';
		LineBuf[nUsedChars ++] = 0;
		if (EOF == _fputts(LineBuf, file))
		{
			fclose(file);
			return FALSE;
		}
#else
		pNewFilePair->pFirstFile->AddLine(LineBuf);
#endif
	}
#ifndef DEMO_VERSION
	if (fflush(file))
	{
		result = FALSE;
	}
	fclose(file);
#else
	CFilePairDoc * pDoc = dynamic_cast<CFilePairDoc *>
						(GetApp()->OpenFilePairView(pNewFilePair));
	if (NULL != pDoc)
	{
		pDoc->m_CopyDisabled = true;
	}
	// remove extra reference and forget about it
	pNewFilePair->Dereference();
#endif
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
		|| line >= (int)m_pFilePair->m_LinePairs.size())
	{
		return NULL;
	}
	return m_pFilePair->m_LinePairs[line];
}

int CFilePairDoc::GetAcceptDeclineFlags(TextPos begin, TextPos end)
{
	if (NULL == m_pFilePair)
	{
		return StringSection::NoDifference;
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

void CAlegrDiffDoc::RunComparisionThread()
{
	unsigned threadid;
	ResetEvent(m_hEvent);
	m_bStopThread = false;
	m_NextPairToRefresh = m_PairList.Head();
	m_NextPairToCompare = m_PairList.Head();

	m_hThread = (HANDLE) _beginthreadex(NULL, 0x10000,
										_CompareThreadFunction, this, 0, & threadid);

}

unsigned _stdcall CAlegrDiffDoc::_CompareThreadFunction(PVOID arg)
{
	CAlegrDiffDoc * pDoc = static_cast<CAlegrDiffDoc *>(arg);

	return pDoc->CompareThreadFunction();
}

unsigned CAlegrDiffDoc::CompareThreadFunction()
{
	// preload first of binary files in pair (calculate MD5 digest)
	CMd5HashCalculator HashCalc;
	FilePair * pPair;
	LONGLONG FileComplete;
	HWND NotifyWnd = GetApp()->m_pMainWnd->m_hWnd;

	{
		CSimpleCriticalSectionLock lock(m_FileListCs);
		m_NextPairToRefresh = m_PairList.Next();
		m_NextPairToCompare = m_PairList.Next();
	}
	for (pPair = m_PairList.Next(); pPair != m_PairList.Head() && ! m_bStopThread; pPair = pPair->Next())
	{
		TRACE("First pass, pPair=%p, result=%d\n", pPair, pPair->m_ComparisionResult);
		if (pPair->m_ComparisionResult != FilePair::ResultUnknown
			|| NULL == pPair->pFirstFile
			|| NULL == pPair->pSecondFile
			|| ! (pPair->pFirstFile->m_IsBinary
				|| pPair->pSecondFile->m_IsBinary)
			|| pPair->pFirstFile->GetFileLength() != pPair->pSecondFile->GetFileLength())
		{
			m_NextPairToCompare = pPair->Next();
			continue;
		}
		pPair->m_ComparisionResult = FilePair::CalculatingFirstFingerprint;
		pPair->m_bChanged = true;
		::PostMessage(NotifyWnd, WM_KICKIDLE, 0, 0);
		if (pPair->pFirstFile->CalculateHashes( & HashCalc, m_bStopThread,
												FileComplete, NotifyWnd)
			|| m_bStopThread)
		{
			pPair->m_ComparisionResult = FilePair::ResultUnknown;
		}
		else
		{
			pPair->m_ComparisionResult = FilePair::ErrorReadingFirstFile;
		}

		pPair->m_bChanged = true;
		m_NextPairToCompare = pPair->Next();

		::PostMessage(NotifyWnd, WM_KICKIDLE, 0, 0);
	}

	{
		CSimpleCriticalSectionLock lock(m_FileListCs);
		m_NextPairToRefresh = m_PairList.Next();
		m_NextPairToCompare = m_PairList.Next();
	}

	for (pPair = m_PairList.Next(); pPair != m_PairList.Head() && ! m_bStopThread; pPair = pPair->Next())
	{
		TRACE("Second pass, pPair=%p, result=%d\n", pPair, pPair->m_ComparisionResult);
		if (FilePair::ResultUnknown != pPair->m_ComparisionResult)
		{
			m_NextPairToCompare = pPair->Next();
			continue;
		}

		pPair->m_ComparisionResult = FilePair::ComparingFiles;
		::PostMessage(NotifyWnd, WM_KICKIDLE, 0, 0);
		pPair->m_ComparisionResult = pPair->PreCompareFiles( & HashCalc, m_bStopThread);
		pPair->m_bChanged = true;
		m_NextPairToCompare = pPair->Next();

	}

	{
		CSimpleCriticalSectionLock lock(m_FileListCs);
		m_NextPairToRefresh = m_PairList.Head();
		m_NextPairToCompare = m_PairList.Head();
	}
	m_bStopThread = true;
	::PostMessage(NotifyWnd, WM_KICKIDLE, 0, 0);
	return 0;
}

void CAlegrDiffDoc::OnIdle()
{
	FilePair * pPair;
	{
		CSimpleCriticalSectionLock lock(m_FileListCs);
		while (m_PairList.Head() != m_NextPairToRefresh
				&& m_NextPairToRefresh != m_NextPairToCompare)
		{
			TRACE("Foreground view refresh, pPair=%p, result=%d\n", m_NextPairToRefresh, m_NextPairToRefresh->m_ComparisionResult);
			if (m_NextPairToRefresh->m_bChanged)
			{
				m_NextPairToRefresh->m_bChanged = false;
				AddListViewItemStruct alvi;
				alvi.pPair = m_NextPairToRefresh;
				UpdateAllViews(NULL, OnUpdateListViewItem, & alvi);
			}
			m_NextPairToRefresh = m_NextPairToRefresh->Next();
		}
		pPair = m_NextPairToCompare;
	}

	if (m_PairList.Head() != pPair)
	{
		((CFrameWnd*)AfxGetMainWnd())->
			SetMessageText(pPair->GetComparisionResult());
	}

	if (m_bStopThread
		&& NULL != m_hThread)
	{
		m_bStopThread = true;
		WaitForSingleObject(m_hThread, 5000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
		UpdateAllViews(NULL);
		((CFrameWnd*)AfxGetMainWnd())->SetMessageText(AFX_IDS_IDLEMESSAGE);
	}
}

void CAlegrDiffDoc::OnFileCancel()
{
	m_bStopThread = true;

}

void CFilePairDoc::OnFileProperties()
{
	if (NULL != m_pFilePair)
	{
		CFilesPropertiesDialog dlg(m_pFilePair);
		dlg.DoModal();
	}
}

void CFilePairDoc::OnUpdateFileMergeSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pFilePair != NULL
					&& m_pFilePair->pFirstFile != NULL
					&& m_pFilePair->pSecondFile != NULL);
}


void CFilePairDoc::OnMergeInclude()
{
	if (NULL != m_pFilePair)
	{
		int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);

		int SetFlags = StringSection::Included;
		int ResetFlags = StringSection::Declined
						| StringSection::Undefined
						| StringSection::Accepted
						| StringSection::Discarded;
		if (flags & StringSection::Included)
		{
			SetFlags = StringSection::Undefined;
			ResetFlags = StringSection::Accepted
						| StringSection::Declined
						| StringSection::Included
						| StringSection::Discarded;
		}

		TextPos begin = DisplayPosToLinePos(m_SelectionAnchor);
		TextPos end = DisplayPosToLinePos(m_CaretPos);
		if (m_pFilePair->ModifyAcceptDeclineFlags(begin, end, SetFlags, ResetFlags))
		{
			InvalidatedRange ir;
			ir.begin = LinePosToDisplayPos(begin);
			ir.end = LinePosToDisplayPos(end);
			UpdateAllViews(NULL, InvalidateRange, & ir);
			SetModifiedFlag(TRUE);
		}
		if (GetApp()->m_bCancelSelectionOnMerge)
		{
			SetSelection(m_CaretPos, m_CaretPos, 0);
		}
	}
}

void CFilePairDoc::OnUpdateMergeInclude(CCmdUI* pCmdUI)
{
	int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);
	pCmdUI->Enable(0 == (flags & StringSection::NoDifference));
	pCmdUI->SetCheck(0 != (flags & StringSection::Included));
}

void CFilePairDoc::OnMergeExclude()
{
	if (NULL != m_pFilePair)
	{
		int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);

		int SetFlags = StringSection::Discarded;
		int ResetFlags = StringSection::Declined
						| StringSection::Undefined
						| StringSection::Accepted
						| StringSection::Included;
		if (flags & StringSection::Discarded)
		{
			SetFlags = StringSection::Undefined;
			ResetFlags = StringSection::Accepted
						| StringSection::Declined
						| StringSection::Included
						| StringSection::Discarded;
		}

		TextPos begin = DisplayPosToLinePos(m_SelectionAnchor);
		TextPos end = DisplayPosToLinePos(m_CaretPos);
		if (m_pFilePair->ModifyAcceptDeclineFlags(begin, end, SetFlags, ResetFlags))
		{
			InvalidatedRange ir;
			ir.begin = LinePosToDisplayPos(begin);
			ir.end = LinePosToDisplayPos(end);
			UpdateAllViews(NULL, InvalidateRange, & ir);
			SetModifiedFlag(TRUE);
		}
		if (GetApp()->m_bCancelSelectionOnMerge)
		{
			SetSelection(m_CaretPos, m_CaretPos, 0);
		}
	}
}

void CFilePairDoc::OnUpdateMergeExclude(CCmdUI* pCmdUI)
{
	int flags = GetAcceptDeclineFlags(m_SelectionAnchor, m_CaretPos);
	pCmdUI->Enable(0 == (flags & StringSection::NoDifference));
	pCmdUI->SetCheck(0 != (flags & StringSection::Discarded));
}

void CAlegrDiffDoc::OnUpdateViewRefresh(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( ! m_sFirstDir.IsEmpty());
}
