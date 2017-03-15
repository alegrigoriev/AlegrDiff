#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "ProgressDialog.h"

// CDirectoryFingerpringCreateDlg dialog

class CDirectoryFingerpringCreateDlg : public CProgressDialog
{
	typedef CProgressDialog BaseClass;
public:
	CDirectoryFingerpringCreateDlg(
									LPCTSTR sDirectory,
									LPCTSTR sFingerprintFilename,
									LPCTSTR sFilenameFilter,
									LPCTSTR sIgnoreFiles,
									LPCTSTR sIgnoreFolders,
									BOOL bIncludeSubdirectories,
									BOOL bIncludeDirectoryStructure,
									CWnd* pParent = NULL);
	virtual ~CDirectoryFingerpringCreateDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_FINGERPRINT_CREATE_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// Name of the currently processed file

	CString m_sDirectory;
	BOOL m_bIncludeSubdirectories;
	BOOL m_bIncludeDirectoryStructure;
	CString m_FingerprintFilename;
	CString m_sFilenameFilter;
	CString m_sIgnoreFiles;
	CString m_sIgnoreFolders;

	FILE * m_pFile;

	virtual INT_PTR DoModal();

	virtual unsigned ThreadProc();
	virtual void OnCancel();

protected:
};
