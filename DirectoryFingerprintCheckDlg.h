#pragma once
#include "ProgressDialog.h"

// CDirectoryFingerprintCheckDlg dialog

class CDirectoryFingerprintCheckDlg : public CProgressDialog
{
	typedef CProgressDialog BaseClass;

public:
	CDirectoryFingerprintCheckDlg(class CAlegrDiffDoc * pDoc,
								LPCTSTR DirectoryToCheck,
								LPCTSTR FingerprintFilename,
								CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirectoryFingerprintCheckDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_FINGERPRINT_CHECK_PROGRESS };

protected:
	class CAlegrDiffDoc * m_pDocument;
	// percents complete

	CString m_sDirectory;
	bool    m_bIncludeSubdirectories;
	CString m_FingerprintFilename;
	CString m_sFilenameFilter;
	CString m_sIgnoreFiles;
	CString m_sIgnoreFolders;

	virtual unsigned ThreadProc();

public:
	virtual BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
