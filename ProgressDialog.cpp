// ProgressDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SimpleCriticalSection.h"
#include "resource.h"
#include "ProgressDialog.h"
#include <afxpriv.h>

// CProgressDialog dialog

IMPLEMENT_DYNAMIC(CProgressDialog, CDialog)
CProgressDialog::CProgressDialog(UINT id, CWnd* pParent /*=NULL*/)
	: CDialog(id, pParent)
	, m_StopRunThread(FALSE)
	, m_Thread(_ThreadProc, this)
	, m_bItemNameChanged(TRUE)
	, m_TotalDataSize(0)
	, m_ProcessedItems(0)
	, m_CurrentItemDone(0)
	, m_CurrentItemSize(0)
	, m_TotalPercentDoneShown(-1)
	, m_ItemPercentDoneShown(-1)
	, m_TickCountStarted(0)
	, m_LastTickCount(0)
	, m_LastDone(0)
	, m_DialogResult(-1)
	, m_hThreadEvent(CreateEvent(NULL, FALSE, FALSE, NULL))
{
	m_Thread.m_bAutoDelete = false;
}

CProgressDialog::~CProgressDialog()
{
	if (m_hThreadEvent)
	{
		CloseHandle(m_hThreadEvent);
	}
}

void CProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	if (NULL != GetDlgItem(IDC_STATIC_FILENAME))
	{
		DDX_Control(pDX, IDC_STATIC_FILENAME, m_ItemName);
	}
	if (NULL != GetDlgItem(IDC_STATIC_PERCENT))
	{
		DDX_Control(pDX, IDC_STATIC_PERCENT, m_ProgressPercent);
	}

	if (NULL != GetDlgItem(IDC_STATIC_PERCENT_ITEM))
	{
		DDX_Control(pDX, IDC_STATIC_PERCENT_ITEM, m_ItemProgressPercent);
	}

	if (NULL != GetDlgItem(IDC_PROGRESS1))
	{
		DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
	}

	if (NULL != GetDlgItem(IDC_PROGRESS2))
	{
		DDX_Control(pDX, IDC_PROGRESS2, m_ItemProgress);
	}

	if (NULL != GetDlgItem(IDC_TIME_LEFT))
	{
		DDX_Control(pDX, IDC_TIME_LEFT, m_TimeLeft);
	}
}


BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_COMMAND(IDYES, OnYes)
	ON_COMMAND(IDABORT, OnAbort)
END_MESSAGE_MAP()

INT_PTR CProgressDialog::DoModalDelay(int Delay)
{
	m_StopRunThread = FALSE;

	m_TickCountStarted = GetTickCount();
	m_LastTickCount = m_TickCountStarted;

	m_Thread.CreateThread(0, 0x10000);
	INT_PTR result = IDOK;

	if (NULL == m_Thread.m_hThread)
	{
		return -1;
	}

	if (WAIT_TIMEOUT == WaitForSingleObject(m_Thread.m_hThread, Delay))
	{
		result = CDialog::DoModal();

		m_StopRunThread = TRUE;
		SetEvent(m_hThreadEvent);

		if (WAIT_TIMEOUT == WaitForSingleObject(m_Thread.m_hThread, 5000))
		{
			TerminateThread(m_Thread.m_hThread, (unsigned)-1);
		}
	}
	CloseHandle(m_Thread.m_hThread);
	m_Thread.m_hThread = NULL;

	if (m_DialogResult != -1)
	{
		return m_DialogResult;
	}

	return result;
}

INT_PTR CProgressDialog::DoModal()
{
	m_StopRunThread = FALSE;

	m_TickCountStarted = GetTickCount();
	m_LastTickCount = m_TickCountStarted;

	m_Thread.CreateThread(0, 0x10000);

	if (NULL == m_Thread.m_hThread)
	{
		return -1;
	}
	INT_PTR result = CDialog::DoModal();

	m_StopRunThread = TRUE;
	SetEvent(m_hThreadEvent);

	if (WAIT_TIMEOUT == WaitForSingleObject(m_Thread.m_hThread, 5000))
	{
		TerminateThread(m_Thread.m_hThread, (unsigned)-1);
	}
	CloseHandle(m_Thread.m_hThread);
	m_Thread.m_hThread = NULL;

	if (m_DialogResult != -1)
	{
		return m_DialogResult;
	}

	return result;
}


// CProgressDialog message handlers

