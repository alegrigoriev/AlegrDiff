// DifferenceProgressDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DifferenceProgressDialog.h"

// CDifferenceProgressDialog dialog

IMPLEMENT_DYNAMIC(CDifferenceProgressDialog, CProgressDialog)
CDifferenceProgressDialog::CDifferenceProgressDialog(CWnd* pParent /*=NULL*/)
	: CProgressDialog(CDifferenceProgressDialog::IDD, pParent)
	, m_pDoc(NULL)
	, BeginAddr(0)
	, EndAddr(0)
{
}

CDifferenceProgressDialog::~CDifferenceProgressDialog()
{
}

void CDifferenceProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CProgressDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDifferenceProgressDialog, CProgressDialog)
END_MESSAGE_MAP()


// CDifferenceProgressDialog message handlers

BOOL CDifferenceProgressDialog::OnInitDialog()
{
	CProgressDialog::OnInitDialog();
	m_TotalDataSize = EndAddr - BeginAddr;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

unsigned CDifferenceProgressDialog::ThreadProc()
{
	m_pDoc->FindDataProc(this);
	if (NULL != m_hWnd)
	{
		::PostMessage(m_hWnd, WM_COMMAND, IDOK, 0);
	}
	return 0;
}

