#if !defined(AFX_FINDDIALOG_H__0632F28C_90BD_4B0F_AF03_980130CEEDE2__INCLUDED_)
#define AFX_FINDDIALOG_H__0632F28C_90BD_4B0F_AF03_980130CEEDE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMyFindDialog dialog

class CMyFindDialog : public CDialog
{
// Construction
public:
	CMyFindDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMyFindDialog)
	enum { IDD = IDD_DIALOG_FIND };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyFindDialog)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMyFindDialog)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDDIALOG_H__0632F28C_90BD_4B0F_AF03_980130CEEDE2__INCLUDED_)
