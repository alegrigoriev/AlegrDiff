// DirectoryFingerprintDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DirectoryFingerprintDlg.h"
#include "FolderDialog.h"
#include <afxpriv.h>

// CDirectoryFingerprintDlg dialog

IMPLEMENT_DYNAMIC(CDirectoryFingerprintDlg, CDialog)
CDirectoryFingerprintDlg::CDirectoryFingerprintDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDirectoryFingerprintDlg::IDD, pParent)
	, m_bIncludeDirectoryStructure(FALSE)
	, m_bIncludeSubdirectories(FALSE)
	, m_sDirectory(_T(""))
	, m_sFilenameFilter(_T(""))
	, m_sIgnoreFiles(_T(""))
	, m_sSaveFilename(_T(""))
	, m_bNeedUpdateControls(TRUE)
	, m_bOkToOverwriteFile(FALSE)
	, m_bSaveAsUnicode(FALSE)
{
}

CDirectoryFingerprintDlg::~CDirectoryFingerprintDlg()
{
}

void CDirectoryFingerprintDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_FIRST_DIR, m_DirCombo);
	DDX_Check(pDX, IDC_CHECK_INCLUDE_DIRECTORY_STRUCTURE, m_bIncludeDirectoryStructure);
	DDX_Check(pDX, IDC_CHECK_INCLUDE_SUBDIRS, m_bIncludeSubdirectories);
	DDX_CBString(pDX, IDC_COMBO_FIRST_DIR, m_sDirectory);
	DDX_Control(pDX, IDC_EDITFILENAME_FILTER, m_FilenameFilterCombo);
	DDX_Text(pDX, IDC_EDITFILENAME_FILTER, m_sFilenameFilter);
	DDX_Control(pDX, IDC_COMBO_IGNORE_FILES, m_cbIgnoreFiles);
	DDX_CBString(pDX, IDC_COMBO_IGNORE_FILES, m_sIgnoreFiles);
	DDX_Control(pDX, IDC_COMBO_SAVE_FILENAME, m_SaveFilename);
	DDX_CBString(pDX, IDC_COMBO_SAVE_FILENAME, m_sSaveFilename);
	DDX_Check(pDX, IDC_CHECK_SAVE_AS_UNICODE, m_bSaveAsUnicode);

	if (pDX->m_bSaveAndValidate)
	{
		CThisApp * pApp = GetApp();

		AddStringToHistory(m_sDirectory, pApp->m_RecentFolders,
							countof(pApp->m_RecentFolders), false);

		AddStringToHistory(m_sIgnoreFiles, m_sIgnoreFilterHistory,
							countof(m_sIgnoreFilterHistory), false);

		AddStringToHistory(m_sFilenameFilter, pApp->m_sFilters,
							countof(pApp->m_sFilters), false);

		AddStringToHistory(m_sSaveFilename, m_sFingerprintFilenameHistory,
							countof(m_sFingerprintFilenameHistory), false);

		m_Profile.FlushAll();
	}
}


BEGIN_MESSAGE_MAP(CDirectoryFingerprintDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_DIR, OnBnClickedButtonBrowseDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SAVE_FILENAME, OnBnClickedButtonBrowseSaveFilename)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_CBN_EDITCHANGE(IDC_COMBO_FIRST_DIR, OnCbnEditchangeComboFirstDir)
	ON_CBN_SELCHANGE(IDC_COMBO_FIRST_DIR, OnCbnSelchangeComboFirstDir)
	ON_CBN_EDITCHANGE(IDC_COMBO_SAVE_FILENAME, OnCbnEditchangeComboSaveFilename)
	ON_CBN_SELCHANGE(IDC_COMBO_SAVE_FILENAME, OnCbnSelchangeComboSaveFilename)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
END_MESSAGE_MAP()


// CDirectoryFingerprintDlg message handlers

void CDirectoryFingerprintDlg::OnBnClickedButtonBrowseDir()
{
	m_DirCombo.GetWindowText(m_sDirectory);
	CString DlgTitle;
	DlgTitle.LoadString(IDS_STRING_DIR_FINGERPRINT);
	CFolderDialog dlg(DlgTitle, m_sDirectory);

	if (IDOK == dlg.DoModal())
	{
		m_DirCombo.SetWindowText(dlg.GetFolderPath());
		m_bNeedUpdateControls = TRUE;
	}
}

