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

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_NEW, OnUpdateWindowNew)
	ON_WM_DESTROY()
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

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;
	cs.style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
				| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	CMDIChildWnd * pActive = ((CMDIFrameWnd *)AfxGetMainWnd())->MDIGetActive();

	if ((pActive == NULL && GetApp()->m_bOpenChildMaximized)
		|| (pActive != NULL && (WS_MAXIMIZE & pActive->GetStyle())))
	{
		cs.style |= WS_MAXIMIZE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

void CChildFrame::OnUpdateWindowNew(CCmdUI* pCmdUI)
{
	CWnd * pChild = GetWindow(GW_CHILD);
	pCmdUI->Enable(pChild->IsKindOf(RUNTIME_CLASS(CDiffFileView)));
}


void CChildFrame::OnDestroy()
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof wp;

	GetWindowPlacement( & wp);

	GetApp()->m_bOpenChildMaximized = 0 != (wp.flags & WPF_RESTORETOMAXIMIZED);
	//(0 != (GetStyle() & WS_MAXIMIZE));
	CMDIChildWnd::OnDestroy();
}
