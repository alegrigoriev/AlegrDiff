// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "MainFrm.h"
#include "DiffFileView.h"
#include <afxpriv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, BaseClass)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DROPFILES()
	ON_COMMAND_EX(ID_VIEW_STATUS_BAR, OnBarCheckStatusBar)
	ON_COMMAND_EX(ID_VIEW_TOOLBAR, OnBarCheckToolbar)
	ON_COMMAND_EX(ID_VIEW_REBAR, OnBarCheckRebar)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CARET_POS, OnUpdateCaretPosIndicator)
	ON_MESSAGE(WM_SETMESSAGESTRING_POST, OnSetMessageStringPost)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CARET_POS,
	//ID_INDICATOR_NUM,
	//ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseClass::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if ( ! m_wndReBar.Create(this) ||
		! m_wndReBar.AddBar(&m_wndToolBar)
		//|| !m_wndReBar.AddBar(&m_wndDlgBar)
		)
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
									countof(indicators)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
							CBRS_TOOLTIPS | CBRS_FLYBY);

	CThisApp * pApp = GetApp();
	if ( ! pApp->m_bShowStatusBar)
	{
		ShowControlBar( & m_wndStatusBar, FALSE, FALSE);
	}
	if ( ! pApp->m_bShowToolbar)
	{
		ShowControlBar( & m_wndToolBar, FALSE, FALSE);
	}
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !BaseClass::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
				| WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	BaseClass::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	BaseClass::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnUpdateCaretPosIndicator(CCmdUI* pCmdUI)
{
	pCmdUI->SetText(_T("           "));
}


void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	SetActiveWindow();      // activate us first !
	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);

	CThisApp * pApp = GetApp();

	ASSERT(pApp != NULL);
	TCHAR szFileName1[_MAX_PATH] = {0};
	TCHAR szFileName2[_MAX_PATH] = {0};
	if (nFiles >=1)
	{
		::DragQueryFile(hDropInfo, 0, szFileName1, _MAX_PATH);
	}

	if (nFiles >=2)
	{
		::DragQueryFile(hDropInfo, 1, szFileName2, _MAX_PATH);
	}

	::DragFinish(hDropInfo);
	GetApp()->OpenPairOfPathnames(szFileName1, szFileName2);
}

BOOL CMainFrame::OnBarCheckStatusBar(UINT nID)
{
	if (CFrameWnd::OnBarCheck(nID))
	{
		GetApp()->m_bShowStatusBar = (0 != (GetControlBar(nID)->GetStyle() & WS_VISIBLE));
		return TRUE;
	}
	return FALSE;
}

BOOL CMainFrame::OnBarCheckToolbar(UINT nID)
{
	if (CFrameWnd::OnBarCheck(nID))
	{
		GetApp()->m_bShowToolbar = (0 != (GetControlBar(nID)->GetStyle() & WS_VISIBLE));
		return TRUE;
	}
	return FALSE;
}

BOOL CMainFrame::OnBarCheckRebar(UINT nID)
{
	if (CFrameWnd::OnBarCheck(nID))
	{
		GetApp()->m_bShowToolbar = (0 != (GetControlBar(nID)->GetStyle() & WS_VISIBLE));
		return TRUE;
	}
	return FALSE;
}

LRESULT CMainFrame::OnSetMessageStringPost(WPARAM wParam, LPARAM lParam)
{
	return SendMessage(WM_SETMESSAGESTRING, wParam, lParam);
}
