// PreferencesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "PreferencesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog dialog


CPreferencesDialog::CPreferencesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPreferencesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPreferencesDialog)
	m_bUseBinaryFilesFilter = FALSE;
	m_bUseCppFilter = FALSE;
	m_bUseIgnoreFilter = FALSE;
	m_sBinaryFilesFilter = _T("");
	m_sCppFilesFilter = _T("");
	m_sIgnoreFilesFilter = _T("");
	m_nTabIndent = 0;
	//}}AFX_DATA_INIT
}


void CPreferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPreferencesDialog)
	DDX_Control(pDX, IDC_SPIN1, m_Spin);
	DDX_Check(pDX, IDC_CHECK_BINARY_FILES, m_bUseBinaryFilesFilter);
	DDX_Check(pDX, IDC_CHECK_C_CPP, m_bUseCppFilter);
	DDX_Check(pDX, IDC_CHECK_IGNORE, m_bUseIgnoreFilter);
	DDX_Text(pDX, IDC_EDIT_BINARY_FILES, m_sBinaryFilesFilter);
	DDX_Text(pDX, IDC_EDIT_C_CPP, m_sCppFilesFilter);
	DDX_Text(pDX, IDC_EDIT_IGNORE, m_sIgnoreFilesFilter);
	DDX_Text(pDX, IDC_EDIT_TAB_INDENT, m_nTabIndent);
	DDV_MinMaxUInt(pDX, m_nTabIndent, 0, 32);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPreferencesDialog, CDialog)
	//{{AFX_MSG_MAP(CPreferencesDialog)
	ON_BN_CLICKED(IDC_CHECK_BINARY_FILES, OnCheckBinaryFiles)
	ON_BN_CLICKED(IDC_CHECK_C_CPP, OnCheckCCpp)
	ON_BN_CLICKED(IDC_CHECK_IGNORE, OnCheckIgnore)
	ON_BN_CLICKED(IDC_BUTTON_NORMAL_FONT, OnButtonNormalFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog message handlers

void CPreferencesDialog::OnCheckBinaryFiles()
{
	CWnd * pEdit = GetDlgItem(IDC_EDIT_BINARY_FILES);
	if (pEdit)
	{
		pEdit->EnableWindow(IsDlgButtonChecked(IDC_CHECK_BINARY_FILES));
	}
}

void CPreferencesDialog::OnCheckCCpp()
{
	CWnd * pEdit = GetDlgItem(IDC_EDIT_C_CPP);
	if (pEdit)
	{
		pEdit->EnableWindow(IsDlgButtonChecked(IDC_CHECK_C_CPP));
	}
}

void CPreferencesDialog::OnCheckIgnore()
{
	CWnd * pEdit = GetDlgItem(IDC_EDIT_IGNORE);
	if (pEdit)
	{
		pEdit->EnableWindow(IsDlgButtonChecked(IDC_CHECK_IGNORE));
	}
}

BOOL CPreferencesDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Spin.SetRange(1, 32);

	CWnd * pEdit = GetDlgItem(IDC_EDIT_C_CPP);
	if (pEdit)
	{
		pEdit->EnableWindow(m_bUseCppFilter);
	}

	pEdit = GetDlgItem(IDC_EDIT_BINARY_FILES);
	if (pEdit)
	{
		pEdit->EnableWindow(m_bUseBinaryFilesFilter);
	}

	pEdit = GetDlgItem(IDC_EDIT_IGNORE);
	if (pEdit)
	{
		pEdit->EnableWindow(m_bUseIgnoreFilter);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPreferencesDialog::OnButtonNormalFont()
{
	//CThisApp * pApp = GetApp();
	CFontDialog dlg( & m_NormalLogFont,
					CF_FIXEDPITCHONLY
					| CF_INITTOLOGFONTSTRUCT
					| CF_NOVECTORFONTS
					| CF_SCREENFONTS
					| CF_EFFECTS);
	dlg.m_cf.rgbColors = m_NormalTextColor;
	if (IDOK == dlg.DoModal())
	{
	}
}
