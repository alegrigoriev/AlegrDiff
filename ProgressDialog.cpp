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
	DDX_Control(pDX, IDC_STATIC_FILENAME, m_ItemName);
	DDX_Control(pDX, IDC_STATIC_PERCENT, m_ProgressPercent);
	if (NULL != GetDlgItem(IDC_STATIC_PERCENT_ITEM))
	{
		DDX_Control(pDX, IDC_STATIC_PERCENT_ITEM, m_ItemProgressPercent);
	}
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
	if (NULL != GetDlgItem(IDC_PROGRESS2))
	{
		DDX_Control(pDX, IDC_PROGRESS2, m_ItemProgress);
	}
}


BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_COMMAND(IDYES, OnYes)
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
			TerminateThread(m_Thread.m_hThread, -1);
		}
	}
	CloseHandle(m_Thread.m_hThread);
	m_Thread.m_hThread = NULL;

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
	UINT_PTR result = CDialog::DoModal();

	m_StopRunThread = TRUE;
	SetEvent(m_hThreadEvent);

	if (WAIT_TIMEOUT == WaitForSingleObject(m_Thread.m_hThread, 5000))
	{
		TerminateThread(m_Thread.m_hThread, -1);
	}
	CloseHandle(m_Thread.m_hThread);
	m_Thread.m_hThread = NULL;

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
		PostMessage(WM_COMMAND, IDYES, 0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

unsigned CProgressDialog::ThreadProc()
{
	if (NULL != m_hWnd)
	{
		::PostMessage(m_hWnd, WM_COMMAND, IDYES, 0);
	}
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
		m_ItemName.SetWindowText(m_CurrentItemName);
		m_bItemNameChanged = FALSE;
	}
	if (m_TotalDataSize != 0)
	{
		LONGLONG Done = m_ProcessedItems + m_CurrentItemDone;

		int Percent = int(100. * Done / m_TotalDataSize);
		if (Percent != m_TotalPercentDoneShown)
		{
			if (m_Progress.m_hWnd != NULL)
			{
				m_Progress.SetPos(Percent);
			}
			m_TotalPercentDoneShown = Percent;
		}
		// calculate time left
		DWORD TicksPassed = GetTickCount() - m_LastTickCount;
		if (TicksPassed > 5000)
		{
			if (m_LastTickCount == m_TickCountStarted)
			{
				m_DonePerSec = DWORD(float(Done - m_LastDone) / TicksPassed);

			}
			else
			{
				m_DonePerSec = DWORD(m_DonePerSec * 0.9 + 0.1 *float(Done - m_LastDone) / TicksPassed);
			}

			DWORD TicksLeft = (m_TotalDataSize - Done) * 1000. / m_DonePerSec;

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

void CProgressDialog::OnYes()
{
	//CString s;
	//s.Format(IDS_STRING_FINGERPRINT_CREATED, LPCTSTR(m_sDirectory), LPCTSTR(m_FingerprintFilename));
	OnKickIdle(0, 0);
	SetDlgItemText(IDCANCEL, _T("OK"));
}

