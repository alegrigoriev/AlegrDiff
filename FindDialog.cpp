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
	: CUiUpdatedDlg(CMyFindDialog::IDD, pParent)
	, m_SearchScope(0)
{
	//{{AFX_DATA_INIT(CMyFindDialog)
	m_bCaseSensitive = FALSE;
	m_bWholeWord = FALSE;
	m_FindDown = -1;
	//}}AFX_DATA_INIT
}


void CMyFindDialog::DoDataExchange(CDataExchange* pDX)
{
	CUiUpdatedDlg::DoDataExchange(pDX);
	CThisApp * pApp = GetApp();

	//{{AFX_DATA_MAP(CMyFindDialog)
	DDX_Control(pDX, IDC_COMBO_FIND, m_FindCombo);
	DDX_Check(pDX, IDC_CHECK_CASE, m_bCaseSensitive);
	DDX_Radio(pDX, IDC_RADIO_UP, m_FindDown);
	//}}AFX_DATA_MAP
	m_FindCombo.SetExtendedUI(TRUE);

	if (-1 != m_SearchScope)
	{
		if (0 == m_SearchScope)
		{
			EnableDlgItem(IDC_CHECK_WHOLE_WORD, FALSE);
			CheckDlgButton(IDC_CHECK_WHOLE_WORD, FALSE);
		}
		else
		{
			DDX_Check(pDX, IDC_CHECK_WHOLE_WORD, m_bWholeWord);
		}
	}
	else
	{
		// disable the controls
		DDX_Check(pDX, IDC_CHECK_WHOLE_WORD, m_bWholeWord);
	}

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


BEGIN_MESSAGE_MAP(CMyFindDialog, CUiUpdatedDlg)
	//{{AFX_MSG_MAP(CMyFindDialog)
	ON_CBN_EDITCHANGE(IDC_COMBO_FIND, OnEditchangeComboFind)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_WHOLE_WORD, OnBnClickedCheckWholeWord)
	ON_CBN_SELCHANGE(IDC_COMBO_FIND, OnEditchangeComboFind)
	ON_CBN_SELENDOK(IDC_COMBO_FIND, OnEditchangeComboFind)
	ON_CBN_SELENDCANCEL(IDC_COMBO_FIND, OnEditchangeComboFind)
	ON_CBN_CLOSEUP(IDC_COMBO_FIND, OnEditchangeComboFind)

	ON_UPDATE_COMMAND_UI(IDC_CHECK_WHOLE_WORD, OnUpdateCheckWholeWord)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyFindDialog message handlers

void CMyFindDialog::OnEditchangeComboFind()
{
	NeedUpdateControls();
}

void CMyFindDialog::OnUpdateCheckWholeWord(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(0 != m_SearchScope);
	CheckDlgButton(IDC_CHECK_WHOLE_WORD, m_bWholeWord && 0 != m_SearchScope);
}

void CMyFindDialog::OnUpdateOk(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(0 != m_FindCombo.GetWindowTextLength());
}

void CMyFindDialog::OnBnClickedCheckWholeWord()
{
	m_bWholeWord = IsDlgButtonChecked(IDC_CHECK_WHOLE_WORD);
}

