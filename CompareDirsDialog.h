#if !defined(AFX_COMPAREDIRSDIALOG_H__146BAAAC_48C3_4A61_97E9_C783FDBD5506__INCLUDED_)
#define AFX_COMPAREDIRSDIALOG_H__146BAAAC_48C3_4A61_97E9_C783FDBD5506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CompareDirsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCompareDirsDialog dialog

class CCompareDirsDialog : public CDialog
{
// Construction
public:
	CCompareDirsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCompareDirsDialog)
	enum { IDD = IDD_DIALOG_COMPARE_DIRS };
	CComboBox	m_FirstDirCombo;
	CComboBox	m_SecondDirCombo;
	BOOL	m_bIncludeSubdirs;
	//}}AFX_DATA
	CString	m_sFirstDir;
	CString	m_sSecondDir;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompareDirsDialog)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCompareDirsDialog)
	afx_msg void OnButtonBrowseFirstDir();
	afx_msg void OnButtonBrowseSecondDir();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPAREDIRSDIALOG_H__146BAAAC_48C3_4A61_97E9_C783FDBD5506__INCLUDED_)
