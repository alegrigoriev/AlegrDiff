// CheckFingerprintDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "CheckFingerprintDlg.h"
#include "FolderDialog.h"
#include <afxpriv.h>
#include "FileDialogWithHistory.h"

// CCheckFingerprintDlg dialog

IMPLEMENT_DYNAMIC(CCheckFingerprintDlg, CDialog)
CCheckFingerprintDlg::CCheckFingerprintDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCheckFingerprintDlg::IDD, pParent)
	, m_sDirectory(_T(""))
	, m_sFilename(_T(""))
	, m_bNeedUpdateControls(TRUE)
	, m_FingerprintFilenameHistory( & m_Profile, _T("History"), _T("FingerprintFile%d"), 15)
{
}

CCheckFingerprintDlg::~CCheckFingerprintDlg()
{
}

void CCheckFingerprintDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_FIRST_DIR, m_cbDirectory);
	DDX_CBString(pDX, IDC_COMBO_FIRST_DIR, m_sDirectory);
	DDX_Control(pDX, IDC_COMBO_SAVE_FILENAME, m_cbFilename);
	DDX_CBString(pDX, IDC_COMBO_SAVE_FILENAME, m_sFilename);

	if (pDX->m_bSaveAndValidate)
	{
		CThisApp * pApp = GetApp();

		pApp->m_RecentFolders.AddString(m_sDirectory, false);
		m_FingerprintFilenameHistory.AddString(m_sFilename, false);

		m_Profile.FlushAll();
	}
}


BEGIN_MESSAGE_MAP(CCheckFingerprintDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_DIR, OnBnClickedButtonBrowseFirstDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SAVE_FILENAME, OnBnClickedButtonBrowseOpenFilename)
	ON_CBN_EDITCHANGE(IDC_COMBO_FIRST_DIR, OnCbnEditchangeComboFirstDir)
	ON_CBN_SELCHANGE(IDC_COMBO_FIRST_DIR, OnCbnSelchangeComboFirstDir)
	ON_CBN_EDITCHANGE(IDC_COMBO_SAVE_FILENAME, OnCbnEditchangeComboSaveFilename)
	ON_CBN_SELCHANGE(IDC_COMBO_SAVE_FILENAME, OnCbnSelchangeComboSaveFilename)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
END_MESSAGE_MAP()


// CCheckFingerprintDlg message handlers

void CCheckFingerprintDlg::OnBnClickedButtonBrowseFirstDir()
{
	m_cbDirectory.GetWindowText(m_sDirectory);
	CString DlgTitle;
	DlgTitle.LoadString(IDS_CHECK_FINGERPRINT_FOLDER_DLG_TITLE);

	CFolderDialog dlg(DlgTitle, m_sDirectory);

	if (IDOK != dlg.DoModal())
	{
		return;
	}
	m_cbDirectory.SetWindowText(dlg.GetFolderPath());
	m_bNeedUpdateControls = TRUE;
}

void CCheckFingerprintDlg::OnBnClickedButtonBrowseOpenFilename()
{
	CString Filter, Title;
	Filter.LoadString(IDS_STRING_FILGERPRINT_FILTER);
	Title.LoadString(IDS_STRING_OPEN_FINGERPRINT_TITLE);

	m_cbFilename.GetWindowText(m_sFilename);

	CFileDialogWithHistory dlg(TRUE, & GetApp()->m_RecentFolders, _T("md5fp"), m_sFilename,
								OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
								Filter);

	dlg.m_ofn.lpstrTitle = Title;

	if (IDOK != dlg.DoModal())
	{
		return;
	}

	m_cbFilename.SetWindowText(dlg.GetPathName());
	m_bNeedUpdateControls = TRUE;
}

BOOL CCheckFingerprintDlg::OnInitDialog()
{
	CThisApp * pApp = GetApp();

	m_FingerprintFilenameHistory.Load();

	m_sFilename = m_FingerprintFilenameHistory[0];
	m_sDirectory = pApp->m_RecentFolders[0];

	CDialog::OnInitDialog();

	pApp->m_RecentFolders.LoadCombo( & m_cbDirectory);
	m_FingerprintFilenameHistory.LoadCombo( & m_cbFilename);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCheckFingerprintDlg::OnCbnEditchangeComboFirstDir()
{
	m_bNeedUpdateControls = TRUE;
}

void CCheckFingerprintDlg::OnCbnSelchangeComboFirstDir()
{
	m_bNeedUpdateControls = TRUE;
}

void CCheckFingerprintDlg::OnCbnEditchangeComboSaveFilename()
{
	m_bNeedUpdateControls = TRUE;
}

void CCheckFingerprintDlg::OnCbnSelchangeComboSaveFilename()
{
	m_bNeedUpdateControls = TRUE;
}

LRESULT CCheckFingerprintDlg::OnKickIdle(WPARAM, LPARAM)
{
	if (m_bNeedUpdateControls)
	{
		UpdateDialogControls(this, FALSE);
	}
	m_bNeedUpdateControls = FALSE;
	return 0;
}

void CCheckFingerprintDlg::OnUpdateOk(CCmdUI * pCmdUI)
{
	CString s1, s2;
	m_cbFilename.GetWindowText(s1);
	s1.Trim();
	m_cbDirectory.GetWindowText(s2);
	s2.Trim();
	pCmdUI->Enable(! s1.IsEmpty() && ! s2.IsEmpty());
}
