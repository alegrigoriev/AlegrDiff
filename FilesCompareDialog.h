#if !defined(AFX_FILESCOMPAREDIALOG_H__75C5B989_FD30_4AE9_A8A3_EF3875574486__INCLUDED_)
#define AFX_FILESCOMPAREDIALOG_H__75C5B989_FD30_4AE9_A8A3_EF3875574486__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FilesCompareDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFilesCompareDialog dialog

class CFilesCompareDialog : public CDialog
{
// Construction
public:
	CFilesCompareDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFilesCompareDialog)
	enum { IDD = IDD_DIALOG_COMPARE_FILES };
	CComboBox	m_SecondCombo;
	CComboBox	m_FirstCombo;
	BOOL	m_bBinaryFile;
	BOOL	m_bCCppFile;
	//}}AFX_DATA
	CString	m_sFirstFileName;
	CString	m_sSecondFileName;

	CString m_FileDir1;
	CString m_FileDir2;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilesCompareDialog)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFilesCompareDialog)
	afx_msg void OnButtonBrowseFirstFile();
	afx_msg void OnButtonBrowseSecondFile();
	virtual BOOL OnInitDialog();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILESCOMPAREDIALOG_H__75C5B989_FD30_4AE9_A8A3_EF3875574486__INCLUDED_)
