// BinaryCompareDoc.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "BinaryCompareDoc.h"


// CBinaryCompareDoc

IMPLEMENT_DYNCREATE(CBinaryCompareDoc, CDocument)

CBinaryCompareDoc::CBinaryCompareDoc()
	: m_pFilePair(NULL)
	, m_CaretPos(0)
	,m_SelectionAnchor(0)
	, m_OriginalSelectionAnchor(0)
{
}

BOOL CBinaryCompareDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
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


BEGIN_MESSAGE_MAP(CBinaryCompareDoc, CDocument)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CARET_POS, OnUpdateCaretPosIndicator)
END_MESSAGE_MAP()


// CBinaryCompareDoc diagnostics

#ifdef _DEBUG
void CBinaryCompareDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBinaryCompareDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
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
	}
	m_CaretPos = 0;
	UpdateAllViews(NULL, FileLoaded);
	SetCaretPosition(0, SetPositionCancelSelection);
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
}

void CBinaryCompareDoc::OnUpdateCaretPosIndicator(CCmdUI* pCmdUI)
{
	CString s;
	s.Format(_T("%08I64X"), m_CaretPos);
	pCmdUI->SetText(s);
}

