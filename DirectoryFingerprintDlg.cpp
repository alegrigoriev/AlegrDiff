// DirectoryFingerprintDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DirectoryFingerprintDlg.h"
#include "FolderDialog.h"

// CDirectoryFingerprintDlg dialog

IMPLEMENT_DYNAMIC(CDirectoryFingerprintDlg, CDialog)
CDirectoryFingerprintDlg::CDirectoryFingerprintDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDirectoryFingerprintDlg::IDD, pParent)
	, m_bIncludeDirectoryStructure(FALSE)
	, m_bIncludeSubdirectories(FALSE)
	, m_sDirectory(_T(""))
	, m_sFilenameFilter(_T(""))
	, m_bIgnoreFiles(FALSE)
	, m_sIgnoreFiles(_T(""))
	, m_sSaveFilename(_T(""))
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
	DDX_Text(pDX, IDC_COMBO_FIRST_DIR, m_sDirectory);
	DDX_Control(pDX, IDC_EDITFILENAME_FILTER, m_FilenameFilterCombo);
	DDX_Text(pDX, IDC_EDITFILENAME_FILTER, m_sFilenameFilter);
	DDX_Check(pDX, IDC_CHECK_IGNORE_FILES, m_bIgnoreFiles);
	DDX_Control(pDX, IDC_COMBO_IGNORE_FILES, m_cbIgnoreFiles);
	DDX_Text(pDX, IDC_COMBO_IGNORE_FILES, m_sIgnoreFiles);
	DDX_Control(pDX, IDC_COMBO_SAVE_FILENAME, m_SaveFilename);
	DDX_CBString(pDX, IDC_COMBO_SAVE_FILENAME, m_sSaveFilename);
}


BEGIN_MESSAGE_MAP(CDirectoryFingerprintDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_DIR, OnBnClickedButtonBrowseDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SAVE_FILENAME, OnBnClickedButtonBrowseSaveFilename)
	ON_BN_CLICKED(IDC_CHECK_IGNORE_FILES, OnBnClickedCheckIgnoreFiles)
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
}


void CDirectoryFingerprintDlg::OnBnClickedCheckIgnoreFiles()
{
	GetDlgItem(IDC_COMBO_IGNORE_FILES)->EnableWindow
		(IsDlgButtonChecked(IDC_CHECK_IGNORE_FILES));
}
