// CompareDirsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "CompareDirsDialog.h"
#include "FolderDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCompareDirsDialog dialog


CCompareDirsDialog::CCompareDirsDialog(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CCompareDirsDialog::IDD, pParent),
	m_bAdvanced(false),
	m_nTabIndent(4)
	, m_BinaryFilterHistory(& m_Profile, _T("History"), _T("BinaryFiles%d"), 5)
	, m_CppFilterHistory(& m_Profile, _T("History"), _T("CppFiles%d"), 5)
	, m_IgnoreFilterHistory(& m_Profile, _T("History"), _T("IgnoreFiles%d"), 10)
	, m_IgnoreFoldersHistory(& m_Profile, _T("History"), _T("IgnoreFolders%d"), 10)
{
	//{{AFX_DATA_INIT(CCompareDirsDialog)
	m_bIncludeSubdirs = FALSE;
	m_FilenameFilter = _T("");
	m_sSecondDir = _T("");
	m_sFirstDir = _T("");
	m_BinaryComparision = FALSE;
	//}}AFX_DATA_INIT
	m_bUseMd5 = TRUE;

	static const ResizableDlgItem items[] =
	{
		IDC_COMBO_FIRST_DIR, ExpandRight,
		IDC_COMBO_SECOND_DIR, ExpandRight,
		IDC_EDITFILENAME_FILTER, ExpandRight,
		IDC_EDIT_BINARY_FILES, ExpandRight,
		IDC_EDIT_C_CPP, ExpandRight,
		IDC_EDIT_IGNORE, ExpandRight,
		IDC_EDIT_IGNORE_DIRS, ExpandRight,
		IDC_BUTTON_BROWSE_FIRST_DIR, MoveRight,
		IDC_BUTTON_BROWSE_SECOND_DIR, MoveRight,
		IDC_BUTTON_ADVANCED, MoveRight,
		IDOK, CenterHorizontally,
		IDCANCEL, CenterHorizontally,
	};
	m_pResizeItems = items;
	m_pResizeItemsCount = countof (items);
}


void CCompareDirsDialog::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	CThisApp * pApp = GetApp();
	//{{AFX_DATA_MAP(CCompareDirsDialog)
	DDX_Control(pDX, IDC_COMBO_FIRST_DIR, m_FirstDirCombo);
	DDX_Control(pDX, IDC_COMBO_SECOND_DIR, m_SecondDirCombo);
	DDX_Control(pDX, IDC_EDITFILENAME_FILTER, m_cFilenameFilter);
	DDX_Check(pDX, IDC_CHECK_INCLUDE_SUBDIRS, m_bIncludeSubdirs);
	DDX_Check(pDX, IDC_CHECK_BINARY, m_BinaryComparision);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDITFILENAME_FILTER, m_FilenameFilter);
	DDX_CBString(pDX, IDC_COMBO_FIRST_DIR, m_sFirstDir);
	DDX_CBString(pDX, IDC_COMBO_SECOND_DIR, m_sSecondDir);
	if (m_bAdvanced)
	{
		DDX_Check(pDX, IDC_CHECK_USE_MD5, m_bUseMd5);
		DDX_Control(pDX, IDC_SPIN1, m_Spin);
		DDX_Control(pDX, IDC_EDIT_BINARY_FILES, m_cbBinaryFilesFilter);
		DDX_Control(pDX, IDC_EDIT_C_CPP, m_cbCppFilesFilter);
		DDX_Control(pDX, IDC_EDIT_IGNORE, m_cbIgnoreFilesFilter);
		DDX_Control(pDX, IDC_EDIT_IGNORE_DIRS, m_cbIgnoreFoldersFilter);

		DDX_CBString(pDX, IDC_EDIT_BINARY_FILES, m_sBinaryFilesFilter);
		DDX_CBString(pDX, IDC_EDIT_C_CPP, m_sCppFilesFilter);
		DDX_CBString(pDX, IDC_EDIT_IGNORE, m_sIgnoreFilesFilter);
		DDX_CBString(pDX, IDC_EDIT_IGNORE_DIRS, m_sIgnoreFoldersFilter);

		DDX_Text(pDX, IDC_EDIT_TAB_INDENT, m_nTabIndent);
		DDV_MinMaxUInt(pDX, m_nTabIndent, 0, 32);
	}

	if (pDX->m_bSaveAndValidate)
	{
		// Update MRU, case insensitive
		pApp->m_RecentFolders.AddString(m_sFirstDir);
		// second folder added after the first. If they both are the same, two same folders added
		pApp->m_RecentFolders.AddString(m_sSecondDir, 1);

		pApp->m_FileFilters.AddString(m_FilenameFilter);

		m_BinaryFilterHistory.AddString(m_sBinaryFilesFilter);
		m_CppFilterHistory.AddString(m_sCppFilesFilter);
		m_IgnoreFilterHistory.AddString(m_sIgnoreFilesFilter);
		m_IgnoreFoldersHistory.AddString(m_sIgnoreFoldersFilter);

		m_Profile.UnloadAll();
		m_PrevWidth = m_DlgWidth;
	}
	else
	{
		m_DlgWidth = m_PrevWidth;
	}
}


