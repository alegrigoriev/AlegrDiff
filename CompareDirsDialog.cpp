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
	: CDialog(CCompareDirsDialog::IDD, pParent),
	m_bAdvanced(false),
	m_nTabIndent(4)
	, m_BinaryFilterHistory(& m_Profile, _T("History"), _T("BinaryFiles%d"), 5)
	, m_CppFilterHistory(& m_Profile, _T("History"), _T("CppFiles%d"), 5)
	, m_IgnoreFilterHistory(& m_Profile, _T("History"), _T("IgnoreFiles%d"), 10)
{
	//{{AFX_DATA_INIT(CCompareDirsDialog)
	m_bIncludeSubdirs = FALSE;
	m_FilenameFilter = _T("");
	m_sSecondDir = _T("");
	m_sFirstDir = _T("");
	m_BinaryComparision = FALSE;
	//}}AFX_DATA_INIT
	m_bUseMd5 = TRUE;
}


void CCompareDirsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
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

		DDX_CBString(pDX, IDC_EDIT_BINARY_FILES, m_sBinaryFilesFilter);
		DDX_CBString(pDX, IDC_EDIT_C_CPP, m_sCppFilesFilter);
		DDX_CBString(pDX, IDC_EDIT_IGNORE, m_sIgnoreFilesFilter);

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

		m_Profile.UnloadAll();
	}
}


BEGIN_MESSAGE_MAP(CCompareDirsDialog, CDialog)
	//{{AFX_MSG_MAP(CCompareDirsDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_DIR, OnButtonBrowseFirstDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SECOND_DIR, OnButtonBrowseSecondDir)
	ON_BN_CLICKED(IDC_BUTTON_ADVANCED, OnButtonAdvanced)
	ON_BN_CLICKED(IDC_CHECK_BINARY, OnCheckBinary)
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
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

int CCompareDirsDialog::DoModal()
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
		Result = CDialog::DoModal();
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
	BOOL NotBinary = ! IsDlgButtonChecked(IDC_CHECK_BINARY);

	CWnd * pWnd = GetDlgItem(IDC_EDIT_C_CPP);
	if (pWnd)
	{
		pWnd->EnableWindow(NotBinary);
	}
	pWnd = GetDlgItem(IDC_EDIT_BINARY_FILES);
	if (pWnd)
	{
		pWnd->EnableWindow(NotBinary);
	}
	pWnd = GetDlgItem(IDC_EDIT_TAB_INDENT);
	if (pWnd)
	{
		pWnd->EnableWindow(NotBinary);
	}
}

BOOL CCompareDirsDialog::OnInitDialog()
{
	CThisApp * pApp = GetApp();

	m_BinaryFilterHistory.Load();

	m_CppFilterHistory.Load();

	m_IgnoreFilterHistory.Load();

	CDialog::OnInitDialog();

	// set the filters to combobox
	pApp->m_RecentFolders.LoadCombo( & m_FirstDirCombo);
	pApp->m_RecentFolders.LoadCombo( & m_SecondDirCombo);
	pApp->m_FileFilters.LoadCombo( & m_cFilenameFilter);

	if (m_bAdvanced)
	{
		m_Spin.SetRange(1, 32);

		m_BinaryFilterHistory.LoadCombo( & m_cbBinaryFilesFilter);
		m_CppFilterHistory.LoadCombo( & m_cbCppFilesFilter);
		m_IgnoreFilterHistory.LoadCombo( & m_cbIgnoreFilesFilter);
	}

	OnCheckBinary();
	DragAcceptFiles();

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
