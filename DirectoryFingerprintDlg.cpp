// DirectoryFingerprintDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DirectoryFingerprintDlg.h"
#include "FolderDialog.h"
#include "FileDialogWithHistory.h"

// CDirectoryFingerprintDlg dialog

CDirectoryFingerprintDlg::CDirectoryFingerprintDlg(
													LPCTSTR sIgnoreFilesFilter,
													BOOL bIncludeSubdirectories,
													CWnd* pParent /*=NULL*/)
	: BaseClass(CDirectoryFingerprintDlg::IDD, pParent)
	, m_bIncludeDirectoryStructure(FALSE)
	, m_bIncludeSubdirectories(bIncludeSubdirectories)
	, m_bOkToOverwriteFile(FALSE)
	, m_sIgnoreFiles(sIgnoreFilesFilter)
	, m_IgnoreFilterHistory( & m_Profile, _T("History"), _T("IgnoreFiles%d"), 10)
	, m_IgnoreFolderHistory( & m_Profile, _T("History"), _T("IgnoreFolders%d"), 10)
	, m_FingerprintFilenameHistory( & m_Profile, _T("History"), _T("FingerprintFile%d"), 15)
{
	static const ResizableDlgItem items[] =
	{
		IDC_COMBO_FIRST_DIR, ExpandRight,
		IDC_COMBO_IGNORE_FILES, ExpandRight,
		IDC_COMBO_SAVE_FILENAME, ExpandRight,
		IDC_EDITFILENAME_FILTER, ExpandRight,
		IDC_COMBO_IGNORE_DIRS, ExpandRight,
		IDC_BUTTON_BROWSE_FIRST_DIR, MoveRight,
		IDC_BUTTON_BROWSE_SAVE_FILENAME, MoveRight,
		IDOK, CenterHorizontally,
		IDCANCEL, CenterHorizontally,
	};
	SetResizeableItems(items, countof(items));
}

CDirectoryFingerprintDlg::~CDirectoryFingerprintDlg()
{
}

void CDirectoryFingerprintDlg::DoDataExchange(CDataExchange* pDX)
{
	BaseClass::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_FIRST_DIR, m_DirCombo);
	DDX_CBString(pDX, IDC_COMBO_FIRST_DIR, m_sDirectory);

	DDX_Check(pDX, IDC_CHECK_INCLUDE_DIRECTORY_STRUCTURE, m_bIncludeDirectoryStructure);
	DDX_Check(pDX, IDC_CHECK_INCLUDE_SUBDIRS, m_bIncludeSubdirectories);

	DDX_Control(pDX, IDC_EDITFILENAME_FILTER, m_FilenameFilterCombo);
	DDX_Text(pDX, IDC_EDITFILENAME_FILTER, m_sFilenameFilter);

	DDX_Control(pDX, IDC_COMBO_IGNORE_FILES, m_cbIgnoreFiles);
	DDX_CBString(pDX, IDC_COMBO_IGNORE_FILES, m_sIgnoreFiles);

	DDX_Control(pDX, IDC_COMBO_IGNORE_DIRS, m_cbIgnoreFolders);
	DDX_CBString(pDX, IDC_COMBO_IGNORE_DIRS, m_sIgnoreFolders);

	DDX_Control(pDX, IDC_COMBO_SAVE_FILENAME, m_SaveFilename);
	DDX_CBString(pDX, IDC_COMBO_SAVE_FILENAME, m_sSaveFilename);

	static int PrevWidth = 0;

	if (pDX->m_bSaveAndValidate)
	{
		CThisApp * pApp = GetApp();

		pApp->m_RecentFolders.AddString(m_sDirectory);
		pApp->m_FileFilters.AddString(m_sFilenameFilter);

		m_IgnoreFilterHistory.AddString(m_sIgnoreFiles);
		m_IgnoreFolderHistory.AddString(m_sIgnoreFolders);
		m_FingerprintFilenameHistory.AddString(m_sSaveFilename);

		m_Profile.FlushAll();

		PrevWidth = m_DlgWidth;
	}
	else
	{
		m_DlgWidth = PrevWidth;
	}
}