BEGIN_MESSAGE_MAP(CCompareDirsDialog, CResizableDialog)
	//{{AFX_MSG_MAP(CCompareDirsDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_DIR, OnButtonBrowseFirstDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SECOND_DIR, OnButtonBrowseSecondDir)
	ON_BN_CLICKED(IDC_BUTTON_ADVANCED, OnButtonAdvanced)
	ON_BN_CLICKED(IDC_CHECK_BINARY, OnCheckBinary)
	ON_BN_CLICKED(IDC_CHECK_INCLUDE_SUBDIRS, OnCheckIncludeSubdirs)
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(IDC_EDIT_IGNORE_DIRS, OnUpdateIgnoreDirs)
	ON_UPDATE_COMMAND_UI(IDC_EDIT_BINARY_FILES, OnUpdateEditBinaryFiles)
	ON_UPDATE_COMMAND_UI(IDC_EDIT_C_CPP, OnUpdateEditCCpp)
	ON_UPDATE_COMMAND_UI(IDC_EDIT_TAB_INDENT, OnUpdateTabIndent)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompareDirsDialog message handlers

void CCompareDirsDialog::OnButtonBrowseFirstDir()
{
	m_FirstDirCombo.GetWindowText(m_sFirstDir);
	CString DlgTitle;
	DlgTitle.LoadString(IDS_FIRST_FOLDER_DLG_TITLE);
	CFolderDialog dlg(DlgTitle, m_sFirstDir);

	if (IDOK == dlg.DoModal())
	{
		m_FirstDirCombo.SetWindowText(dlg.GetFolderPath());
	}
}

void CCompareDirsDialog::OnButtonBrowseSecondDir()
{
	m_SecondDirCombo.GetWindowText(m_sSecondDir);
	CString DlgTitle;
	DlgTitle.LoadString(IDS_SECOND_FOLDER_DLG_TITLE);
	CFolderDialog dlg(DlgTitle, m_sSecondDir);

	if (IDOK == dlg.DoModal())
	{
		m_SecondDirCombo.SetWindowText(dlg.GetFolderPath());
	}
}

