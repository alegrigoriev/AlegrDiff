#pragma once
#include "afxwin.h"


// CDirectoryFingerprintDlg dialog

class CDirectoryFingerprintDlg : public CDialog
{
	DECLARE_DYNAMIC(CDirectoryFingerprintDlg)

public:
	CDirectoryFingerprintDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirectoryFingerprintDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_DIR_FINGERPRINT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonBrowseDir();
	afx_msg void OnBnClickedButtonBrowseSaveFilename();
	CComboBox m_DirCombo;
	BOOL m_bIncludeDirectoryStructure;
	BOOL m_bIncludeSubdirectories;
	CString m_sDirectory;
	CComboBox m_FilenameFilterCombo;
	CString m_sFilenameFilter;
	// If TRUE, use "Ignore files" filter
	BOOL m_bIgnoreFiles;
	CComboBox m_cbIgnoreFiles;
	CString m_sIgnoreFiles;
	afx_msg void OnBnClickedCheckIgnoreFiles();
	CComboBox m_SaveFilename;
	CString m_sSaveFilename;
};
