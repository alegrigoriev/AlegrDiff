// DifferenceProgressDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DifferenceProgressDialog.h"
#include <afxpriv.h>

// CDifferenceProgressDialog dialog

IMPLEMENT_DYNAMIC(CDifferenceProgressDialog, CDialog)
CDifferenceProgressDialog::CDifferenceProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CDifferenceProgressDialog::IDD, pParent)
	, m_PercentCompleted(0)
	, m_StopSearch(FALSE)
	, m_pView(NULL)
	, m_PercentDisplayed(-1)
	, m_SearchCompleted(FALSE)
{
}

CDifferenceProgressDialog::~CDifferenceProgressDialog()
{
}

void CDifferenceProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
	DDX_Control(pDX, IDC_STATIC_PERCENT, m_Percent);
}


BEGIN_MESSAGE_MAP(CDifferenceProgressDialog, CDialog)
ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()


// CDifferenceProgressDialog message handlers

BOOL CDifferenceProgressDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Progress.SetRange(0, 100);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CDifferenceProgressDialog::OnKickIdle(WPARAM, LPARAM )
{
	// update static
	if (m_PercentDisplayed != m_PercentCompleted)
	{
		if (NULL != m_Progress.m_hWnd)
		{
			m_Progress.SetPos(m_PercentCompleted);
		}

		if (NULL != m_Percent.m_hWnd)
		{
			CString s;
			s.Format(_T("%d%%"), m_PercentCompleted);
			m_Percent.SetWindowText(s);
		}
		m_PercentDisplayed = m_PercentCompleted;
	}

	if (m_SearchCompleted)
	{
		EndDialog(IDOK);
	}
	return 0;
}

void CDifferenceProgressDialog::OnCancel()
{
	m_StopSearch = TRUE;

	CDialog::OnCancel();
}

void CDifferenceProgressDialog::OnOK()
{
	m_StopSearch = ! m_SearchCompleted;

	CDialog::OnOK();
}
