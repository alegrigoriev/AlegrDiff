#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "ProgressDialog.h"

// CDirectoryFingerpringCreateDlg dialog

class CDirectoryFingerpringCreateDlg : public CProgressDialog
{
	DECLARE_DYNAMIC(CDirectoryFingerpringCreateDlg)

public:
	CDirectoryFingerpringCreateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirectoryFingerpringCreateDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_FINGERPRINT_CREATE_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// Name of the currently processed file
	BOOL m_bSaveAsUnicode;

	CString m_sDirectory;
	BOOL m_bIncludeSubdirectories;
	BOOL m_bIncludeDirectoryStructure;
	CString m_FingerprintFilename;
	CString m_sFilenameFilter;
	CString m_sIgnoreFiles;
	FILE * m_pFile;
	virtual INT_PTR DoModal();

	unsigned ThreadProc();

protected:
};
