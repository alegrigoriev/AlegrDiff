#if !defined(AFX_PREFERENCESDIALOG_H__6B414FED_6394_4F4C_B09C_1978222E1249__INCLUDED_)
#define AFX_PREFERENCESDIALOG_H__6B414FED_6394_4F4C_B09C_1978222E1249__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreferencesDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog dialog

class CPreferencesDialog : public CDialog
{
// Construction
public:
	CPreferencesDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPreferencesDialog)
	enum { IDD = IDD_DIALOG_PREFERENCES };
	BOOL	m_bUseBinaryFilesFilter;
	BOOL	m_bUseCppFilter;
	BOOL	m_bUseIgnoreFilter;
	CString	m_sBinaryFilesFilter;
	CString	m_sCppFilesFilter;
	CString	m_sIgnoreFilesFilter;
	UINT	m_nTabIndent;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreferencesDialog)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPreferencesDialog)
	afx_msg void OnCheckBinaryFiles();
	afx_msg void OnCheckCCpp();
	afx_msg void OnCheckIgnore();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREFERENCESDIALOG_H__6B414FED_6394_4F4C_B09C_1978222E1249__INCLUDED_)
