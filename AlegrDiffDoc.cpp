// AlegrDiffDoc.cpp : implementation of the CAlegrDiffDoc class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "AlegrDiffDoc.h"
#include "CompareDirsDialog.h"
#include "DiffFileView.h"
#include "FilesPropertiesDialog.h"
#include <process.h>
#include <afxpriv.h>
#include "AcceptDeclineDlg.h"
#include "FileDialogWithHistory.h"
#include "ComparisonProgressDlg.h"
#include "FileLine.h"
#include "MessageBoxSynch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc

IMPLEMENT_DYNCREATE(CAlegrDiffBaseDoc, CDocument)
IMPLEMENT_DYNCREATE(CAlegrDiffDoc, CAlegrDiffBaseDoc)

BEGIN_MESSAGE_MAP(CAlegrDiffDoc, CAlegrDiffBaseDoc)
	//{{AFX_MSG_MAP(CAlegrDiffDoc)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_VIEW_REFRESH, OnUpdateViewRefresh)
END_MESSAGE_MAP()

void CAlegrDiffBaseDoc::OnUpdateAllViews(CView* pSender,
										LPARAM lHint, CObject* pHint)
{
	UpdateAllViews(pSender, lHint, pHint);
}
/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc construction/destruction

CAlegrDiffDoc::CAlegrDiffDoc()
	: m_nFilePairs(0),
	m_bRecurseSubdirs(false),
	m_bCheckingFingerprint(false)
	, m_bNeedUpdateViews(false)
{
}

CAlegrDiffDoc::~CAlegrDiffDoc()
{
	FreeFilePairList();
}

void CAlegrDiffDoc::SetFingerprintCheckingMode(LPCTSTR DirectoryToCheck,
												LPCTSTR FingerprintFilename)
{
	m_bCheckingFingerprint = true;

	m_sSecondDir = DirectoryToCheck;

	CString title(FingerprintFilename);
	title += _T(" - ");
	title += DirectoryToCheck;

	SetTitle(title);
}

bool CAlegrDiffDoc::RunDirectoriesComparison(LPCTSTR dir1, LPCTSTR dir2,
											LPCTSTR FilenameFilter,
											LPCTSTR IgnoreFolders,
											bool bRecurseSubdirs, bool BinaryComparison)
{
	// look through all files in the directory and subdirs
	CThisApp * pApp = GetApp();
	m_sInclusionPattern = PatternToMultiCString(FilenameFilter);

	if (m_sInclusionPattern.IsEmpty())
	{
		m_sInclusionPattern = "*";
	}

	m_sExclusionPattern = PatternToMultiCString(pApp->m_sIgnoreFilesFilter);
	m_sIgnoreDirsPattern = PatternToMultiCString(IgnoreFolders);

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

	UpdateAllViews(NULL, OnUpdateRebuildListView);
	// make full names from the directories
	LPTSTR pFilePart;
	TCHAR buf[MAX_PATH];

	GetFullPathName(dir1, MAX_PATH, buf, & pFilePart);
	m_sFirstDir = buf;

	GetFullPathName(dir2, MAX_PATH, buf, & pFilePart);
	m_sSecondDir = buf;

	SetTitle(m_sFirstDir + _T(" - ") + m_sSecondDir);

	CComparisonProgressDlg dlg(this);

	int result = dlg.DoModal();
	if (IDOK == result)
	{
		bool HasFiles = false;

		for (FilePair * pPair = m_PairList.First();
			m_PairList.NotEnd(pPair); pPair = pPair->Next())
		{
			if (pPair->FilesIdentical != pPair->GetComparisonResult()
				|| ! pPair->pFirstFile->IsFolder())
			{
				HasFiles = true;
				break;
			}
		}

		if ( ! HasFiles)
		{
			AfxMessageBox(IDS_STRING_NO_FILES_TO_COMPARE, MB_OK);
			return false;
		}

		UpdateAllViews(NULL);
		return true;
	}

	return false;
}

bool CAlegrDiffDoc::CanCancelComparison(CProgressDialog * /*pDlg*/)
{
	if (IDYES == AfxMessageBox(IDS_CAN_CANCEL_COMPARISON_PROMPT, MB_YESNO))
	{
		return true;
	}
	return FALSE;
}

bool operator ==(const FILETIME & time1, const FILETIME & time2)
{
	return time1.dwLowDateTime == time2.dwLowDateTime
			&& time1.dwHighDateTime == time2.dwHighDateTime;
}

bool operator !=(const FILETIME & time1, const FILETIME & time2)
{
	return time1.dwLowDateTime != time2.dwLowDateTime
			|| time1.dwHighDateTime != time2.dwHighDateTime;
}

bool CAlegrDiffDoc::RebuildFilePairList(CProgressDialog * pDlg)
{
	FileList FileList1;
	FileList FileList2;

	if (! FileList1.LoadFolder(m_sFirstDir, m_bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern, m_sCFilesPattern,
								m_sBinaryFilesPattern, m_sIgnoreDirsPattern))
	{
		FreeFilePairList();
		CString s;
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, LPCTSTR(m_sFirstDir));
		MessageBoxSync(s);
		return false;
	}

	if (! FileList2.LoadFolder(m_sSecondDir, m_bRecurseSubdirs,
								m_sInclusionPattern, m_sExclusionPattern,
								m_sCFilesPattern, m_sBinaryFilesPattern, m_sIgnoreDirsPattern))
	{
		CString s;
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, LPCTSTR(m_sSecondDir));
		MessageBoxSync(s);
		return false;
	}

	return BuildFilePairList(FileList1, FileList2, pDlg);
}

