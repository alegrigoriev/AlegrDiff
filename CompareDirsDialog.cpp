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
	if ( ! pDX->m_bSaveAndValidate)
	{
		// read last dirs from the registry
		for (int i = 0; i < sizeof m_sHistory / sizeof m_sHistory[0]; i++)
		{
			CString s;
			s.Format("dir%d", i);
			pApp->Profile.AddItem(_T("History"), s, m_sHistory[i]);
			m_sHistory[i].TrimLeft();
			m_sHistory[i].TrimRight();
		}
		m_sFirstDir = m_sHistory[0];
		m_sSecondDir = m_sHistory[1];
	}
	//{{AFX_DATA_MAP(CCompareDirsDialog)
	DDX_Control(pDX, IDC_COMBO_FIRST_DIR, m_FirstDirCombo);
	DDX_Control(pDX, IDC_COMBO_SECOND_DIR, m_SecondDirCombo);
	DDX_Check(pDX, IDC_CHECK_INCLUDE_SUBDIRS, m_bIncludeSubdirs);
	DDX_Text(pDX, IDC_EDITFILENAME_FILTER, m_FilenameFilter);
	DDX_Check(pDX, IDC_CHECK_BINARY, m_BinaryComparision);
	//}}AFX_DATA_MAP
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
		//m_FirstDirCombo.SetExtendedUI();
		//m_SecondDirCombo.SetExtendedUI();
		// set the dirs to combobox
		for (int i = 0; i < sizeof m_sHistory / sizeof m_sHistory[0]; i++)
		{
			if ( ! m_sHistory[i].IsEmpty())
			{
				m_FirstDirCombo.AddString(m_sHistory[i]);
				m_SecondDirCombo.AddString(m_sHistory[i]);
			}
		}
	}
	else
	{
		// don't need to read dirs from the combobox
		// remove those that match the currently selected dirs
		int i, j;
		for (i = 0, j = 0; i < sizeof m_sHistory / sizeof m_sHistory[0]; i++)
		{
			if (0 == m_sFirstDir.CompareNoCase(m_sHistory[i])
				|| 0 == m_sSecondDir.CompareNoCase(m_sHistory[i]))
			{
				continue;
			}
			if (i != j)
			{
				m_sHistory[j] = m_sHistory[i];
			}
			j++;
		}
		// remove two last dirs from the list
		for (i = (sizeof m_sHistory / sizeof m_sHistory[0]) - 1; i >= 2; i--)
		{
			m_sHistory[i] = m_sHistory[i - 2];
		}
		m_sHistory[0] = m_sFirstDir;
		m_sHistory[1] = m_sSecondDir;
		// write last dirs to the registry
		pApp->Profile.UnloadSection(_T("History"));
	}
}


BEGIN_MESSAGE_MAP(CCompareDirsDialog, CDialog)
	//{{AFX_MSG_MAP(CCompareDirsDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_DIR, OnButtonBrowseFirstDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SECOND_DIR, OnButtonBrowseSecondDir)
	ON_BN_CLICKED(IDC_BUTTON_ADVANCED, OnButtonAdvanced)
	ON_BN_CLICKED(IDC_CHECK_BINARY, OnCheckBinary)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompareDirsDialog message handlers

void CCompareDirsDialog::OnButtonBrowseFirstDir()
{
	m_FirstDirCombo.GetWindowText(m_sFirstDir);
	CFolderDialog dlg("Select First Folder To Compare",
					m_sFirstDir);
	if (IDOK == dlg.DoModal())
	{
		m_FirstDirCombo.SetWindowText(dlg.GetFolderPath());
	}
}

void CCompareDirsDialog::OnButtonBrowseSecondDir()
{
	m_SecondDirCombo.GetWindowText(m_sSecondDir);
	CFolderDialog dlg("Select Second Folder To Compare",
					m_sSecondDir);
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