BEGIN_MESSAGE_MAP(CDirectoryFingerprintDlg, BaseClass)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_DIR, OnBnClickedButtonBrowseDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SAVE_FILENAME, OnBnClickedButtonBrowseSaveFilename)

	ON_BN_CLICKED(IDC_CHECK_INCLUDE_SUBDIRS, OnBnClickedIncludeSubdirs)

	ON_CBN_EDITCHANGE(IDC_COMBO_FIRST_DIR, OnCbnEditchangeComboFirstDir)
	ON_CBN_SELCHANGE(IDC_COMBO_FIRST_DIR, OnCbnSelchangeComboFirstDir)

	ON_CBN_EDITCHANGE(IDC_COMBO_SAVE_FILENAME, OnCbnEditchangeComboSaveFilename)
	ON_CBN_SELCHANGE(IDC_COMBO_SAVE_FILENAME, OnCbnSelchangeComboSaveFilename)

	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
	ON_UPDATE_COMMAND_UI(IDC_COMBO_IGNORE_DIRS, OnUpdateIgnoreDirs)
	ON_UPDATE_COMMAND_UI(IDC_CHECK_INCLUDE_DIRECTORY_STRUCTURE, OnUpdateIncludeDirectoryStructure)
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
		NeedUpdateControls();
	}
}

void CDirectoryFingerprintDlg::OnBnClickedButtonBrowseSaveFilename()
{
	CString Filter, Title;
	Filter.LoadString(IDS_STRING_FILGERPRINT_FILTER);
	Title.LoadString(IDS_STRING_FINGERPRINT_TITLE);

	m_SaveFilename.GetWindowText(m_sSaveFilename);

	CFileDialogWithHistory dlg(FALSE, & GetApp()->m_RecentFolders, _T("md5fp"), m_sSaveFilename,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT
								| OFN_NOTESTFILECREATE
								| OFN_PATHMUSTEXIST
								| OFN_NOCHANGEDIR | OFN_EXPLORER,
								Filter);
	dlg.m_ofn.lpstrTitle = Title;

	if (IDOK != dlg.DoModal())
	{
		return;
	}
	m_SaveFilename.SetWindowText(dlg.GetPathName());

	m_bOkToOverwriteFile = TRUE;
	NeedUpdateControls();
}

BOOL CDirectoryFingerprintDlg::OnInitDialog()
{
	CThisApp * pApp = GetApp();

	m_IgnoreFilterHistory.Load();
	m_IgnoreFolderHistory.Load();
	m_FingerprintFilenameHistory.Load();

	m_Profile.AddBoolItem(_T("Settings"), _T("IncludeDirsToFingerprint"), m_bIncludeDirectoryStructure, TRUE);

	m_sSaveFilename = m_FingerprintFilenameHistory[0];
	m_sFilenameFilter = pApp->m_FileFilters[0];
	m_sIgnoreFiles = m_IgnoreFilterHistory[0];
	m_sIgnoreFolders = m_IgnoreFolderHistory[0];

	m_sDirectory = pApp->m_RecentFolders[0];

	BaseClass::OnInitDialog();

	SetBigAndSmallIcons(IDI_CREATE_FINGERPRINT);

	m_DirCombo.LimitText(MAX_PATH);

	pApp->m_RecentFolders.LoadCombo( & m_DirCombo);
	pApp->m_FileFilters.LoadCombo( & m_FilenameFilterCombo);

	m_IgnoreFilterHistory.LoadCombo( & m_cbIgnoreFiles);
	m_IgnoreFolderHistory.LoadCombo( & m_cbIgnoreFolders);

	m_SaveFilename.LimitText(MAX_PATH);
	m_FingerprintFilenameHistory.LoadCombo( & m_SaveFilename);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDirectoryFingerprintDlg::OnCbnEditchangeComboFirstDir()
{
	NeedUpdateControls();
}

void CDirectoryFingerprintDlg::OnCbnSelchangeComboFirstDir()
{
	NeedUpdateControls();
}

void CDirectoryFingerprintDlg::OnBnClickedIncludeSubdirs()
{
	NeedUpdateControls();
}

void CDirectoryFingerprintDlg::OnCbnEditchangeComboSaveFilename()
{
	m_bOkToOverwriteFile = FALSE;
	NeedUpdateControls();
}

void CDirectoryFingerprintDlg::OnCbnSelchangeComboSaveFilename()
{
	m_bOkToOverwriteFile = FALSE;
	NeedUpdateControls();
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

void CDirectoryFingerprintDlg::OnUpdateIgnoreDirs(CCmdUI * pCmdUI)
{
	pCmdUI->Enable(IsDlgButtonChecked(IDC_CHECK_INCLUDE_SUBDIRS));
}

void CDirectoryFingerprintDlg::OnUpdateIncludeDirectoryStructure(CCmdUI * pCmdUI)
{
	pCmdUI->Enable(IsDlgButtonChecked(IDC_CHECK_INCLUDE_SUBDIRS));
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

	BaseClass::OnOK();
}

void CDirectoryFingerprintDlg::OnMetricsChange()
{
	BaseClass::OnMetricsChange();
	m_mmxi.ptMaxTrackSize.y = m_mmxi.ptMinTrackSize.y;
	m_mmxi.ptMaxSize.y = m_mmxi.ptMinTrackSize.y;
}
