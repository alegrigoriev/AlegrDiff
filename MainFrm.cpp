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

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DROPFILES()
	ON_COMMAND_EX(ID_VIEW_STATUS_BAR, OnBarCheckStatusBar)
	ON_COMMAND_EX(ID_VIEW_TOOLBAR, OnBarCheckToolbar)
	ON_COMMAND_EX(ID_VIEW_REBAR, OnBarCheckRebar)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
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
	m_nRotateChildIndex = 0;
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

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// catch Ctrl key down and up
	if (WM_KEYDOWN == pMsg->message)
	{
		if (VK_CONTROL == pMsg->wParam
			&& 0 == (0x40000000 & pMsg->lParam))
		{
			TRACE("Ctrl key was just pressed\n");
			m_nRotateChildIndex = 0;
		}
		else
		{
			if ((VK_TAB == pMsg->wParam || VK_F6 == pMsg->wParam)
				&& (0x8000 & GetKeyState(VK_CONTROL)))
			{
				CMDIChildWnd * pActive = MDIGetActive();
				if (NULL == pActive)
				{
					return TRUE;
				}
				CWnd * pBottom = pActive->GetWindow(GW_HWNDLAST);

				if (pBottom != pActive)
				{
					CWnd * pPlaceWnd = pActive;
					CWnd * pFrameToActivate;
					if (0x8000 & GetKeyState(VK_SHIFT))
					{
						if (m_nRotateChildIndex > 0)
						{
							for (int i = 0; i < m_nRotateChildIndex - 1; i++)
							{
								pPlaceWnd = pPlaceWnd->GetWindow(GW_HWNDNEXT);
								if (pPlaceWnd == pBottom)
								{
									break;
								}
							}
							m_nRotateChildIndex = i;
							if (pPlaceWnd == pBottom)
							{
								pFrameToActivate = pBottom;
								pPlaceWnd = pBottom->GetWindow(GW_HWNDPREV);
							}
							else
							{
								pFrameToActivate = pPlaceWnd->GetWindow(GW_HWNDNEXT);
							}
						}
						else
						{
							pFrameToActivate = pBottom;
							pPlaceWnd = pFrameToActivate;
							m_nRotateChildIndex = 1000;  // arbitrary big
						}
					}
					else
					{
						for (int i = 0; i < m_nRotateChildIndex; i++)
						{
							pPlaceWnd = pPlaceWnd->GetWindow(GW_HWNDNEXT);
							if (pPlaceWnd == pBottom)
							{
								break;
							}
						}
						m_nRotateChildIndex = i + 1;

						if (pPlaceWnd == pBottom)
						{
							pFrameToActivate = pActive->GetWindow(GW_HWNDNEXT);
							m_nRotateChildIndex = 0;
						}
						else
						{
							pFrameToActivate = pPlaceWnd->GetWindow(GW_HWNDNEXT);
						}
					}

					if (0) TRACE("m_nRotateChildIndex=%d, prev active=%X, pFrameToActivate=%X, pPlaceWnd=%X\n",
								m_nRotateChildIndex, pActive, pFrameToActivate, pPlaceWnd);

					// first activate new frame
					((CMDIChildWnd *) pFrameToActivate)->MDIActivate();
					// then move previously active window under pPlaceWnd
					pActive->SetWindowPos(pPlaceWnd, 0, 0, 0, 0,
										SWP_NOACTIVATE
										| SWP_NOMOVE
										| SWP_NOOWNERZORDER
										| SWP_NOSIZE);
				}
				return TRUE;  // message eaten
			}
		}
	}
	else if (WM_KEYUP == pMsg->message
			&& VK_CONTROL == pMsg->wParam)
	{
		m_nRotateChildIndex = 0;
	}

	return CMDIFrameWnd::PreTranslateMessage(pMsg);
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

void CMainFrame::OnDestroy()
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof wp;

	GetWindowPlacement( & wp);
	GetApp()->m_bOpenMaximized = 0 != (wp.flags & WPF_RESTORETOMAXIMIZED);

	CMDIFrameWnd::OnDestroy();
}

LRESULT CMainFrame::OnSetMessageStringPost(WPARAM wParam, LPARAM lParam)
{
	return SendMessage(WM_SETMESSAGESTRING, wParam, lParam);
}
