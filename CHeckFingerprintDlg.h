#pragma once
#include "afxwin.h"


// CCheckFingerprintDlg dialog

class CCheckFingerprintDlg : public CDialog
{
	DECLARE_DYNAMIC(CCheckFingerprintDlg)

public:
	CCheckFingerprintDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCheckFingerprintDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_DIR_FINGERPRINT_CHECK };
	BOOL m_bNeedUpdateControls;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonBrowseFirstDir();
	afx_msg void OnBnClickedButtonBrowseOpenFilename();
	CComboBox m_cbDirectory;
	CString m_sDirectory;
	CComboBox m_cbFilename;
	CString m_sFilename;
	virtual BOOL OnInitDialog();
	CString m_sFingerprintFilenameHistory[10];

	CApplicationProfile m_Profile;
	void OnUpdateOk(CCmdUI * pCmdUI);
	LRESULT OnKickIdle(WPARAM, LPARAM);
	afx_msg void OnCbnEditchangeComboFirstDir();
	afx_msg void OnCbnSelchangeComboFirstDir();
	afx_msg void OnCbnEditchangeComboSaveFilename();
	afx_msg void OnCbnSelchangeComboSaveFilename();
};
