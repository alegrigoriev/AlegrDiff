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
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
}


BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_COMMAND(IDYES, OnYes)
END_MESSAGE_MAP()

INT_PTR CProgressDialog::DoModalDelay(int Delay)
{
	m_StopRunThread = FALSE;
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
	if (m_Progress.m_hWnd != NULL && m_TotalDataSize != 0)
	{
		int Percent = int(100. * (m_ProcessedItems + m_CurrentItemDone) / m_TotalDataSize);
		if (Percent != m_Progress.GetPos())
		{
			m_Progress.SetPos(Percent);
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