void CDirectoryFingerprintDlg::OnBnClickedButtonBrowseSaveFilename()
{
	CString Filter, Title;
	Filter.LoadString(IDS_STRING_FILGERPRINT_FILTER);
	Title.LoadString(IDS_STRING_FINGERPRINT_TITLE);

	m_SaveFilename.GetWindowText(m_sSaveFilename);

	CFileDialog dlg(FALSE, _T("md5fp"), m_sSaveFilename,
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					Filter);
	dlg.m_ofn.lpstrTitle = Title;

	if (IDOK != dlg.DoModal())
	{
		return;
	}
	m_SaveFilename.SetWindowText(dlg.GetPathName());

	m_bOkToOverwriteFile = TRUE;
	m_bNeedUpdateControls = TRUE;
}

BOOL CDirectoryFingerprintDlg::OnInitDialog()
{
	CThisApp * pApp = GetApp();
	LoadHistory(m_Profile, _T("History"), _T("FingerprintFile%d"), m_sFingerprintFilenameHistory,
				countof(m_sFingerprintFilenameHistory), true);

	LoadHistory(m_Profile, _T("History"), _T("IgnoreFiles%d"), m_sIgnoreFilterHistory,
				countof(m_sIgnoreFilterHistory), true);

	m_Profile.AddBoolItem(_T("Settings"), _T("IncludeDirsToFingerprint"), m_bIncludeDirectoryStructure, TRUE);
	m_Profile.AddBoolItem(_T("Settings"), _T("SaveFingerprintAsUnicode"), m_bSaveAsUnicode, FALSE);

	m_sSaveFilename = m_sFingerprintFilenameHistory[0];
	m_sIgnoreFiles = m_sIgnoreFilterHistory[0];
	m_sDirectory = pApp->m_RecentFolders[0];

	CDialog::OnInitDialog();

	LoadHistoryCombo(m_DirCombo, pApp->m_RecentFolders,
					countof(pApp->m_RecentFolders));

	LoadHistoryCombo(m_FilenameFilterCombo, pApp->m_sFilters,
					countof(pApp->m_sFilters));

	LoadHistoryCombo(m_cbIgnoreFiles, m_sIgnoreFilterHistory,
					countof(m_sIgnoreFilterHistory));

	LoadHistoryCombo(m_SaveFilename, m_sFingerprintFilenameHistory,
					countof(m_sFingerprintFilenameHistory));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CDirectoryFingerprintDlg::OnKickIdle(WPARAM, LPARAM)
{
	if (m_bNeedUpdateControls)
	{
		UpdateDialogControls(this, FALSE);
	}
	m_bNeedUpdateControls = FALSE;
	return 0;
}


void CDirectoryFingerprintDlg::OnCbnEditchangeComboFirstDir()
{
	m_bNeedUpdateControls = TRUE;
}

void CDirectoryFingerprintDlg::OnCbnSelchangeComboFirstDir()
{
	m_bNeedUpdateControls = TRUE;
}

void CDirectoryFingerprintDlg::OnCbnEditchangeComboSaveFilename()
{
	m_bOkToOverwriteFile = FALSE;
	m_bNeedUpdateControls = TRUE;
}

void CDirectoryFingerprintDlg::OnCbnSelchangeComboSaveFilename()
{
	m_bOkToOverwriteFile = FALSE;
	m_bNeedUpdateControls = TRUE;
}

void CDirectoryFingerprintDlg::OnUpdateOk(CCmdUI * pCmdUI)
{
	CString s1, s2;
	m_SaveFilename.GetWindowText(s1);
	s1.Trim();
	m_DirCombo.GetWindowText(s2);
	s2.Trim();
	pCmdUI->Enable(! s1.IsEmpty() && ! s2.IsEmpty());
}


void CDirectoryFingerprintDlg::OnOK()
{
	// Check if it's OK to overwrite the file

	if ( ! m_bOkToOverwriteFile)
	{
		// check if the file exists
		m_SaveFilename.GetWindowText(m_sSaveFilename);
		DWORD attr = GetFileAttributes(m_sSaveFilename);
		if (0xFFFFFFFF != attr)
		{
			CString s;
			s.Format(IDS_REPLACEYESNO, LPCTSTR(m_sSaveFilename));
			if (IDYES != AfxMessageBox(s, MB_YESNO | MB_DEFBUTTON2))
			{
				return;
			}
		}
	}

	CDialog::OnOK();
}
