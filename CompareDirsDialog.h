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
	BOOL	    m_bIncludeSubdirs;
	CComboBox   m_cFilenameFilter;
	BOOL	m_BinaryComparision;
	//}}AFX_DATA
	BOOL    m_bUseMd5;
	CSpinButtonCtrl	m_Spin;

	CString	m_FilenameFilter;
	CString	m_sFirstDir;
	CString	m_sSecondDir;
	bool m_bAdvanced;

	// Advanced items:
	CComboBox m_cbBinaryFilesFilter;
	CComboBox m_cbCppFilesFilter;
	CComboBox m_cbIgnoreFilesFilter;
	CString	m_sBinaryFilesFilter;
	CString	m_sCppFilesFilter;
	CString	m_sIgnoreFilesFilter;
	UINT	m_nTabIndent;

	CString m_sBinaryFilterHistory[5];
	CString m_sCppFilterHistory[5];
	CString m_sIgnoreFilterHistory[10];

	CApplicationProfile m_Profile;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompareDirsDialog)
public:
	virtual int DoModal();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCompareDirsDialog)
	afx_msg void OnButtonBrowseFirstDir();
	afx_msg void OnButtonBrowseSecondDir();
	afx_msg void OnButtonAdvanced();
	afx_msg void OnCheckBinary();
	virtual BOOL OnInitDialog();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPAREDIRSDIALOG_H__146BAAAC_48C3_4A61_97E9_C783FDBD5506__INCLUDED_)
