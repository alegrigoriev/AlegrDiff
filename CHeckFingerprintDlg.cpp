// CheckFingerprintDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "CheckFingerprintDlg.h"
#include "FolderDialog.h"

// CCheckFingerprintDlg dialog

IMPLEMENT_DYNAMIC(CCheckFingerprintDlg, CDialog)
CCheckFingerprintDlg::CCheckFingerprintDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCheckFingerprintDlg::IDD, pParent)
	, m_sDirectory(_T(""))
	, m_sFilename(_T(""))
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

		AddStringToHistory(m_sDirectory, pApp->m_RecentFolders,
							sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0], false);

		AddStringToHistory(m_sFilename, m_sFingerprintFilenameHistory,
							sizeof m_sFingerprintFilenameHistory / sizeof m_sFingerprintFilenameHistory[0], false);

		m_Profile.FlushAll();
	}
}


BEGIN_MESSAGE_MAP(CCheckFingerprintDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_DIR, OnBnClickedButtonBrowseFirstDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SAVE_FILENAME, OnBnClickedButtonBrowseOpenFilename)
END_MESSAGE_MAP()


// CCheckFingerprintDlg message handlers

void CCheckFingerprintDlg::OnBnClickedButtonBrowseFirstDir()
{
	m_cbDirectory.GetWindowText(m_sDirectory);
	CString DlgTitle;
	DlgTitle.LoadString(IDS_CHECK_FINGERPRINT_FOLDER_DLG_TITLE);

	CFolderDialog dlg(DlgTitle, m_sDirectory);

	if (IDOK == dlg.DoModal())
	{
		m_cbDirectory.SetWindowText(dlg.GetFolderPath());
	}
}

void CCheckFingerprintDlg::OnBnClickedButtonBrowseOpenFilename()
{
	CString Filter, Title;
	Filter.LoadString(IDS_STRING_FILGERPRINT_FILTER);
	Title.LoadString(IDS_STRING_OPEN_FINGERPRINT_TITLE);

	m_cbFilename.GetWindowText(m_sFilename);

	CFileDialog dlg(TRUE, _T("md5fp"), m_sFilename,
					OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
					Filter);

	dlg.m_ofn.lpstrTitle = Title;

	if (IDOK != dlg.DoModal())
	{
		return;
	}

	m_cbFilename.SetWindowText(dlg.GetPathName());
}

BOOL CCheckFingerprintDlg::OnInitDialog()
{
	CThisApp * pApp = GetApp();
	LoadHistory(m_Profile, _T("History"), _T("FingerprintFile%d"), m_sFingerprintFilenameHistory,
				sizeof m_sFingerprintFilenameHistory / sizeof m_sFingerprintFilenameHistory[0], true);

	m_sFilename = m_sFingerprintFilenameHistory[0];
	m_sDirectory = pApp->m_RecentFolders[0];

	CDialog::OnInitDialog();

	LoadHistoryCombo(m_cbDirectory, pApp->m_RecentFolders,
					sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0]);

	LoadHistoryCombo(m_cbFilename, m_sFingerprintFilenameHistory,
					sizeof m_sFingerprintFilenameHistory / sizeof m_sFingerprintFilenameHistory[0]);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
