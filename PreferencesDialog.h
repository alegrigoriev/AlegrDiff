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
	CSpinButtonCtrl	m_Spin;
	BOOL	m_bUseBinaryFilesFilter;
	BOOL	m_bUseCppFilter;
	BOOL	m_bUseIgnoreFilter;
	CString	m_sBinaryFilesFilter;
	CString	m_sCppFilesFilter;
	CString	m_sIgnoreFilesFilter;
	UINT	m_nTabIndent;
	BOOL	m_AutoReloadChangedFiles;
	//}}AFX_DATA

	LOGFONT m_NormalLogFont;
	CFont m_NormalFont;
	DWORD m_NormalTextColor;

	LOGFONT m_ErasedLogFont;
	CFont m_ErasedFont;
	DWORD m_ErasedTextColor;

	LOGFONT m_AddedLogFont;
	CFont m_AddedFont;
	DWORD m_AddedTextColor;

	int m_FontPointSize;

	bool m_bFontChanged;
	void FontChanged();
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
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonNormalFont();
	afx_msg void OnButtonInsertedFont();
	afx_msg void OnButtonErasedFont();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREFERENCESDIALOG_H__6B414FED_6394_4F4C_B09C_1978222E1249__INCLUDED_)
