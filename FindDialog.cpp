// FindDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "FindDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyFindDialog dialog


CMyFindDialog::CMyFindDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMyFindDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMyFindDialog)
	m_bCaseSensitive = FALSE;
	m_bWholeWord = FALSE;
	m_FindDown = -1;
	//}}AFX_DATA_INIT
}


void CMyFindDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CThisApp * pApp = GetApp();

	//{{AFX_DATA_MAP(CMyFindDialog)
	DDX_Control(pDX, IDC_COMBO_FIND, m_FindCombo);
	DDX_Check(pDX, IDC_CHECK_CASE, m_bCaseSensitive);
	DDX_Check(pDX, IDC_CHECK_WHOLE_WORD, m_bWholeWord);
	DDX_Radio(pDX, IDC_RADIO_UP, m_FindDown);
	//}}AFX_DATA_MAP
	if ( ! pDX->m_bSaveAndValidate)
	{
		pApp->m_FindHistory.LoadCombo( & m_FindCombo);
	}

	DDX_CBString(pDX, IDC_COMBO_FIND, m_sFindCombo);

	if (! pDX->m_bSaveAndValidate)
	{
		OnEditchangeComboFind();
	}
}


BEGIN_MESSAGE_MAP(CMyFindDialog, CDialog)
	//{{AFX_MSG_MAP(CMyFindDialog)
	ON_CBN_EDITCHANGE(IDC_COMBO_FIND, OnEditchangeComboFind)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyFindDialog message handlers

void CMyFindDialog::OnEditchangeComboFind()
{
	if (m_FindCombo.m_hWnd != NULL)
	{
		CWnd * pWnd = GetDlgItem(IDOK);
		if (pWnd)
		{
			pWnd->EnableWindow(0 != m_FindCombo.GetWindowTextLength());
		}
	}
}

