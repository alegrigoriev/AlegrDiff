#pragma once

// CDirectoryFingerprintCheckDlg dialog

class CAlegrDiffDoc;
class CDirectoryFingerprintCheckDlg : public CDialog
{
	DECLARE_DYNAMIC(CDirectoryFingerprintCheckDlg)

public:
	CDirectoryFingerprintCheckDlg(CAlegrDiffDoc * pDoc, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirectoryFingerprintCheckDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_FINGERPRINT_CHECK_PROGRESS };

	CAlegrDiffDoc * m_pDocument;
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
	CWinThread m_Thread;
	unsigned _ThreadProc();
	BOOL volatile m_StopRunThread;
	HANDLE m_hThreadEvent;

	CSimpleCriticalSection m_cs;
	CString m_CurrentFilename;
	BOOL volatile m_bFilenameChanged;
	LONGLONG m_TotalDataSize;
	LONGLONG m_ProcessedFiles;
	LONGLONG m_CurrentFileDone;
	LRESULT OnKickIdle(WPARAM, LPARAM);

	static UINT AFX_CDECL ThreadProc(PVOID arg)
	{
		return ((CDirectoryFingerprintCheckDlg *) arg)->_ThreadProc();
	}
	virtual BOOL OnInitDialog();
	virtual INT_PTR DoModal();
	CProgressCtrl m_Progress;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
