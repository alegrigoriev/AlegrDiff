#pragma once
#include "ProgressDialog.h"

// CDirectoryFingerprintCheckDlg dialog

class CDirectoryFingerprintCheckDlg : public CProgressDialog
{
	typedef CProgressDialog BaseClass;
	DECLARE_DYNAMIC(CDirectoryFingerprintCheckDlg)

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
	BOOL m_bSaveAsUnicode;
	// percents complete

	CString m_sDirectory;
	BOOL m_bIncludeSubdirectories;
	BOOL m_bIncludeDirectoryStructure;
	CString m_FingerprintFilename;
	CString m_sFilenameFilter;
	CString m_sIgnoreFiles;
	CString m_sIgnoreFolders;

	FILE * m_pFile;
	virtual unsigned ThreadProc();
public:
	virtual BOOL OnInitDialog();
	virtual INT_PTR DoModal();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
