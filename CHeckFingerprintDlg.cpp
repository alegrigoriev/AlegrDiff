// CheckFingerprintDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "CheckFingerprintDlg.h"
#include "FolderDialog.h"
#include "FileDialogWithHistory.h"

// CCheckFingerprintDlg dialog

CCheckFingerprintDlg::CCheckFingerprintDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CCheckFingerprintDlg::IDD, pParent)
	, m_FingerprintFilenameHistory( & m_Profile, _T("History"), _T("FingerprintFile%d"), 15)
{
	static const ResizableDlgItem items[] =
	{
		IDC_COMBO_FIRST_DIR, ExpandRight,
		IDC_COMBO_SAVE_FILENAME, ExpandRight,
		IDC_BUTTON_BROWSE_FIRST_DIR, MoveRight,
		IDC_BUTTON_BROWSE_SAVE_FILENAME, MoveRight,
		IDOK, CenterHorizontally,
		IDCANCEL, CenterHorizontally,
	};
	m_pResizeItems = items;
	m_pResizeItemsCount = countof (items);
}

CCheckFingerprintDlg::~CCheckFingerprintDlg()
{
}

void CCheckFingerprintDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_FIRST_DIR, m_cbDirectory);
	DDX_CBString(pDX, IDC_COMBO_FIRST_DIR, m_sDirectory);
	DDX_Control(pDX, IDC_COMBO_SAVE_FILENAME, m_cbFilename);
	DDX_CBString(pDX, IDC_COMBO_SAVE_FILENAME, m_sFilename);

	static int PrevWidth = 0;

	if (pDX->m_bSaveAndValidate)
	{
		CThisApp * pApp = GetApp();

		pApp->m_RecentFolders.AddString(m_sDirectory);
		m_FingerprintFilenameHistory.AddString(m_sFilename);

		m_Profile.FlushAll();

		PrevWidth = m_DlgWidth;
	}
	else
	{
		m_DlgWidth = PrevWidth;
	}
}


BEGIN_MESSAGE_MAP(CCheckFingerprintDlg, CResizableDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_DIR, OnBnClickedButtonBrowseFirstDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SAVE_FILENAME, OnBnClickedButtonBrowseOpenFilename)
	ON_CBN_EDITCHANGE(IDC_COMBO_FIRST_DIR, OnCbnEditchangeComboFirstDir)
	ON_CBN_SELCHANGE(IDC_COMBO_FIRST_DIR, OnCbnSelchangeComboFirstDir)
	ON_CBN_EDITCHANGE(IDC_COMBO_SAVE_FILENAME, OnCbnEditchangeComboSaveFilename)
	ON_CBN_SELCHANGE(IDC_COMBO_SAVE_FILENAME, OnCbnSelchangeComboSaveFilename)
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
	NeedUpdateControls();
}

void CCheckFingerprintDlg::OnBnClickedButtonBrowseOpenFilename()
{
	CString Filter, Title;
	Filter.LoadString(IDS_STRING_FILGERPRINT_FILTER);
	Title.LoadString(IDS_STRING_OPEN_FINGERPRINT_TITLE);

	m_cbFilename.GetWindowText(m_sFilename);

	CFileDialogWithHistory dlg(TRUE, & GetApp()->m_RecentFolders, _T("md5fp"), m_sFilename,
								OFN_HIDEREADONLY | OFN_NOCHANGEDIR
								| OFN_FILEMUSTEXIST | OFN_EXPLORER,
								Filter);

	dlg.m_ofn.lpstrTitle = Title;

	if (IDOK != dlg.DoModal())
	{
		return;
	}

	m_cbFilename.SetWindowText(dlg.GetPathName());
	NeedUpdateControls();
}

BOOL CCheckFingerprintDlg::OnInitDialog()
{
	CThisApp * pApp = GetApp();

	m_FingerprintFilenameHistory.Load();

	m_sFilename = m_FingerprintFilenameHistory[0];
	m_sDirectory = pApp->m_RecentFolders[0];

	CResizableDialog::OnInitDialog();

	m_cbDirectory.LimitText(MAX_PATH);
	m_cbFilename.LimitText(MAX_PATH);

	pApp->m_RecentFolders.LoadCombo( & m_cbDirectory);
	m_FingerprintFilenameHistory.LoadCombo( & m_cbFilename);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCheckFingerprintDlg::OnCbnEditchangeComboFirstDir()
{
	NeedUpdateControls();
}

void CCheckFingerprintDlg::OnCbnSelchangeComboFirstDir()
{
	NeedUpdateControls();
}

void CCheckFingerprintDlg::OnCbnEditchangeComboSaveFilename()
{
	NeedUpdateControls();
}

void CCheckFingerprintDlg::OnCbnSelchangeComboSaveFilename()
{
	NeedUpdateControls();
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

void CCheckFingerprintDlg::OnMetricsChange()
{
	CResizableDialog::OnMetricsChange();
	m_mmxi.ptMaxTrackSize.y = m_mmxi.ptMinTrackSize.y;
	m_mmxi.ptMaxSize.y = m_mmxi.ptMinTrackSize.y;
}
