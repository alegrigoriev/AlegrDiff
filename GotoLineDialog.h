#if !defined(AFX_GOTOLINEDIALOG_H__7D6288AA_BC59_4827_AA31_70CC5CDB6F5E__INCLUDED_)
#define AFX_GOTOLINEDIALOG_H__7D6288AA_BC59_4827_AA31_70CC5CDB6F5E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GotoLineDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGotoLineDialog dialog

class CGotoLineDialog : public CDialog
{
// Construction
public:
	CGotoLineDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGotoLineDialog)
	enum { IDD = IDD_DIALOG_GOTO_LINE };
	UINT	m_LineNumber;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGotoLineDialog)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGotoLineDialog)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOTOLINEDIALOG_H__7D6288AA_BC59_4827_AA31_70CC5CDB6F5E__INCLUDED_)
