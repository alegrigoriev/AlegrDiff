#pragma once


// CDirectoryFingerprintCheckDlg dialog

class CDirectoryFingerprintCheckDlg : public CDialog
{
	DECLARE_DYNAMIC(CDirectoryFingerprintCheckDlg)

public:
	CDirectoryFingerprintCheckDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirectoryFingerprintCheckDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_FINGERPRINT_CHECK_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
