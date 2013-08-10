#if !defined(AFX_COMPAREDIRSDIALOG_H__146BAAAC_48C3_4A61_97E9_C783FDBD5506__INCLUDED_)
#define AFX_COMPAREDIRSDIALOG_H__146BAAAC_48C3_4A61_97E9_C783FDBD5506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CompareDirsDialog.h : header file
//
#include "ApplicationProfile.h"
#include "ResizableDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CCompareDirsDialog dialog

class CCompareDirsDialog : public CResizableDialog
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
	CComboBox m_cbIgnoreFoldersFilter;

	CString	m_sBinaryFilesFilter;
	CString	m_sCppFilesFilter;
	CString	m_sIgnoreFilesFilter;
	CString	m_sIgnoreFoldersFilter;

	UINT	m_nTabIndent;

	CApplicationProfile m_Profile;

	CStringHistory m_BinaryFilterHistory;
	CStringHistory m_CppFilterHistory;
	CStringHistory m_IgnoreFilterHistory;
	CStringHistory m_IgnoreFoldersHistory;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompareDirsDialog)
public:
	virtual INT_PTR DoModal();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	static int m_PrevWidth;

	virtual void OnMetricsChange();
	// Generated message map functions
	//{{AFX_MSG(CCompareDirsDialog)
	afx_msg void OnButtonBrowseFirstDir();
	afx_msg void OnButtonBrowseSecondDir();
	afx_msg void OnButtonAdvanced();
	afx_msg void OnCheckBinary();
	afx_msg void OnCheckIncludeSubdirs();
	virtual BOOL OnInitDialog();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnUpdateIgnoreDirs(CCmdUI * pCmdUI);
	afx_msg void OnUpdateEditBinaryFiles(CCmdUI * pCmdUI);
	afx_msg void OnUpdateEditCCpp(CCmdUI * pCmdUI);
	afx_msg void OnUpdateTabIndent(CCmdUI * pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPAREDIRSDIALOG_H__146BAAAC_48C3_4A61_97E9_C783FDBD5506__INCLUDED_)
