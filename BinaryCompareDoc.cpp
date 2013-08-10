// BinaryCompareDoc.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "BinaryCompareDoc.h"
#include "DifferenceProgressDialog.h"
// CBinaryCompareDoc

IMPLEMENT_DYNCREATE(CBinaryCompareDoc, CAlegrDiffBaseDoc)

CBinaryCompareDoc::CBinaryCompareDoc()
	: m_pFilePair(NULL)
	, m_CaretPos(0)
	, m_SelectionAnchor(0)
	, m_OriginalSelectionAnchor(0)
{
}

BOOL CBinaryCompareDoc::OnNewDocument()
{
	if (!CAlegrDiffBaseDoc::OnNewDocument())
		return FALSE;
	m_CaretPos = 0;
	m_SelectionAnchor = 0;
	m_OriginalSelectionAnchor = 0;
	return TRUE;
}

CBinaryCompareDoc::~CBinaryCompareDoc()
{
	if (NULL != m_pFilePair)
	{
		m_pFilePair->Dereference();
	}
}

void CBinaryCompareDoc::OnUpdateAllViews(CView* pSender,
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
	else if (UpdateViewsCloseOpenFiles == lHint)
	{
		m_pFilePair->UnloadFiles(true); // force close
	}
	else
	{
		CAlegrDiffBaseDoc::OnUpdateAllViews(pSender, lHint, pHint);
	}
}

BEGIN_MESSAGE_MAP(CBinaryCompareDoc, CAlegrDiffBaseDoc)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CARET_POS, OnUpdateCaretPosIndicator)
	ON_COMMAND(ID_VIEW_VIEWASTEXTFILES, OnViewViewastextfiles)
END_MESSAGE_MAP()


// CBinaryCompareDoc diagnostics

#ifdef _DEBUG
void CBinaryCompareDoc::AssertValid() const
{
	CAlegrDiffBaseDoc::AssertValid();
}

void CBinaryCompareDoc::Dump(CDumpContext& dc) const
{
	CAlegrDiffBaseDoc::Dump(dc);
}
#endif //_DEBUG


// CBinaryCompareDoc serialization

void CBinaryCompareDoc::Serialize(CArchive& ar)
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


// CBinaryCompareDoc commands
void CBinaryCompareDoc::SetFilePair(FilePair * pPair)
{
	if (NULL != m_pFilePair)
	{
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
			CString title(pPair->pSecondFile->GetFullName());
			SetTitle(title);
		}
		else
		{
			SetTitle(_T(""));
		}
	}
	m_CaretPos = 0;


	SetCaretPosition(0, SetPositionCancelSelection | SetPositionMakeVisible);

	GetApp()->NotifyFilePairChanged(pPair);

	if (pPair->ResultUnknown == pPair->GetComparisonResult())
	{
		CDifferenceProgressDialog dlg;
		dlg.m_pDoc = this;
		dlg.BeginAddr = m_CaretPos;
		dlg.EndAddr = GetFileSize();

		INT_PTR result = dlg.DoModalDelay(200);

		if (IDOK == result)
		{
			if (dlg.BeginAddr == dlg.EndAddr)
			{
				dlg.BeginAddr = 0;
				pPair->SetComparisonResult(pPair->FilesIdentical);
			}
			else
			{
				pPair->SetComparisonResult(pPair->FilesDifferent);
			}
			SetCaretPosition(dlg.BeginAddr,
							SetPositionCancelSelection | SetPositionMakeVisible);

		}
		else
		{
			SetCaretPosition(0, SetPositionCancelSelection | SetPositionMakeVisible);
		}
	}

	_tcsncpy_s(m_ComparisonResult, countof(m_ComparisonResult), pPair->GetComparisonResultStr(),
				countof(m_ComparisonResult));
	m_ComparisonResult[countof(m_ComparisonResult) - 1] = 0;
	((CFrameWnd*)AfxGetMainWnd())->PostMessage(WM_SETMESSAGESTRING_POST, 0, (LPARAM)m_ComparisonResult);

	GetApp()->NotifyFilePairChanged(pPair);
}

LONGLONG CBinaryCompareDoc::GetFileSize() const
{
	if (NULL == m_pFilePair
		|| (NULL == m_pFilePair->pFirstFile && NULL == m_pFilePair->pSecondFile))
	{
		return 0;
	}
	if (NULL == m_pFilePair->pFirstFile)
	{
		return m_pFilePair->pSecondFile->GetFileLength();
	}
	if (NULL == m_pFilePair->pSecondFile
		|| m_pFilePair->pFirstFile->GetFileLength() >= m_pFilePair->pSecondFile->GetFileLength())
	{
		return m_pFilePair->pFirstFile->GetFileLength();
	}
	else
	{
		return m_pFilePair->pSecondFile->GetFileLength();
	}
}

void CBinaryCompareDoc::SetCaretPosition(LONGLONG Addr, int flags)
{
	if (Addr > GetFileSize())
	{
		Addr = GetFileSize();
	}
	if (Addr < 0)
	{
		Addr = 0;
	}

	m_CaretPos = Addr;

	if (0 != (flags & SetPositionCancelSelection))
	{
		m_SelectionAnchor = m_CaretPos;
		m_OriginalSelectionAnchor = m_CaretPos;  // for word mode selection
		// if canceling selection, check for word mode reset
	}
	UpdateAllViews(NULL, CaretPositionChanged, NULL);

	if (flags & SetPositionMakeVisible)
	{
		UpdateAllViews(NULL, UpdateMakeCaretVisible);
	}
	if (flags & SetPositionMakeCentered)
	{
		UpdateAllViews(NULL, UpdateMakeCaretCentered);
	}
}

