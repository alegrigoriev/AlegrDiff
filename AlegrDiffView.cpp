// AlegrDiffView.cpp : implementation of the CAlegrDiffView class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "AlegrDiffDoc.h"
#include "AlegrDiffView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView

IMPLEMENT_DYNCREATE(CAlegrDiffView, CView)

BEGIN_MESSAGE_MAP(CAlegrDiffView, CView)
	//{{AFX_MSG_MAP(CAlegrDiffView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView construction/destruction

CAlegrDiffView::CAlegrDiffView()
{
	// TODO: add construction code here

}

CAlegrDiffView::~CAlegrDiffView()
{
}

BOOL CAlegrDiffView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView drawing

void CAlegrDiffView::OnDraw(CDC* pDC)
{
	CAlegrDiffDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView printing

BOOL CAlegrDiffView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CAlegrDiffView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CAlegrDiffView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView diagnostics

#ifdef _DEBUG
void CAlegrDiffView::AssertValid() const
{
	CView::AssertValid();
}

void CAlegrDiffView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CAlegrDiffDoc* CAlegrDiffView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAlegrDiffDoc)));
	return (CAlegrDiffDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView message handlers
