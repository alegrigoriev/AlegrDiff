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
	m_bUseBinaryFilesFilter(true),
	m_bUseCppFilter(true),
	m_bUseIgnoreFilter(true),
	m_nTabIndent(4)
{
	//{{AFX_DATA_INIT(CCompareDirsDialog)
	m_bIncludeSubdirs = FALSE;
	m_FilenameFilter = _T("");
	m_sSecondDir = _T("");
	m_sFirstDir = _T("");
	m_BinaryComparision = FALSE;
	//}}AFX_DATA_INIT
}


void CCompareDirsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CThisApp * pApp = GetApp();
	int i;
	if ( ! pDX->m_bSaveAndValidate)
	{
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
			m_FilenameFilter = pApp->m_sFilters[0];
		}
	}
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
		DDX_Control(pDX, IDC_SPIN1, m_Spin);
		DDX_Check(pDX, IDC_CHECK_BINARY_FILES, m_bUseBinaryFilesFilter);
		DDX_Check(pDX, IDC_CHECK_C_CPP, m_bUseCppFilter);
		DDX_Check(pDX, IDC_CHECK_IGNORE, m_bUseIgnoreFilter);
		DDX_Text(pDX, IDC_EDIT_BINARY_FILES, m_sBinaryFilesFilter);
		DDX_Text(pDX, IDC_EDIT_C_CPP, m_sCppFilesFilter);
		DDX_Text(pDX, IDC_EDIT_IGNORE, m_sIgnoreFilesFilter);
		DDX_Text(pDX, IDC_EDIT_TAB_INDENT, m_nTabIndent);
		DDV_MinMaxUInt(pDX, m_nTabIndent, 0, 32);
	}
	if ( ! pDX->m_bSaveAndValidate)
	{
		// set the dirs to combobox
		for (i = 0; i < sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0]; i++)
		{
			if ( ! pApp->m_RecentFolders[i].IsEmpty())
			{
				m_FirstDirCombo.AddString(pApp->m_RecentFolders[i]);
				m_SecondDirCombo.AddString(pApp->m_RecentFolders[i]);
			}
		}

		// set the filters to combobox
		for (i = 0; i < sizeof pApp->m_sFilters / sizeof pApp->m_sFilters[0]; i++)
		{
			if ( ! pApp->m_sFilters[i].IsEmpty())
			{
				m_cFilenameFilter.AddString(pApp->m_sFilters[i]);
			}
		}
	}
	else
	{
		// Update MRU, case insensitive
		AddStringToHistory(m_sSecondDir, pApp->m_RecentFolders,
							sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0], false);
		AddStringToHistory(m_sFirstDir, pApp->m_RecentFolders,
							sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0], false);
		AddStringToHistory(m_FilenameFilter, pApp->m_sFilters,
							sizeof pApp->m_sFilters / sizeof pApp->m_sFilters[0], false);

	}
}


BEGIN_MESSAGE_MAP(CCompareDirsDialog, CDialog)
	//{{AFX_MSG_MAP(CCompareDirsDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_DIR, OnButtonBrowseFirstDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SECOND_DIR, OnButtonBrowseSecondDir)
	ON_BN_CLICKED(IDC_BUTTON_ADVANCED, OnButtonAdvanced)
	ON_BN_CLICKED(IDC_CHECK_BINARY, OnCheckBinary)
	ON_BN_CLICKED(IDC_CHECK_BINARY_FILES, OnCheckBinaryFiles)
	ON_BN_CLICKED(IDC_CHECK_C_CPP, OnCheckCCpp)
	ON_BN_CLICKED(IDC_CHECK_IGNORE, OnCheckIgnore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompareDirsDialog message handlers

void CCompareDirsDialog::OnCheckBinaryFiles()
{
	CWnd * pEdit = GetDlgItem(IDC_EDIT_BINARY_FILES);
	if (pEdit)
	{
		pEdit->EnableWindow(IsDlgButtonChecked(IDC_CHECK_BINARY_FILES));
	}
}

void CCompareDirsDialog::OnCheckCCpp()
{
	CWnd * pEdit = GetDlgItem(IDC_EDIT_C_CPP);
	if (pEdit)
	{
		pEdit->EnableWindow(IsDlgButtonChecked(IDC_CHECK_C_CPP));
	}
}

void CCompareDirsDialog::OnCheckIgnore()
{
	CWnd * pEdit = GetDlgItem(IDC_EDIT_IGNORE);
	if (pEdit)
	{
		pEdit->EnableWindow(IsDlgButtonChecked(IDC_CHECK_IGNORE));
	}
}

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

void CCompareDirsDialog::OnCancel()
{
	GetApp()->Profile.RemoveSection(_T("History"));
	CDialog::OnCancel();
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

	CWnd * pWnd = GetDlgItem(IDC_CHECK_C_CPP);
	if (pWnd)
	{
		pWnd->EnableWindow(NotBinary);
	}
	pWnd = GetDlgItem(IDC_EDIT_C_CPP);
	if (pWnd)
	{
		pWnd->EnableWindow(NotBinary && IsDlgButtonChecked(IDC_CHECK_C_CPP));
	}
	pWnd = GetDlgItem(IDC_CHECK_BINARY_FILES);
	if (pWnd)
	{
		pWnd->EnableWindow(NotBinary);
	}
	pWnd = GetDlgItem(IDC_EDIT_BINARY_FILES);
	if (pWnd)
	{
		pWnd->EnableWindow(NotBinary && IsDlgButtonChecked(IDC_CHECK_BINARY_FILES));
	}
	pWnd = GetDlgItem(IDC_EDIT_TAB_INDENT);
	if (pWnd)
	{
		pWnd->EnableWindow(NotBinary);
	}
}

BOOL CCompareDirsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_bAdvanced)
	{
		m_Spin.SetRange(1, 32);
	}

	OnCheckBinary();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
