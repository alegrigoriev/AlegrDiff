#pragma once
#include "SimpleCriticalSection.h"

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
	CStatic m_ItemProgressPercent;
	CStatic m_TimeLeft;
	CProgressCtrl m_Progress;
	CProgressCtrl m_ItemProgress;

	virtual INT_PTR DoModal();
	INT_PTR DoModalDelay(int Delay = 200);

	void KickDialogUpdate();
	void SetCurrentItemDone(LONGLONG Done);
	void SetTotalDataSize(LONGLONG size)
	{
		m_TotalDataSize = size;
	}
	void AddDoneItem(LONGLONG size);
	void SetNextItem(LPCTSTR Name, LONGLONG size, DWORD ItemOverhead, bool EllipsePath = false);

	UINT SignalDialogEnd(UINT Command);

	CWinThread m_Thread;
	virtual unsigned ThreadProc();
	BOOL volatile m_StopRunThread;
	HANDLE m_hThreadEvent;
	UINT m_DialogResult;

	CSimpleCriticalSection m_cs;
	CString m_CurrentItemName;
	BOOL m_bItemNameChanged;
	BOOL m_bEllipsePath;

	LONGLONG m_TotalDataSize;
	LONGLONG m_CurrentItemSize;
	LONGLONG m_ProcessedItems;
	LONGLONG m_CurrentItemDone;

	LONGLONG m_DonePerSec;
	LONGLONG m_LastDone;
	DWORD m_TickCountStarted;
	DWORD m_LastTickCount;

	int m_TotalPercentDoneShown;
	int m_ItemPercentDoneShown;

protected:
	virtual LRESULT OnKickIdle(WPARAM, LPARAM);

	static UINT AFX_CDECL _ThreadProc(PVOID arg)
	{
		return ((CProgressDialog *) arg)->ThreadProc();
	}
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnYes();
	afx_msg void OnAbort();
};
