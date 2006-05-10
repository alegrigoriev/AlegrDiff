// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__0407E411_96C2_4B96_911A_2406C9492BC2__INCLUDED_)
#define AFX_MAINFRM_H__0407E411_96C2_4B96_911A_2406C9492BC2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MainFrameEx.h"
#include "MessageBoxSynch.h"

typedef FrameExParameters<MainFrameRememberMaximized
						| MainFrameRememberSize
						| MainFrameNeatCtrlTab,
						DialogProxyWnd<CMDIFrameWnd> >

	MainFrameExParameters;

class CMainFrame : public CMainFrameExT<MainFrameExParameters>
{
	typedef CMainFrameExT<MainFrameExParameters> BaseClass;
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
	CDialogBar      m_wndDlgBar;

// Generated message map functions
protected:
	void OnUpdateCaretPosIndicator(CCmdUI* pCmdUI);
	afx_msg LRESULT OnSetMessageStringPost(WPARAM wParam, LPARAM lParam);
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg BOOL OnBarCheckStatusBar(UINT nID);
	afx_msg BOOL OnBarCheckToolbar(UINT nID);
	afx_msg BOOL OnBarCheckRebar(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__0407E411_96C2_4B96_911A_2406C9492BC2__INCLUDED_)
