#pragma once


// CProgressDialog dialog

class CProgressDialog : public CDialog
{
	DECLARE_DYNAMIC(CProgressDialog)

public:
	CProgressDialog(UINT id, CWnd* pParent = NULL);   // standard constructor
	virtual ~CProgressDialog();

// Dialog Data
	CStatic m_ItemName;
	CStatic m_ProgressPercent;
	virtual INT_PTR DoModal();
	INT_PTR DoModalDelay(int Delay = 200);
	void KickDialogUpdate();

	CWinThread m_Thread;
	virtual unsigned ThreadProc();
	BOOL volatile m_StopRunThread;
	HANDLE m_hThreadEvent;

	CSimpleCriticalSection m_cs;
	CString m_CurrentItemName;
	BOOL m_bItemNameChanged;

	LONGLONG m_TotalDataSize;
	LONGLONG m_ProcessedItems;
	LONGLONG m_CurrentItemDone;

protected:
	LRESULT OnKickIdle(WPARAM, LPARAM);

	static UINT AFX_CDECL _ThreadProc(PVOID arg)
	{
		return ((CProgressDialog *) arg)->ThreadProc();
	}
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnYes();
public:
	CProgressCtrl m_Progress;
};