void CCompareDirsDialog::OnButtonAdvanced()
{
	EndDialog(IDC_BUTTON_ADVANCED);
	if (!UpdateData(TRUE))
	{
		TRACE0("UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}
}

INT_PTR CCompareDirsDialog::DoModal()
{
	int Result;
	CThisApp * pApp = GetApp();
	if (m_sFirstDir.IsEmpty())
	{
		m_sFirstDir = pApp->m_RecentFolders[0];
	}
	if (m_sSecondDir.IsEmpty())
	{
		m_sSecondDir = pApp->m_RecentFolders[1];
	}

	if (m_FilenameFilter.IsEmpty())
	{
		m_FilenameFilter = pApp->m_FileFilters[0];
	}
	while (1)
	{
		if (m_bAdvanced)
		{
			m_lpszTemplateName = MAKEINTRESOURCE(IDD_DIALOG_COMPARE_DIRS_ADVANCED);
		}
		else
		{
			m_lpszTemplateName = MAKEINTRESOURCE(IDD_DIALOG_COMPARE_DIRS);
		}

		Result = CResizableDialog::DoModal();
		if (Result != IDC_BUTTON_ADVANCED)
		{
			break;
		}
		m_bAdvanced = ! m_bAdvanced;
	}
	return Result;
}

void CCompareDirsDialog::OnCheckBinary()
{
	NeedUpdateControls();
}

void CCompareDirsDialog::OnCheckIncludeSubdirs()
{
	NeedUpdateControls();
}

BOOL CCompareDirsDialog::OnInitDialog()
{
	CThisApp * pApp = GetApp();

	m_BinaryFilterHistory.Load();

	m_CppFilterHistory.Load();

	m_IgnoreFilterHistory.Load();
	m_IgnoreFoldersHistory.Load();

	CResizableDialog::OnInitDialog();

	// set the filters to combobox
	m_FirstDirCombo.LimitText(MAX_PATH);
	m_SecondDirCombo.LimitText(MAX_PATH);

	pApp->m_RecentFolders.LoadCombo( & m_FirstDirCombo);
	pApp->m_RecentFolders.LoadCombo( & m_SecondDirCombo);
	pApp->m_FileFilters.LoadCombo( & m_cFilenameFilter);

	if (m_bAdvanced)
	{
		m_Spin.SetRange(1, 32);

		m_BinaryFilterHistory.LoadCombo( & m_cbBinaryFilesFilter);
		m_CppFilterHistory.LoadCombo( & m_cbCppFilesFilter);
		m_IgnoreFilterHistory.LoadCombo( & m_cbIgnoreFilesFilter);
		m_IgnoreFoldersHistory.LoadCombo( & m_cbIgnoreFoldersFilter);
	}

	DragAcceptFiles();

	HICON hIcon = (HICON) LoadImage(AfxFindResourceHandle
									(MAKEINTRESOURCE(IDI_COMPARE_DIRS), RT_GROUP_ICON),
									MAKEINTRESOURCE(IDI_COMPARE_DIRS),
									IMAGE_ICON,
									GetSystemMetrics(SM_CXICON),
									GetSystemMetrics(SM_CYICON), 0);
	SetIcon(hIcon, TRUE);			// Set big icon
	hIcon = (HICON) LoadImage(AfxFindResourceHandle
							(MAKEINTRESOURCE(IDI_COMPARE_DIRS), RT_GROUP_ICON),
							MAKEINTRESOURCE(IDI_COMPARE_DIRS),
							IMAGE_ICON,
							GetSystemMetrics(SM_CXSMICON),
							GetSystemMetrics(SM_CYSMICON), 0);
	SetIcon(hIcon, FALSE);			// Set small icon

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCompareDirsDialog::OnDropFiles(HDROP hDropInfo)
{
	SetActiveWindow();      // activate us first !
	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	CPoint point;
	::DragQueryPoint(hDropInfo, & point);
	//TRACE("::DragQueryPoint=%d, %d\n", point.x, point.y);
	// point contains client coordinates
	TCHAR szFileName1[_MAX_PATH] = {0};

	if (nFiles >=1)
	{
		::DragQueryFile(hDropInfo, 0, szFileName1, _MAX_PATH);
	}

	::DragFinish(hDropInfo);
	if (0 == szFileName1[0])
	{
		return;
	}
	// if drop occured over an edit control, put the file name there
	CRect r, r1, r2;
	GetClientRect( & r);
	m_FirstDirCombo.GetWindowRect( & r1);
	ScreenToClient( & r1);

	m_SecondDirCombo.GetWindowRect( & r2);
	ScreenToClient( & r2);

	// if drop occured outside edit controls, put the file name to the empty field
	if (r1.PtInRect(point))
	{
		m_FirstDirCombo.SetWindowText(szFileName1);
	}
	else if (r2.PtInRect(point))
	{
		m_SecondDirCombo.SetWindowText(szFileName1);
	}
	else
	{
		if (0 == m_FirstDirCombo.GetWindowTextLength())
		{
			m_FirstDirCombo.SetWindowText(szFileName1);
		}
		else if (0 == m_SecondDirCombo.GetWindowTextLength())
		{
			m_SecondDirCombo.SetWindowText(szFileName1);
		}
		else
		{
			// TODO
		}
	}
}

void CCompareDirsDialog::OnUpdateIgnoreDirs(CCmdUI * pCmdUI)
{
	pCmdUI->Enable(IsDlgButtonChecked(IDC_CHECK_INCLUDE_SUBDIRS));
}

void CCompareDirsDialog::OnUpdateEditBinaryFiles(CCmdUI * pCmdUI)
{
	pCmdUI->Enable(! IsDlgButtonChecked(IDC_CHECK_BINARY));
}

void CCompareDirsDialog::OnUpdateEditCCpp(CCmdUI * pCmdUI)
{
	pCmdUI->Enable(! IsDlgButtonChecked(IDC_CHECK_BINARY));
}

void CCompareDirsDialog::OnUpdateTabIndent(CCmdUI * pCmdUI)
{
	pCmdUI->Enable(! IsDlgButtonChecked(IDC_CHECK_BINARY));
}

void CCompareDirsDialog::OnMetricsChange()
{
	CResizableDialog::OnMetricsChange();
	m_mmxi.ptMaxTrackSize.y = m_mmxi.ptMinTrackSize.y;
	m_mmxi.ptMaxSize.y = m_mmxi.ptMinTrackSize.y;
}

int CCompareDirsDialog::m_PrevWidth;
