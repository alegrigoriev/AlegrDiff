#if !defined(AFX_ACCEPTDECLINEDLG_H__5D365DA0_35AE_42F5_A959_F537971E10AE__INCLUDED_)
#define AFX_ACCEPTDECLINEDLG_H__5D365DA0_35AE_42F5_A959_F537971E10AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AcceptDeclineDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAcceptDeclineDlg dialog
#include "resource.h"

class CAcceptDeclineDlg : public CDialog
{
// Construction
public:
	CAcceptDeclineDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAcceptDeclineDlg)
	enum { IDD = IDD_DIALOG_ACCEPT_OR_DECLINE_ALL };
	CStatic	m_Question;
	//}}AFX_DATA
	CString m_File1;
	CString m_File2;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAcceptDeclineDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAcceptDeclineDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACCEPTDECLINEDLG_H__5D365DA0_35AE_42F5_A959_F537971E10AE__INCLUDED_)
