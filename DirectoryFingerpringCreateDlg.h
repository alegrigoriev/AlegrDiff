#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CDirectoryFingerpringCreateDlg dialog

class CDirectoryFingerpringCreateDlg : public CDialog
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
	CStatic m_Filename;
	BOOL m_bSaveAsUnicode;
	// percents complete
	CStatic m_ProgressPercent;

	CString m_sDirectory;
	BOOL m_bIncludeSubdirectories;
	BOOL m_bIncludeDirectoryStructure;
	CString m_FingerprintFilename;
	CString m_sFilenameFilter;
	CString m_sIgnoreFiles;
	FILE * m_pFile;
	virtual INT_PTR DoModal();

	CWinThread m_Thread;
	unsigned _ThreadProc();
	BOOL volatile m_StopRunThread;
	HANDLE m_hThreadEvent;

	CSimpleCriticalSection m_cs;
	CString m_CurrentFilename;
	LONGLONG m_TotalDataSize;
	LONGLONG m_ProcessedFiles;
	LONGLONG m_CurrentFileDone;
	LRESULT OnKickIdle(WPARAM, LPARAM);

	static UINT AFX_CDECL ThreadProc(PVOID arg)
	{
		return ((CDirectoryFingerpringCreateDlg *) arg)->_ThreadProc();
	}
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
	CProgressCtrl m_Progress;
};
