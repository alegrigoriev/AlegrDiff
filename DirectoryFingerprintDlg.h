#pragma once
#include "afxwin.h"
#include "ApplicationProfile.h"

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

	BOOL m_bIncludeDirectoryStructure;
	BOOL m_bIncludeSubdirectories;
	BOOL m_bSaveAsUnicode;
	BOOL m_bNeedUpdateControls;

	CComboBox m_DirCombo;
	CString m_sDirectory;

	CApplicationProfile m_Profile;
	CComboBox m_FilenameFilterCombo;
	CString m_sFilenameFilter;

	CComboBox m_cbIgnoreFiles;
	CString m_sIgnoreFiles;
	CStringHistory m_IgnoreFilterHistory;

	CComboBox m_SaveFilename;
	CString m_sSaveFilename;
	BOOL m_bOkToOverwriteFile;
	CStringHistory m_FingerprintFilenameHistory;

	virtual BOOL OnInitDialog();

	LRESULT OnKickIdle(WPARAM, LPARAM);
	afx_msg void OnCbnEditchangeComboFirstDir();
	afx_msg void OnCbnSelchangeComboFirstDir();
	afx_msg void OnCbnEditchangeComboSaveFilename();
	afx_msg void OnCbnSelchangeComboSaveFilename();
	void OnUpdateOk(CCmdUI * pCmdUI);
protected:
	virtual void OnOK();
};