bool CAlegrDiffDoc::BuildFilePairList(FileList & FileList1, FileList & FileList2,
									CProgressDialog * pDlg)
{
	typedef vector<FileItem *> FilesVector;
	typedef FilesVector::iterator FilesVectorIterator;
	FilesVector Files1;
	FilesVector Files2;

	FileList1.GetSortedList(Files1, FileList::SortDirFirst);
	FileList2.GetSortedList(Files2, FileList::SortDirFirst);

	// we don't call FreeFilePairList, because we could be performing file list refresh

	// amount of data to process
	ULONGLONG TotalFilesSize = 0;
	//    ULONG DirectoryNameIndex = 0;
	//    ULONG FileNameIndex = 0;
	// the list is sorted backwards. The folder goes last.
	ListHead<FilePair> TmpPairList;

	for (FilesVectorIterator pf1 = Files1.begin(), pf2 = Files2.begin();
		pf1 != Files1.end() || pf2 != Files2.end(); )
	{
		FilePair * pPair = new FilePair;

		int comparison;
		FileItem * pParentDir;
		FileItem * pFile1 = NULL;
		FileItem * pFile2 = NULL;
		FilePair::eFileComparisionResult result;

		if (pf1 == Files1.end())
		{
			pFile2 = *pf2;
			comparison = -1;
		}
		else
		{
			pFile1 = *pf1;

			if (pf2 == Files2.end())
			{
				comparison = 1;
			}
			else
			{
				pFile2 = *pf2;
				if (NULL != pFile1->m_pParentDir
					&& pFile1->m_pParentDir->IsAlone())
				{
					comparison = 1;
				}
				else if (NULL != pFile2->m_pParentDir
						&& pFile2->m_pParentDir->IsAlone())
				{
					comparison = -1;
				}
				else
				{
					comparison = -FileItem::DirNameCompare(pFile1, pFile2);
				}
			}
		}

		if (comparison < 0)
		{
			pPair->pFirstFile = NULL;
			pPair->pSecondFile = pFile2;
			pParentDir = pFile2->m_pParentDir;

			if (pFile2->IsFolder())
			{
				if (pParentDir != NULL
					&& pParentDir->IsAlone())
				{
					result = FilePair::SubdirsParentInSecondDirOnly;
				}
				else
				{
					result = FilePair::OnlySecondDirectory;
				}

				pFile2->SetAlone(true);
			}
			else
			{
				if (pParentDir != NULL
					&& pParentDir->IsAlone())
				{
					result = FilePair::FileFromSubdirInSecondDirOnly;
				}
				else
				{
					result = FilePair::OnlySecondFile;
				}
			}
			pf2++;

			if (0) TRACE(_T("File \"%s\" exists only in dir \"%s\"\n"),
						LPCTSTR(pFile2->GetName()),
						LPCTSTR(FileList2.m_BaseDir + pFile2->GetSubdir()));
		}
		else if (comparison > 0)
		{
			pPair->pFirstFile = pFile1;
			pPair->pSecondFile = NULL;
			pParentDir = pFile1->m_pParentDir;

			if (pFile1->m_bIsPhantomFile)
			{
				// reading fingerprint
				if (pFile1->IsFolder())
				{
					result = FilePair::DirectoryInFingerprintFileOnly;
				}
				else
				{
					result = FilePair::FileInFingerprintFileOnly;
				}
			}
			else
			{
				if (pFile1->IsFolder())
				{
					if (pParentDir != NULL
						&& pParentDir->IsAlone())
					{
						result = FilePair::SubdirsParentInFirstDirOnly;
					}
					else
					{
						result = FilePair::OnlyFirstDirectory;
					}

					pFile1->SetAlone(true);
				}
				else
				{
					if (pParentDir != NULL
						&& pParentDir->IsAlone())
					{
						result = FilePair::FileFromSubdirInFirstDirOnly;
					}
					else
					{
						result = FilePair::OnlyFirstFile;
					}
				}
			}
			pf1++;

			if (0) TRACE(_T("File \"%s\" exists only in dir \"%s\"\n"),
						LPCTSTR(pFile1->GetName()),
						LPCTSTR(FileList1.m_BaseDir + pFile1->GetSubdir()));
		}
		else
		{
			pPair->pFirstFile = pFile1;
			pPair->pSecondFile = pFile2;
			pf1++;
			pf2++;

			if (pFile1->IsFolder())
			{
				result = pPair->FilesIdentical;
			}
			else
			{
				result = pPair->ResultUnknown;
			}

			if (0) TRACE(_T("File \"%s\" exists in both \"%s\" and \"%s\"\n"),
						LPCTSTR(pFile1->GetName()),
						LPCTSTR(FileList1.m_BaseDir + pFile1->GetSubdir()),
						LPCTSTR(FileList2.m_BaseDir + pFile2->GetSubdir()));
		}

		pPair->SetComparisonResult(result);
		TmpPairList.InsertTail(pPair);
	}

	FilePair * pInsertBefore = m_PairList.First();

	while ( ! TmpPairList.IsEmpty())
	{
		FilePair * pPair = TmpPairList.RemoveTail();

		while (m_PairList.NotEnd(pInsertBefore))
		{
			// check if we insert or remove items, or the item is duplicate
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

			int comparison = FileItem::DirNameCompare(pItem1, pItem2);
			if (comparison < 0)
			{
				pInsertBefore->m_bDeleted = true;
				m_bNeedUpdateViews = true;
				pInsertBefore = pInsertBefore->Next();

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
				&& (pPair->NeedBinaryComparison() == pInsertBefore->NeedBinaryComparison()))
			{
				if ((pPair->pFirstFile != NULL
						&& pPair->pFirstFile->GetLastWriteTime() !=
						pInsertBefore->pFirstFile->GetLastWriteTime())
					|| (pPair->pSecondFile != NULL
						&& pPair->pSecondFile->GetLastWriteTime() !=
						pInsertBefore->pSecondFile->GetLastWriteTime()))
				{
					// files times changed only
					pInsertBefore->m_bChanged = true;
					pPair->SetComparisonResult(pPair->ResultUnknown);
					m_bNeedUpdateViews = true;
				}
				pPair->Dereference();
				pPair = NULL;
			}
			else
			{
				pInsertBefore->m_bDeleted = true;
				pPair->m_bFocused = pInsertBefore->m_bFocused;
				pPair->m_bSelected = pInsertBefore->m_bSelected;
				pInsertBefore->m_bFocused = false;
			}
			pInsertBefore = m_PairList.Next(pInsertBefore);
			break;
		}

		if (pPair != NULL)
		{
			m_bNeedUpdateViews = true;
			pInsertBefore->InsertAsPrevItem(pPair);
			m_nFilePairs++;

			if (pPair->ResultUnknown == pPair->GetComparisonResult())
			{
				// add files to the "data to process" size
				if (pPair->NeedBinaryComparison())
				{
					if (pPair->pFirstFile->GetFileLength()
						== pPair->pSecondFile->GetFileLength())
					{
						if ( ! pPair->pFirstFile->m_bMd5Calculated)
						{
							// overhead is 0x2000
							TotalFilesSize += FILE_OPEN_OVERHEAD + pPair->pFirstFile->GetFileLength();
						}
						if ( ! pPair->pSecondFile->m_bMd5Calculated)
						{
							TotalFilesSize += FILE_OPEN_OVERHEAD + pPair->pSecondFile->GetFileLength();
						}
					}
				}
				else
				{
					// text files
					TotalFilesSize += FILE_OPEN_OVERHEAD * 2 + 2 * (pPair->pFirstFile->GetFileLength()
										+ pPair->pSecondFile->GetFileLength());
				}
			}
		}
	}

	while (m_PairList.NotEnd(pInsertBefore))
	{
		pInsertBefore->m_bDeleted = true;
		pInsertBefore = m_PairList.Next(pInsertBefore);
	}
	// all files are referenced in FilePair list
	FileList1.Detach();
	FileList2.Detach();

	pDlg->SetTotalDataSize(TotalFilesSize);

	return true;
}