void CBinaryCompareDoc::OnUpdateCaretPosIndicator(CCmdUI* pCmdUI)
{
	CString s;
	s.Format(_T("%08I64X"), m_CaretPos);
	pCmdUI->SetText(s);
}

unsigned CBinaryCompareDoc::FindDataProc(CDifferenceProgressDialog * pDlg)
{
	FilePair * pFilePair = GetFilePair();
	if (NULL == pFilePair
		|| NULL == pFilePair->pFirstFile
		|| NULL == pFilePair->pSecondFile)
	{
		return IDCANCEL;
	}
	int const BufferSize = 0x10000;
	UCHAR * File1Buffer = new UCHAR[BufferSize];
	UCHAR * File2Buffer = new UCHAR[BufferSize];
	long File1BufFilled = 0;
	long File2BufFilled = 0;
	long File1BufIndex = 0;
	long File2BufIndex = 0;
	BOOL FindingFirstDifference = FALSE;

	LONGLONG Addr = pDlg->BeginAddr;

	if (Addr > pDlg->EndAddr)
	{
		FindingFirstDifference = TRUE;

		pDlg->SetTotalDataSize(Addr - pDlg->EndAddr);
		pDlg->SetNextItem(_T(""), Addr - pDlg->EndAddr, 0);

		// search backward
		while (! pDlg->m_StopRunThread
				&& Addr > pDlg->EndAddr)
		{
			if (File1BufIndex <= 0)
			{
				ULONG ToRead = BufferSize;
				if (Addr - pDlg->EndAddr < BufferSize)
				{
					ToRead = ULONG(Addr - pDlg->EndAddr);
				}

				File1BufFilled = pFilePair->pFirstFile->GetFileData(Addr - ToRead,
									File1Buffer, ToRead);
				File1BufIndex = File1BufFilled;
			}

			if (File2BufIndex <= 0)
			{
				ULONG ToRead = BufferSize;
				if (Addr - pDlg->EndAddr < BufferSize)
				{
					ToRead = ULONG(Addr - pDlg->EndAddr);
				}

				File2BufFilled = pFilePair->pSecondFile->GetFileData(Addr - ToRead,
									File2Buffer, ToRead);
				File2BufIndex = File2BufFilled;

				pDlg->SetCurrentItemDone(pDlg->BeginAddr - Addr);
			}

			if (0 == File1BufIndex
				|| 0 == File2BufIndex)
			{
				break;
			}

			File1BufIndex--;
			File2BufIndex--;

			if (FindingFirstDifference)
			{
				if (File1Buffer[File1BufIndex] != File2Buffer[File2BufIndex])
				{
					FindingFirstDifference = FALSE;
				}
			}
			else
			{
				if (File1Buffer[File1BufIndex] == File2Buffer[File2BufIndex])
				{
					break;
				}
			}
			Addr--;
		}
		pDlg->BeginAddr = Addr;
	}
	else
	{
		// search forward
		pDlg->SetTotalDataSize(pDlg->EndAddr - Addr);
		pDlg->SetNextItem(_T(""), pDlg->EndAddr - Addr, 0);

		while (! pDlg->m_StopRunThread
				&& Addr < pDlg->EndAddr)
		{
			if (File1BufIndex >= File1BufFilled)
			{
				ULONG ToRead = BufferSize;
				if (pDlg->EndAddr - Addr < BufferSize)
				{
					ToRead = ULONG(pDlg->EndAddr - Addr);
				}

				File1BufFilled = pFilePair->pFirstFile->GetFileData(Addr,
									File1Buffer, ToRead);
				File1BufIndex = 0;
			}

			if (File2BufIndex >= File2BufFilled)
			{
				ULONG ToRead = BufferSize;
				if (pDlg->EndAddr - Addr < BufferSize)
				{
					ToRead = ULONG(pDlg->EndAddr - Addr);
				}

				File2BufFilled = pFilePair->pSecondFile->GetFileData(Addr,
									File2Buffer, ToRead);
				File2BufIndex = 0;

				pDlg->SetCurrentItemDone(Addr - pDlg->BeginAddr);
			}

			if (File1BufIndex >= File1BufFilled
				|| File2BufIndex >= File2BufFilled)
			{
				break;
			}

			if (FindingFirstDifference)
			{
				if (File1Buffer[File1BufIndex] != File2Buffer[File2BufIndex])
				{
					break;
				}
			}
			else
			{
				if (File1Buffer[File1BufIndex] == File2Buffer[File2BufIndex])
				{
					FindingFirstDifference = TRUE;
				}
			}
			File1BufIndex++;
			File2BufIndex++;
			Addr++;
		}
		pDlg->BeginAddr = Addr;
	}
	delete[] File1Buffer;
	delete[] File2Buffer;
	if (pDlg->m_StopRunThread)
	{
		return IDCANCEL;
	}

	return IDOK;
}

void CBinaryCompareDoc::OnViewViewastextfiles()
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
		pPair->pFirstFile->m_IsBinary = false;
	}
	if (NULL != pPair->pSecondFile)
	{
		pPair->pSecondFile->m_IsBinary = false;
	}

	GetApp()->OpenFilePairView(pPair);
	pPair->Dereference();
}