BOOL CProgressDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	TRACE("CProgressDialog::OnInitDialog()\n");

	m_Progress.SetRange(0, 100);
	// if the thread is already completed, or not even started, close the dialog
	if (WAIT_TIMEOUT != WaitForSingleObject(m_Thread.m_hThread, 0))
	{
		if (-1 == m_DialogResult)
		{
			m_DialogResult = IDYES;
		}
		SignalDialogEnd(m_DialogResult);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

unsigned CProgressDialog::ThreadProc()
{
	SignalDialogEnd(IDYES);
	return 0;
}

void CProgressDialog::KickDialogUpdate()
{
	if (NULL != m_hWnd)
	{
		::PostMessage(m_hWnd, WM_KICKIDLE, 0, 0);
	}
}

LRESULT CProgressDialog::OnKickIdle(WPARAM, LPARAM)
{

	CSimpleCriticalSectionLock lock(m_cs);

	if (m_ItemName.m_hWnd != NULL && m_bItemNameChanged)
	{
		if (m_bEllipsePath)
		{
			m_ItemName.ModifyStyle(0, SS_PATHELLIPSIS);
		}
		else
		{
			m_ItemName.ModifyStyle(SS_PATHELLIPSIS, 0);
		}
		m_ItemName.SetWindowText(m_CurrentItemName);
		m_bItemNameChanged = FALSE;
	}
	if (! m_StopRunThread
		&& m_TotalDataSize != 0)
	{
		LONGLONG Done = m_ProcessedItems + m_CurrentItemDone;

		int Percent = int(100. * Done / m_TotalDataSize);
		if (Percent != m_TotalPercentDoneShown)
		{
			if (m_Progress.m_hWnd != NULL)
			{
				m_Progress.SetPos(Percent);
			}
			if (m_ProgressPercent.m_hWnd != NULL)
			{
				CString s;
				s.Format(IDS_STRING_PERCENTS_DONE, Percent);
				m_ProgressPercent.SetWindowText(s);
			}
			m_TotalPercentDoneShown = Percent;
		}
		// calculate time left
		DWORD TicksPassed = GetTickCount() - m_LastTickCount;
		if (TicksPassed > 2000 && m_LastTickCount != m_TickCountStarted
			|| TicksPassed > 5000)
		{
			if (m_LastTickCount == m_TickCountStarted)
			{
				// just started, don't use smoothing
				m_DonePerSec = DWORD(double(Done - m_LastDone) * 1000. / TicksPassed);
				//TRACE("Just started, Done per sec = %I64d\n", m_DonePerSec);
			}
			else
			{
				m_DonePerSec = DWORD(m_DonePerSec * 0.9 + 100. * double(Done - m_LastDone) / TicksPassed);
				//TRACE("Done per sec = %I64d\n", m_DonePerSec);
			}

			if (0 != m_DonePerSec
				&& NULL != m_TimeLeft.m_hWnd)
			{
				CString s;
				DWORD SecondsLeft;
				if (Done <= m_TotalDataSize)
				{
					SecondsLeft = DWORD((m_TotalDataSize - Done) / m_DonePerSec);

					if (SecondsLeft < 60)
					{
						s.Format(IDS_TIME_LEFT_SECONDS, SecondsLeft);
					}
					else
					{
						s.Format(IDS_TIME_LEFT_MINUTES, SecondsLeft / 60);
					}
					m_TimeLeft.SetWindowText(s);
				}
			}

			m_LastDone = Done;
			m_LastTickCount += TicksPassed;
		}
	}
	if (m_ItemProgress.m_hWnd != NULL && m_CurrentItemSize != 0)
	{
		int Percent = int(100. * m_CurrentItemDone / m_CurrentItemSize);
		if (Percent != m_ItemPercentDoneShown)
		{
			m_ItemPercentDoneShown = Percent;
			m_ItemProgress.SetPos(Percent);
		}
	}
	return 0;
}

void CProgressDialog::SetCurrentItemDone(LONGLONG Done)
{
	{
		CSimpleCriticalSectionLock lock(m_cs);
		m_CurrentItemDone = Done;
	}
	// kick the update if more than 100 ms passed
	if (0 != m_TotalDataSize
		&& int(100. * (m_ProcessedItems + Done) / m_TotalDataSize)
		!= m_TotalPercentDoneShown)
	{
		KickDialogUpdate();
	}
}

void CProgressDialog::AddDoneItem(LONGLONG size)
{
	CSimpleCriticalSectionLock lock(m_cs);
	m_ProcessedItems += size;
}

void CProgressDialog::SetNextItem(LPCTSTR Name, LONGLONG size, DWORD ItemOverhead, bool EllipsePath)
{
	{
		CSimpleCriticalSectionLock lock(m_cs);

		m_CurrentItemName = Name;
		m_bItemNameChanged = TRUE;
		m_bEllipsePath = EllipsePath;
		m_CurrentItemDone = 0;
		m_CurrentItemSize = size;
		m_ProcessedItems += ItemOverhead;
	}
	KickDialogUpdate();
}

void CProgressDialog::OnYes()
{
	m_StopRunThread = TRUE;
	OnKickIdle(0, 0);

	if (NULL != m_Progress.m_hWnd)
	{
		m_Progress.ShowWindow(SW_HIDE);
	}
	if (NULL != m_ItemProgress.m_hWnd)
	{
		m_ItemProgress.ShowWindow(SW_HIDE);
	}
	if (NULL != m_TimeLeft.m_hWnd)
	{
		m_TimeLeft.ShowWindow(SW_HIDE);
	}
	if (NULL != m_ProgressPercent.m_hWnd)
	{
		m_ProgressPercent.ShowWindow(SW_HIDE);
	}
	if (NULL != m_ItemProgressPercent.m_hWnd)
	{
		m_ItemProgressPercent.ShowWindow(SW_HIDE);
	}

	CWnd * pCancel = GetDlgItem(IDCANCEL);
	if (pCancel)
	{
		pCancel->SetDlgCtrlID(IDOK);
		pCancel->SetWindowText(_T("OK"));
	}
}

void CProgressDialog::OnAbort()
{
	m_StopRunThread = TRUE;
	OnKickIdle(0, 0);
	if (NULL != m_Progress.m_hWnd)
	{
		m_Progress.ShowWindow(SW_HIDE);
	}
	if (NULL != m_ItemProgress.m_hWnd)
	{
		m_ItemProgress.ShowWindow(SW_HIDE);
	}

	if (NULL != m_TimeLeft.m_hWnd)
	{
		m_TimeLeft.ShowWindow(SW_HIDE);
	}

	CWnd * pCancel = GetDlgItem(IDCANCEL);
	if (pCancel)
	{
		pCancel->SetDlgCtrlID(IDABORT);
		pCancel->SetWindowText(_T("OK"));
	}
	else
	{
		EndDialog(IDABORT);
	}
}

INT_PTR CProgressDialog::SignalDialogEnd(INT_PTR Command)
{
	m_DialogResult = Command;
	if (NULL != m_hWnd)
	{
		::PostMessage(m_hWnd, WM_COMMAND, Command, 0);
	}
	return Command;
}