void CAlegrDiffDoc::FreeFilePairList()
{
	while (! m_PairList.IsEmpty())
	{
		m_PairList.RemoveHead()->Dereference();
	}
	m_nFilePairs = 0;
}

void CAlegrDiffDoc::OnUpdateAllViews(CView* pSender,
									LPARAM lHint, CObject* pHint)
{
	CAlegrDiffBaseDoc::OnUpdateAllViews(pSender, lHint, pHint);

	if (UpdateViewsFilePairDeleteFromList == lHint)
	{
		FilePairChangedArg * pArg = dynamic_cast<FilePairChangedArg *>(pHint);
		if (NULL != pArg)
		{
			FilePair * const pPairToDelete = pArg->m_pPair;

			CSimpleCriticalSectionLock lock(m_FileListCs);
			// find if it is in the list and remove from the list
			for (FilePair * pPair = m_PairList.First(); m_PairList.NotEnd(pPair); pPair = pPair->Next())
			{
				if (pPairToDelete == pPair)
				{
					pPair->RemoveFromList();
					pPair->Dereference();
					break;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc diagnostics

#ifdef _DEBUG
void CAlegrDiffDoc::AssertValid() const
{
	CAlegrDiffBaseDoc::AssertValid();
}

void CAlegrDiffDoc::Dump(CDumpContext& dc) const
{
	CAlegrDiffBaseDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffDoc commands

/////////////////////////////////////////////////////////////////////////////
// CFilePairDoc

IMPLEMENT_DYNCREATE(CFilePairDoc, CAlegrDiffBaseDoc)

CFilePairDoc::CFilePairDoc()
	: m_TotalLines(0),
	m_pFilePair(NULL),
	m_CopyDisabled(false),
	m_CaretPos(0, 0, 0)
	, m_SelectionAnchor(0, 0, 0)
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

void CFilePairDoc::OnUpdateAllViews(CView* pSender,
									LPARAM lHint, CObject* pHint)
{
	if (UpdateViewsFilePairDeleteFromList == lHint
		|| UpdateViewsFilePairDeleteView == lHint)
	{
		FilePairChangedArg * pArg = dynamic_cast<FilePairChangedArg *>(pHint);
		if (NULL != pArg
			&& pArg->m_pPair == m_pFilePair)
		{
			OnCloseDocument();
			return;
		}
	}
	else
	{
		CAlegrDiffBaseDoc::OnUpdateAllViews(pSender, lHint, pHint);
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
		if (NULL != pPair->pFirstFile && ! pPair->pFirstFile->m_bIsPhantomFile)
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
			SetTitle(pPair->pSecondFile->GetFullName());
		}
		else
		{
			SetTitle(_T(""));
		}

		if (pPair->m_LinePairs.empty())
		{
			//UpdateAllViews(NULL, 0);    // erase the views
			((CFrameWnd*)AfxGetMainWnd())->SetMessageText(_T("Loading and comparing files..."));

			CWaitCursor WaitCursor;
			pPair->SetComparisonResult(pPair->CompareFiles(NULL));
		}

		m_TotalLines = pPair->m_LinePairs.size();
		_tcsncpy_s(m_ComparisonResult, countof(m_ComparisonResult), pPair->GetComparisonResultStr(),
					countof(m_ComparisonResult));
		m_ComparisonResult[countof(m_ComparisonResult) - 1] = 0;
		((CFrameWnd*)AfxGetMainWnd())->PostMessage(WM_SETMESSAGESTRING_POST, 0, (LPARAM)m_ComparisonResult);
	}

	GetApp()->NotifyFilePairChanged(pPair);

	SetCaretPosition(0, 0, SetPositionCancelSelection);
}

void CFilePairDoc::SetSelection(TextPosDisplay CaretPos, TextPosDisplay AnchorPos, int flags)
{
	m_SelectionAnchor = AnchorPos;
	SetCaretPosition(CaretPos.pos, CaretPos.line, flags & ~SetPositionCancelSelection);
}

void CFilePairDoc::SetCaretPosition(int pos, int line, int flags)
{
	SetCaretPosition(TextPosDisplay(line, pos, m_CaretPos.scope), flags);
}

void CFilePairDoc::SetCaretPosition(TextPosLine pos, int FileScope, int flags)
{
	SetCaretPosition(LinePosToDisplayPos(pos, FileScope), flags);
}

void CFilePairDoc::SetCaretPosition(TextPosDisplay pos, int flags)
{
	if (pos.line > GetTotalLines())
	{
		pos.line = GetTotalLines();
	}
	if (pos.line < 0)
	{
		pos.line = 0;
	}
	m_CaretPos.line = pos.line;

	if (pos.pos < 0)
	{
		pos.pos = 0;
	}
	if (pos.pos > 2048)
	{
		pos.pos = 2048;
	}
	m_CaretPos.pos = pos.pos;
	m_CaretPos.scope = pos.scope;

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
		TextPosDisplay AnchorBegin = m_OriginalSelectionAnchor;
		TextPosDisplay AnchorEnd = m_OriginalSelectionAnchor;
		GetWordOnPos(m_OriginalSelectionAnchor, AnchorBegin, AnchorEnd);

		TextPosDisplay Begin = m_CaretPos;
		TextPosDisplay End = m_CaretPos;
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

		for (StringSection * pSection = pLine->StrSections.First();
			pLine->StrSections.NotEnd(pSection); pSection = pSection->Next())
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
		for (StringSection * pSection = pLine->StrSections.First();
			pLine->StrSections.NotEnd(pSection); pSection = pSection->Next())
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

BEGIN_MESSAGE_MAP(CFilePairDoc, CAlegrDiffBaseDoc)
	//{{AFX_MSG_MAP(CFilePairDoc)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTONEXTDIFF, OnUpdateEditGotonextdiff)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTOPREVDIFF, OnUpdateEditGotoprevdiff)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
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
	ON_COMMAND(ID_VIEW_AS_BINARY, OnViewAsBinary)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CARET_POS, OnUpdateCaretPosIndicator)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilePairDoc diagnostics

#ifdef _DEBUG
void CFilePairDoc::AssertValid() const
{
	CAlegrDiffBaseDoc::AssertValid();
}

void CFilePairDoc::Dump(CDumpContext& dc) const
{
	CAlegrDiffBaseDoc::Dump(dc);
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

ULONG CFilePairDoc::CopyTextToMemory(LPTSTR pBuf, ULONG BufLen,
									TextPosDisplay pFrom, TextPosDisplay pTo,
									int FileSelect)
{
	// FileSelect 1 - file 1, 2 - file 2, 0 = both files
	ULONG TotalChars = 0;
//    CThisApp * pApp = GetApp();
	TextPosDisplay begin, end;
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

		if ((1 == FileSelect && NULL == pPair->pFirstLine)
			|| (2 == FileSelect && NULL == pPair->pSecondLine))
		{
			continue;
		}

		for (StringSection * pSection = pPair->StrSections.First();
			pPair->StrSections.NotEnd(pSection); pSection = pSection->Next())
		{
			if ((pSection->Attr & pSection->Whitespace)
				&& (pSection->Attr & pSection->Erased)
				&& m_bIgnoreWhitespaces && 0 == FileSelect)
			{
				continue;   // don't show the section
			}
			if ((pSection->Attr & pSection->Erased)
				&& 2 == FileSelect)
			{
				continue;
			}
			if ((pSection->Attr & pSection->Inserted)
				&& 1 == FileSelect)
			{
				continue;
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

void CFilePairDoc::OnEditCopy(int FileSelect)
{
	if(m_CopyDisabled)
	{
		return;
	}
	// todo: perform UNICODE or ANSI copy?
	// calculate length of the selection
	ULONG Len = CopyTextToMemory(NULL, 0, m_SelectionAnchor, m_CaretPos, FileSelect);
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
		CopyTextToMemory(pMem, Len, m_SelectionAnchor, m_CaretPos, FileSelect);
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

	CThisApp * pApp = GetApp();
	// rescan the directories again
	m_bNeedUpdateViews = false;

	CComparisonProgressDlg dlg(this);

	// release all open files
	pApp->UpdateAllViews(UpdateViewsCloseOpenFiles);

	dlg.DoModalDelay();

	bool HasFiles = false;

	for (FilePair * pPair = m_PairList.First();
		m_PairList.NotEnd(pPair); )
	{
		if (pPair->m_bDeleted)
		{
			FilePair * pTmp = pPair;
			pPair = m_PairList.Next(pPair);
			pTmp->RemoveFromList();

			FilePairChangedArg arg(pTmp);

			pApp->UpdateAllViews(UpdateViewsFilePairDeleteView, & arg);

			pTmp->Dereference();
			m_bNeedUpdateViews = true;
		}
		else
		{
			if (pPair->FilesIdentical != pPair->GetComparisonResult()
				|| ! pPair->pFirstFile->IsFolder())
			{
				HasFiles = true;
			}

			if (pPair->m_bChanged)
			{
				pPair->m_bChanged = false;
				pApp->NotifyFilePairChanged(pPair);
			}
			pPair = m_PairList.Next(pPair);
		}
	}

	if ( ! HasFiles)
	{
		AfxMessageBox(IDS_STRING_NO_FILES_TO_COMPARE, MB_OK);
		OnCloseDocument();      // close all views
		return;
	}

	if (m_bNeedUpdateViews)
	{
		// scan to remove the deleted pairs

		UpdateAllViews(NULL, OnUpdateRebuildListView);
	}
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
	int flags = GetAcceptDeclineFlags(TextPosLine(0, 0), TextPosLine(GetTotalLines(), 0));
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
		FilePairChangedArg arg(m_pFilePair);
		GetApp()->UpdateAllViews(UpdateViewsFilePairDeleteFromList, & arg);
		return;
	}

	TRACE("Reloading the files\n");
	TextPosDisplay caretpos = m_CaretPos;

	m_pFilePair->LoadFiles();   // make one more reference
	m_pFilePair->Reference();

	SetFilePair(m_pFilePair);

	m_pFilePair->UnloadFiles(); // decrement extra reference
	m_pFilePair->Dereference();

	SetCaretPosition(caretpos, SetPositionCancelSelection);

	GetApp()->NotifyFilePairChanged(m_pFilePair);
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

bool CFilePairDoc::FindTextString(LPCTSTR pStrToFind, bool bBackward, bool bCaseSensitive, bool WholeWord, int SearchScope)
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
	TextPosDisplay LineSelectionAnchor = m_SelectionAnchor; //DisplayPosToLinePos(m_SelectionAnchor);
	TextPosDisplay LineCaretPos = m_CaretPos; //DisplayPosToLinePos(m_CaretPos);

	int nSearchPos;
	unsigned nSearchLine;

	if (0 == SearchScope)
	{
		WholeWord = false;
	}

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
	unsigned nStartSearchLine = nSearchLine;
	int nPatternLen = _tcslen(pStrToFind);

	if (0 == nPatternLen)
	{
		return FALSE;
	}

	while(1)
	{
		LPCTSTR pStr = NULL;
		int StrLen = 0;
		pStr = GetLineText(nSearchLine, line, countof(line), & StrLen, SearchScope);

		if ( ! bBackward)
		{
			for ( ;nSearchPos <= StrLen - nPatternLen; nSearchPos++)
			{
				if (WholeWord)
				{
					// check if it is word boundary
					if ('_' == pStrToFind[0] || _istalnum(TCHAR_MASK & pStrToFind[0]))
					{
						// check that the previous character is NOT alphanum
						if (nSearchPos > 0
							&& ('_' == pStr[nSearchPos - 1]
								|| _istalnum(TCHAR_MASK & pStr[nSearchPos - 1])))
						{
							continue;
						}
					}
					// check if the pattern end is on word boundary
					if ('_' == pStrToFind[nPatternLen - 1] ||
						_istalnum(TCHAR_MASK & pStrToFind[nPatternLen - 1]))
					{
						// check that the next character is NOT alphanum
						if (nSearchPos + nPatternLen < StrLen
							&& ('_' == pStr[nSearchPos + nPatternLen] ||
								_istalnum(TCHAR_MASK & pStr[nSearchPos + nPatternLen])))
						{
							continue;
						}
					}
				}
				if (bCaseSensitive)
				{
					if (0 == _tcsncmp(pStr + nSearchPos, pStrToFind, nPatternLen))
					{
						// found
						SetSelection(TextPosDisplay(nSearchLine, nSearchPos + nPatternLen, SearchScope),
									TextPosDisplay(nSearchLine, nSearchPos, SearchScope));
						return true;
					}
				}
				else
				{
					if (0 ==_tcsnicmp(pStr + nSearchPos, pStrToFind, nPatternLen))
					{
						// found
						SetSelection(TextPosDisplay(nSearchLine, nSearchPos + nPatternLen, SearchScope),
									TextPosDisplay(nSearchLine, nSearchPos, SearchScope));
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
				if (WholeWord)
				{
					// check if it is word boundary
					if ('_' == pStrToFind[0] || _istalnum(TCHAR_MASK & pStrToFind[0]))
					{
						// check that the previous character is NOT alphanum
						if (nSearchPos > 0
							&& ('_' == pStr[nSearchPos - 1]
								|| _istalnum(TCHAR_MASK & pStr[nSearchPos - 1])))
						{
							continue;
						}
					}
					// check if the pattern end is on word boundary
					if ('_' == pStrToFind[nPatternLen - 1] ||
						_istalnum(TCHAR_MASK & pStrToFind[nPatternLen - 1]))
					{
						// check that the next character is NOT alphanum
						if (nSearchPos + nPatternLen < StrLen
							&& ('_' == pStr[nSearchPos + nPatternLen] ||
								_istalnum(TCHAR_MASK & pStr[nSearchPos + nPatternLen])))
						{
							continue;
						}
					}
				}
				if (bCaseSensitive)
				{
					if (0 == _tcsncmp(pStr + nSearchPos, pStrToFind, nPatternLen))
					{
						// found
						SetSelection(TextPosDisplay(nSearchLine, nSearchPos + nPatternLen, SearchScope),
									TextPosDisplay(nSearchLine, nSearchPos, SearchScope));
						return true;
					}
				}
				else
				{
					if (0 ==_tcsnicmp(pStr + nSearchPos, pStrToFind, nPatternLen))
					{
						// found
						SetSelection(TextPosDisplay(nSearchLine, nSearchPos + nPatternLen, SearchScope),
									TextPosDisplay(nSearchLine, nSearchPos, SearchScope));
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

bool CFilePairDoc::GetWordOnPos(TextPosDisplay OnPos, TextPosDisplay &Start, TextPosDisplay &End)
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

	ListHead<StringSection> StrSections;
	StringSection Section;
	ListHead<StringSection> * pStrSections;

	if (0 == OnPos.scope)
	{
		pStrSections = & pPair->StrSections;
	}
	else
	{
		pStrSections = & StrSections;
		Section.Attr = 0;
		Section.pDiffSection = NULL;
		if (1 == OnPos.scope)
		{
			if (NULL == pPair->pFirstLine)
			{
				return false;
			}
			Section.pBegin = pPair->pFirstLine->GetText();
			Section.Length = (USHORT)pPair->pFirstLine->GetLength();
		}
		else
		{
			if (NULL == pPair->pSecondLine)
			{
				return false;
			}
			Section.pBegin = pPair->pSecondLine->GetText();
			Section.Length = (USHORT)pPair->pSecondLine->GetLength();
		}
		StrSections.InsertHead( & Section);
	}

	for (StringSection * pSection = pStrSections->First();
		pStrSections->NotEnd(pSection); pSection = pSection->Next())
	{
		if ((pSection->Attr & pSection->Whitespace)
			&& (pSection->Attr & pSection->Erased)
			&& m_bIgnoreWhitespaces && 0 == OnPos.scope)
		{
			continue;   // don't copy the section
		}
		if ((pSection->Attr & pSection->Erased)
			&& 2 == OnPos.scope)
		{
			continue;   // don't copy the section
		}
		if ((pSection->Attr & pSection->Inserted)
			&& 1 == OnPos.scope)
		{
			continue;   // don't copy the section
		}
		// if position is on the end of line or on space, and the previous char is alpha, get the word to the left
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
				// if position is on the end of line or on space, and the previous char is alpha, get the word to the left
				if (CaretPos > nPos)
				{
					c = pSection->pBegin[CaretPos - nPos - 1];
					if (_istalnum(TCHAR_MASK & c) || '_' == c)
					{
						CaretPos --;
					}
					else
					{
#ifdef _DEBUG
						StrSections.RemoveHead();
#endif
						return false;
					}
				}
				else
				{
#ifdef _DEBUG
					StrSections.RemoveHead();
#endif
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
#ifdef _DEBUG
					StrSections.RemoveHead();
#endif
					return false;
				}
				// get one char under the cursor
				Start.pos = CaretPos;
				End.pos = CaretPos + 1;
			}
#ifdef _DEBUG
			StrSections.RemoveHead();
#endif
			return true;
		}
		nPos += pSection->Length;
	}

#ifdef _DEBUG
	StrSections.RemoveHead();
#endif
	return false;
}

void CFilePairDoc::GetWordUnderCursor(CString & Str)
{
	int nBeginOffset = 0;
	int nLength = 0;
	if (m_CaretPos == m_SelectionAnchor
		|| m_CaretPos.line != m_SelectionAnchor.line)
	{
		TextPosDisplay Begin, End;
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
		LPCTSTR pStr = GetLineText(m_CaretPos.line, line, countof(line), & StrLen, m_CaretPos.scope);
		if (NULL != pStr
			&& nBeginOffset < StrLen)
		{
			if (nBeginOffset + nLength > StrLen)
			{
				nLength = StrLen - nBeginOffset;
			}
			LPTSTR StrBuf = Str.GetBuffer(nLength);
			if (NULL != StrBuf)
			{
				_tcsncpy(StrBuf, pStr + nBeginOffset, nLength);
				Str.ReleaseBuffer(nLength);
			}
		}
	}
}

// returns a pointer to a line text
// buf is used to assembly the string if it is fragmented
LPCTSTR CFilePairDoc::GetLineText(int nLineNum, LPTSTR buf, size_t BufChars, int *pStrLen, int Scope)
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
		buf[0] = 0;
		* pStrLen = 0;
		return buf;
	}
	return pPair->GetText(buf, BufChars, pStrLen, m_bIgnoreWhitespaces, Scope);
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
		pLine = GetLineText(CaretLine, linebuf, 2048, & StrLen, m_CaretPos.scope);
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
			pLine = GetLineText(CaretLine, linebuf, 2048, & StrLen, m_CaretPos.scope);
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
	pLine = GetLineText(CaretLine, linebuf, 2048, & StrLen, m_CaretPos.scope);
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
			pLine = GetLineText(CaretLine, linebuf, 2048, & StrLen, m_CaretPos.scope);
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

		TextPosLine begin = DisplayPosToLinePos(m_SelectionAnchor);
		TextPosLine end = DisplayPosToLinePos(m_CaretPos);
		if (m_pFilePair->ModifyAcceptDeclineFlags(begin, end, SetFlags, ResetFlags))
		{
			InvalidatedRange ir;
			ir.begin = begin;
			ir.end = end;
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
		TextPosLine begin = DisplayPosToLinePos(m_SelectionAnchor);
		TextPosLine end = DisplayPosToLinePos(m_CaretPos);
		if (m_pFilePair->ModifyAcceptDeclineFlags(begin, end, SetFlags, ResetFlags))
		{
			InvalidatedRange ir;
			ir.begin = begin;
			ir.end = end;
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
	int flags = m_pFilePair->GetAcceptDeclineFlags(TextPosLine(0, 0),
													TextPosLine(GetTotalLines(), 0), false);
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

class CMergedFilesSaveDlg : public CFileDialogWithHistory
{
public:
	CMergedFilesSaveDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
						LPCTSTR lpszDefExt = NULL,
						LPCTSTR lpszFileName = NULL,
						DWORD dwFlags = OFN_NOCHANGEDIR
										| OFN_HIDEREADONLY
										| OFN_NOREADONLYRETURN
										| OFN_PATHMUSTEXIST
										| OFN_OVERWRITEPROMPT,
						LPCTSTR lpszFilter = NULL,
						CWnd* pParentWnd = NULL)
		: CFileDialogWithHistory(bOpenFileDialog, & GetApp()->m_RecentFolders, lpszDefExt, lpszFileName,
								dwFlags, lpszFilter, pParentWnd),
		m_bUnicode(FALSE)
	{
		if (OPENFILENAME_SIZE_VERSION_400 == m_ofn.lStructSize)
		{
			m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_SAVE_MERGED_TEMPLATE_V4);
		}
		else
		{
			m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_SAVE_MERGED_TEMPLATE_V5);
		}

		m_ofn.Flags |= OFN_ENABLETEMPLATE;
	}
	BOOL m_bUnicode;
	virtual void OnInitDone( )
	{
		CFileDialogWithHistory::OnInitDone();
		CheckDlgButton(IDC_CHECK_UNICODE, m_bUnicode);
	}

	void OnCheckUnicode()
	{
		m_bUnicode = IsDlgButtonChecked(IDC_CHECK_UNICODE);
	}
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CMergedFilesSaveDlg, CFileDialogWithHistory)
	//{{AFX_MSG_MAP(CMergedFilesSaveDlg)
	ON_BN_CLICKED(IDC_CHECK_UNICODE, OnCheckUnicode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CFilePairDoc::DoSaveMerged(BOOL bOpenResultFile)
{
	// check if there are unmarked differences
	int flags = GetAcceptDeclineFlags(TextPosLine(0, 0),
									TextPosLine(GetTotalLines(), 0));
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
							| OFN_NOTESTFILECREATE
							| OFN_PATHMUSTEXIST
							| OFN_NONETWORKBUTTON
							| OFN_NOCHANGEDIR
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
		CString FileName = dlg.GetPathName();
		pApp->m_LastSaveMergedDir = dlg.GetLastFolder();

		if (! SaveMergedFile(FileName, DefaultFlags, dlg.m_bUnicode))
		{
			AfxMessageBox(IDS_STRING_COULDNT_SAVE_MERGED);
			return FALSE;
		}
		SetModifiedFlag(FALSE);
		if (bOpenResultFile)
		{
#ifndef DEMO_VERSION
			pApp->OpenSingleFile(FileName);
#else
			AfxMessageBox("DEMO version doesn't save the merged file. You can only view it\n", MB_OK);
#endif
		}
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
	FILE * file = NULL;
	_tfopen_s(& file, Name, FileMode);
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

		StringSection * pSection = pPair->StrSections.First();

		if (NULL == pPair->pFirstLine && pPair->StrSections.NotEnd(pSection))
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
		else if (NULL == pPair->pSecondLine && pPair->StrSections.NotEnd(pSection))
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
		for ( ; pPair->StrSections.NotEnd(pSection); pSection = pSection->Next())
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
	TextPosLine CaretLinePos = DisplayPosToLinePos(m_CaretPos);
	TextPosLine AnchorLinePos = DisplayPosToLinePos(m_SelectionAnchor);

	m_bIgnoreWhitespaces = ! m_bIgnoreWhitespaces;
	GetApp()->m_bIgnoreWhitespaces = m_bIgnoreWhitespaces;

	m_CaretPos = LinePosToDisplayPos(CaretLinePos, m_CaretPos.scope);
	m_SelectionAnchor = LinePosToDisplayPos(AnchorLinePos, m_CaretPos.scope);

	UpdateAllViews(NULL);
}

TextPosDisplay CFilePairDoc::LinePosToDisplayPos(TextPosLine position, int FileScope)
{
	if (NULL == m_pFilePair)
	{
		return TextPosDisplay(position.line, position.pos, FileScope);
	}
	return m_pFilePair->LinePosToDisplayPos(position, m_bIgnoreWhitespaces, FileScope);
}
// recalculates offset in the line with or without whitespaces shown to offset in the raw line
TextPosLine CFilePairDoc::DisplayPosToLinePos(TextPosDisplay position)
{
	if (NULL == m_pFilePair)
	{
		return TextPosLine(position.line, position.pos);
	}
	return m_pFilePair->DisplayPosToLinePos(position, m_bIgnoreWhitespaces);
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

int CFilePairDoc::GetAcceptDeclineFlags(TextPosLine begin, TextPosLine end)
{
	if (NULL == m_pFilePair)
	{
		return StringSection::NoDifference;
	}
	return m_pFilePair->GetAcceptDeclineFlags(begin, end, m_bIgnoreWhitespaces);
}

int CFilePairDoc::GetAcceptDeclineFlags(TextPosDisplay begin, TextPosDisplay end)
{
	if (NULL == m_pFilePair)
	{
		return StringSection::NoDifference;
	}
	return m_pFilePair->GetAcceptDeclineFlags(DisplayPosToLinePos(begin),
											DisplayPosToLinePos(end), m_bIgnoreWhitespaces);
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

unsigned CAlegrDiffDoc::CompareDirectoriesFunction(CComparisonProgressDlg * pDlg)
{
	if ( ! RebuildFilePairList(pDlg))
	{
		// TODO
		return IDABORT;
	}
	// preload first of binary files in pair (calculate MD5 digest)

	CMd5HashCalculator HashCalc;
	FilePair * pPair;

	for (pPair = m_PairList.First();
		m_PairList.NotEnd(pPair) && (NULL == pDlg || ! pDlg->m_StopRunThread);
		pPair = pPair->Next())
	{
		if (0) TRACE("First pass, pPair=%p, result=%d\n", pPair, pPair->m_ComparisonResult);
		if (pPair->m_ComparisonResult != FilePair::ResultUnknown
			|| NULL == pPair->pFirstFile
			|| NULL == pPair->pSecondFile
			|| ! pPair->NeedBinaryComparison()
			|| pPair->pFirstFile->GetFileLength() != pPair->pSecondFile->GetFileLength())
		{
			continue;
		}

		pPair->SetComparisonResult(pPair->CalculatingFirstFingerprint);

		pDlg->SetNextItem(pPair->GetComparisonResultStr(),
						pPair->pFirstFile->GetFileLength(), FILE_OPEN_OVERHEAD);

		if (pPair->pFirstFile->CalculateHashes( & HashCalc, pDlg)
			|| pDlg->m_StopRunThread)
		{
			pPair->SetComparisonResult(FilePair::ResultUnknown);
		}
		else
		{
			pPair->SetComparisonResult(FilePair::ErrorReadingFirstFile);
		}

		pDlg->AddDoneItem(pPair->pFirstFile->GetFileLength());

		pPair->m_bChanged = true;

	}

	for (pPair = m_PairList.First();
		m_PairList.NotEnd(pPair) && (NULL == pDlg || ! pDlg->m_StopRunThread);
		pPair = pPair->Next())
	{
		if (0) TRACE("Second pass, pPair=%p, result=%d\n", pPair, pPair->GetComparisonResult());
		if (FilePair::ResultUnknown != pPair->GetComparisonResult())
		{
			continue;
		}

		pPair->SetComparisonResult(pPair->PreCompareFiles( & HashCalc, pDlg));
		pPair->m_bChanged = true;

	}

	return IDOK;
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
		TextPosLine begin = DisplayPosToLinePos(m_SelectionAnchor);
		TextPosLine end = DisplayPosToLinePos(m_CaretPos);
		int flags = GetAcceptDeclineFlags(begin, end);

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

		if (m_pFilePair->ModifyAcceptDeclineFlags(begin, end, SetFlags, ResetFlags))
		{
			InvalidatedRange ir;
			ir.begin = begin;
			ir.end = end;
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
		TextPosLine begin = DisplayPosToLinePos(m_SelectionAnchor);
		TextPosLine end = DisplayPosToLinePos(m_CaretPos);

		int flags = GetAcceptDeclineFlags(begin, end);

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

		if (m_pFilePair->ModifyAcceptDeclineFlags(begin, end, SetFlags, ResetFlags))
		{
			InvalidatedRange ir;
			ir.begin = begin;
			ir.end = end;
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

void CFilePairDoc::OnViewAsBinary()
{
	FilePair * pPair = GetFilePair();

	pPair->Reference();
	OnCloseDocument();

	if (NULL == pPair->pFirstFile
		|| ! pPair->pFirstFile->m_bIsPhantomFile)
	{
		pPair->SetComparisonResult(pPair->ResultUnknown);
	}

	pPair->UnloadFiles(true);

	if (NULL != pPair->pFirstFile)
	{
		pPair->pFirstFile->m_IsBinary = true;
	}
	if (NULL != pPair->pSecondFile)
	{
		pPair->pSecondFile->m_IsBinary = true;
	}

	GetApp()->OpenFilePairView(pPair);
	pPair->Dereference();
}
