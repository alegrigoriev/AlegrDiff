// AcceptDeclineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AcceptDeclineDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAcceptDeclineDlg dialog


CAcceptDeclineDlg::CAcceptDeclineDlg(CWnd* pParent /*=NULL*/) noexcept
	: CDialog(CAcceptDeclineDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAcceptDeclineDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAcceptDeclineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAcceptDeclineDlg)
	DDX_Control(pDX, IDC_STATIC_QEUSTION, m_Question);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAcceptDeclineDlg, CDialog)
	//{{AFX_MSG_MAP(CAcceptDeclineDlg)
	ON_BN_CLICKED(IDNO, OnNo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAcceptDeclineDlg message handlers

BOOL CAcceptDeclineDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CString s;
	m_Question.GetWindowText(s);

	CString s1;
	s1.Format(s, LPCTSTR(m_File1), LPCTSTR(m_File2));

	m_Question.SetWindowText(s1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAcceptDeclineDlg::OnNo()
{
	EndDialog(IDNO);
}
