// BinaryCompareView.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "BinaryCompareView.h"
#include "BinaryCompareDoc.h"

// CBinaryCompareView

IMPLEMENT_DYNCREATE(CBinaryCompareView, CView)

CBinaryCompareView::CBinaryCompareView()
{
}

CBinaryCompareView::~CBinaryCompareView()
{
}

BEGIN_MESSAGE_MAP(CBinaryCompareView, CView)
END_MESSAGE_MAP()


// CBinaryCompareView drawing

void CBinaryCompareView::OnDraw(CDC* pDC)
{
	CBinaryCompareDoc* pDoc = GetDocument();
	// TODO: add draw code here
}


// CBinaryCompareView diagnostics

#ifdef _DEBUG
void CBinaryCompareView::AssertValid() const
{
	CView::AssertValid();
}

void CBinaryCompareView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
CBinaryCompareDoc * CBinaryCompareView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBinaryCompareDoc)));
	return (CBinaryCompareDoc*)m_pDocument;
}
#endif //_DEBUG


// CBinaryCompareView message handlers
