// BinaryCompareDoc.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "BinaryCompareDoc.h"


// CBinaryCompareDoc

IMPLEMENT_DYNCREATE(CBinaryCompareDoc, CDocument)

CBinaryCompareDoc::CBinaryCompareDoc()
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
