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
	//m_sFindCombo = _T("");
}


void CMyFindDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CThisApp * pApp = GetApp();
	int i;
	//{{AFX_DATA_MAP(CMyFindDialog)
	DDX_Control(pDX, IDC_COMBO_FIND, m_FindCombo);
	DDX_Check(pDX, IDC_CHECK_CASE, m_bCaseSensitive);
	DDX_Check(pDX, IDC_CHECK_WHOLE_WORD, m_bWholeWord);
	DDX_Radio(pDX, IDC_RADIO_UP, m_FindDown);
	//}}AFX_DATA_MAP
	if ( ! pDX->m_bSaveAndValidate)
	{
		// load from registry
		for (i = 0; i < sizeof m_sHistory / sizeof m_sHistory[0]; i++)
		{
			CString s;
			s.Format("find%d", i);
			pApp->Profile.AddItem(_T("History"), s, m_sHistory[i]);
			if ( ! m_sHistory[i].IsEmpty())
			{
				m_FindCombo.AddString(m_sHistory[i]);
			}
		}
		//if (m_sFindCombo.IsEmpty())
		{
			//m_sFindCombo = m_sHistory[0];
		}
		OnEditchangeComboFind();
	}

	DDX_CBString(pDX, IDC_COMBO_FIND, m_sFindCombo);

	if (pDX->m_bSaveAndValidate)
	{
		// remove those that match the currently selected dirs
		int j;
		for (i = 0, j = 0; i < sizeof m_sHistory / sizeof m_sHistory[0]; i++)
		{
			if (0 == m_sFindCombo.Compare(m_sHistory[i]))
			{
				continue;
			}
			if (i != j)
			{
				m_sHistory[j] = m_sHistory[i];
			}
			j++;
		}
		// remove last dir from the list
		for (i = (sizeof m_sHistory / sizeof m_sHistory[0]) - 1; i >= 1; i--)
		{
			m_sHistory[i] = m_sHistory[i - 1];
		}
		m_sHistory[0] = m_sFindCombo;
		pApp->Profile.UnloadSection(_T("History"));
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

void CMyFindDialog::OnCancel()
{
	GetApp()->Profile.RemoveSection(_T("History"));
	CDialog::OnCancel();
}
