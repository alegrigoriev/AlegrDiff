// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "ChildFrm.h"
#include "DiffFileView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, BaseClass)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_NEW, OnUpdateWindowNew)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here

}

CChildFrame::~CChildFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	BaseClass::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	BaseClass::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

void CChildFrame::OnUpdateWindowNew(CCmdUI* pCmdUI)
{
	CWnd * pChild = GetWindow(GW_CHILD);
	pCmdUI->Enable(pChild->IsKindOf(RUNTIME_CLASS(CDiffFileView)));
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !BaseClass::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style |= FWS_ADDTOTITLE;

	return TRUE;
}
