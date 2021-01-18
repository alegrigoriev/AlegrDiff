#pragma once
#include "afxwin.h"
#include "ResizableDialog.h"
#include "PathEx.h"

// CCheckFingerprintDlg dialog

class CCheckFingerprintDlg : public CResizableDialog
{
	typedef CResizableDialog BaseClass;

public:
	CCheckFingerprintDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCheckFingerprintDlg();

	LPCTSTR GetFingerprintFilename() const noexcept
	{
		return m_sFilename;
	}
	LPCTSTR GetDirectory() const noexcept
	{
		return m_sDirectory;
	}
// Dialog Data
	enum { IDD = IDD_DIALOG_DIR_FINGERPRINT_CHECK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnMetricsChange();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedButtonBrowseFirstDir();
	afx_msg void OnBnClickedButtonBrowseOpenFilename();
	CComboBox m_cbDirectory;
	CPathEx m_sDirectory;
	CComboBox m_cbFilename;
	CString m_sFilename;
	CApplicationProfile m_Profile;
	CStringHistory m_FingerprintFilenameHistory;

public:
	virtual BOOL OnInitDialog();

	void OnUpdateOk(CCmdUI * pCmdUI);
	afx_msg void OnCbnEditchangeComboFirstDir();
	afx_msg void OnCbnSelchangeComboFirstDir();
	afx_msg void OnCbnEditchangeComboSaveFilename();
	afx_msg void OnCbnSelchangeComboSaveFilename();
};
