// BinaryCompareDoc.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "BinaryCompareDoc.h"


// CBinaryCompareDoc

IMPLEMENT_DYNCREATE(CBinaryCompareDoc, CDocument)

CBinaryCompareDoc::CBinaryCompareDoc()
	: m_pFilePair(NULL)
{
}

BOOL CBinaryCompareDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
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
	pPair->Reference();
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

	if (0 != (flags & SetPositionCancelSelection))
	{
		m_SelectionAnchor = m_CaretPos;
		m_OriginalSelectionAnchor = m_CaretPos;  // for word mode selection
		// if canceling selection, check for word mode reset
	}
	UpdateAllViews(NULL, CaretPositionChanged, NULL);
}

