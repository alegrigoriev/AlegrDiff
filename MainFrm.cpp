// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "MainFrm.h"
#include "DiffFileView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_NEW, OnWindowNew)
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CARET_POS, OnUpdateCaretPosIndicator)
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
	// TODO: add member initialization code here

}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
#if 0
	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME,
							CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;		// fail to create
	}
#endif

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
									sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
							CBRS_TOOLTIPS | CBRS_FLYBY);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
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
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnWindowNew()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDiffFileView * pView = dynamic_cast<CDiffFileView *>(pActiveChild->GetWindow(GW_CHILD));
	if (NULL != pView)
	{
		GetApp()->OpenFilePairView(pView->GetDocument()->GetFilePair());
	}
}

void CMainFrame::OnUpdateCaretPosIndicator(CCmdUI* pCmdUI)
{
	pCmdUI->SetText("           ");
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

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// catch Ctrl key down and up
	if (WM_KEYDOWN == pMsg->message)
	{
	}
	else if (WM_KEYUP == pMsg->message)
	{
	}

	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}
