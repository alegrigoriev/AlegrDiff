#if !defined(AFX_SAVEFILELISTDLG_H__01A887BA_44C9_43AE_9427_ED9D1A32CFCC__INCLUDED_)
#define AFX_SAVEFILELISTDLG_H__01A887BA_44C9_43AE_9427_ED9D1A32CFCC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveFileListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaveFileListDlg dialog

class CSaveFileListDlg : public CDialog
{
// Construction
public:
	CSaveFileListDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSaveFileListDlg)
	enum { IDD = IDD_DIALOG_SAVE_FILE_LIST };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveFileListDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSaveFileListDlg)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEFILELISTDLG_H__01A887BA_44C9_43AE_9427_ED9D1A32CFCC__INCLUDED_)
