// SaveFileListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "SaveFileListDlg.h"
#include "FileDialogWithHistory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveFileListDlg dialog


CSaveFileListDlg::CSaveFileListDlg(CWnd* pParent /*=NULL*/)
	: CUiUpdatedDlg(CSaveFileListDlg::IDD, pParent)
	, m_bEnableSelectedItems(FALSE)
{
	//{{AFX_DATA_INIT(CSaveFileListDlg)
	m_bIncludeComparisonResult = FALSE;
	m_bIncludeDifferentFiles = FALSE;
	m_bIncludeDifferentInBlanksFiles = FALSE;
	m_bIncludeDifferentVersionFiles = FALSE;
	m_bIncludeFolder1OnlyFiles = FALSE;
	m_bIncludeFolder2OnlyFiles = FALSE;
	m_bIncludeIdenticalFiles = FALSE;
	m_bIncludeSubdirectoryName = FALSE;
	m_bIncludeTimestamp = FALSE;
	m_bIncludeLength = FALSE;
	m_sFilename = _T("");
	m_IncludeFilesSelect = -1;
	//}}AFX_DATA_INIT

	m_Profile.AddItem(_T("Settings\\SaveList"), _T("Filename"), m_sFilename, _T(""));
	m_Profile.AddBoolItem(_T("Settings\\SaveList"), _T("IncludeComparisonResult"), m_bIncludeComparisonResult, FALSE);
	m_Profile.AddBoolItem(_T("Settings\\SaveList"), _T("DifferentFiles"), m_bIncludeDifferentFiles, TRUE);
	m_Profile.AddBoolItem(_T("Settings\\SaveList"), _T("DifferentInBlanksFiles"), m_bIncludeDifferentInBlanksFiles, FALSE);
	m_Profile.AddBoolItem(_T("Settings\\SaveList"), _T("DifferentInVersionFiles"), m_bIncludeDifferentVersionFiles, FALSE);
	m_Profile.AddBoolItem(_T("Settings\\SaveList"), _T("Folder1OnlyFiles"), m_bIncludeFolder1OnlyFiles, FALSE);
	m_Profile.AddBoolItem(_T("Settings\\SaveList"), _T("Folder2OnlyFiles"), m_bIncludeFolder2OnlyFiles, FALSE);
	m_Profile.AddBoolItem(_T("Settings\\SaveList"), _T("IdenticalFiles"), m_bIncludeIdenticalFiles, FALSE);
	m_Profile.AddBoolItem(_T("Settings\\SaveList"), _T("IncludeSubdirectoryName"), m_bIncludeSubdirectoryName, TRUE);
	m_Profile.AddBoolItem(_T("Settings\\SaveList"), _T("IncludeTimestamp"), m_bIncludeTimestamp, FALSE);
	m_Profile.AddBoolItem(_T("Settings\\SaveList"), _T("IncludeLength"), m_bIncludeLength, FALSE);
	m_Profile.AddItem(_T("Settings\\SaveList"), _T("FilesSelect"), m_IncludeFilesSelect, 2, 0, 2);

}


void CSaveFileListDlg::DoDataExchange(CDataExchange* pDX)
{
	CUiUpdatedDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveFileListDlg)
	DDX_Control(pDX, IDC_EDIT_FILENAME, m_eFilename);
	DDX_Check(pDX, IDC_CHECK_COMPARISON_RESULT, m_bIncludeComparisonResult);
	DDX_Check(pDX, IDC_CHECK_DIFFERENT, m_bIncludeDifferentFiles);
	DDX_Check(pDX, IDC_CHECK_DIFFERENT_IN_BLANKS, m_bIncludeDifferentInBlanksFiles);
	DDX_Check(pDX, IDC_CHECK_DIFFERENT_IN_VERSION, m_bIncludeDifferentVersionFiles);
	DDX_Check(pDX, IDC_CHECK_FOLDER1_ONLY, m_bIncludeFolder1OnlyFiles);
	DDX_Check(pDX, IDC_CHECK_FOLDER2_ONLY, m_bIncludeFolder2OnlyFiles);
	DDX_Check(pDX, IDC_CHECK_IDENTICAL, m_bIncludeIdenticalFiles);
	DDX_Check(pDX, IDC_CHECK_SUBDIRECTORY, m_bIncludeSubdirectoryName);
	DDX_Check(pDX, IDC_CHECK_TIMESTAMP, m_bIncludeTimestamp);
	DDX_Check(pDX, IDC_CHECK_FILE_LENGTH, m_bIncludeLength);
	DDX_Text(pDX, IDC_EDIT_FILENAME, m_sFilename);
	DDX_Radio(pDX, IDC_RADIO_INCLUDE_FILES, m_IncludeFilesSelect);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		m_Profile.FlushAll();
	}
}


BEGIN_MESSAGE_MAP(CSaveFileListDlg, CUiUpdatedDlg)
	//{{AFX_MSG_MAP(CSaveFileListDlg)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
	ON_UPDATE_COMMAND_UI(IDC_CHECK_IDENTICAL, OnUpdateCheckIncludeGroup)
	ON_UPDATE_COMMAND_UI(IDC_CHECK_DIFFERENT, OnUpdateCheckIncludeGroup)
	ON_UPDATE_COMMAND_UI(IDC_CHECK_DIFFERENT_IN_BLANKS, OnUpdateCheckIncludeGroup)
	ON_UPDATE_COMMAND_UI(IDC_CHECK_DIFFERENT_IN_VERSION, OnUpdateCheckIncludeGroup)
	ON_UPDATE_COMMAND_UI(IDC_CHECK_FOLDER1_ONLY, OnUpdateCheckIncludeGroup)
	ON_UPDATE_COMMAND_UI(IDC_CHECK_FOLDER2_ONLY, OnUpdateCheckIncludeGroup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveFileListDlg message handlers

void CSaveFileListDlg::OnUpdateOk(CCmdUI * pCmdUI)
{
	CString s;
	m_eFilename.GetWindowText(s);
	s.Trim();
	pCmdUI->Enable(! s.IsEmpty());
}

void CSaveFileListDlg::OnUpdateCheckIncludeGroup(CCmdUI * pCmdUI)
{
	pCmdUI->Enable(1 == IsDlgButtonChecked(IDC_RADIO_SELECTED_GROUPS));
}

void CSaveFileListDlg::OnButtonBrowse()
{
	CString Filter;
	Filter.LoadString(IDS_SAVE_LIST_FILENAME_FILTER);

	CString title;
	title.LoadString(IDS_SAVE_LIST_TITLE);

	CFileDialogWithHistory dlg(FALSE, & GetApp()->m_RecentFolders, _T(".txt"), NULL,
								OFN_HIDEREADONLY
								| OFN_NOTESTFILECREATE
								| OFN_PATHMUSTEXIST
								| OFN_EXPLORER
								| OFN_NOCHANGEDIR | OFN_ENABLESIZING,
								Filter);

	dlg.m_ofn.lpstrTitle = title;
	CString Name;
	m_eFilename.GetWindowText(Name);

	TCHAR FullPath[MAX_PATH + 1] = {0};
	LPTSTR FileNamePart = FullPath;

	if ( ! Name.IsEmpty())
	{
		if (GetFullPathName(Name, MAX_PATH, FullPath, & FileNamePart)
			&& NULL != FileNamePart)
		{
			_tcsncpy(dlg.m_ofn.lpstrFile, FileNamePart, dlg.m_ofn.nMaxFile - 1);
			*FileNamePart = 0;
		}
	}

	dlg.m_ofn.lpstrInitialDir = FullPath;

	if (IDOK != dlg.DoModal())
	{
		return;
	}

	m_eFilename.SetWindowText(dlg.GetPathName());
}

BOOL CSaveFileListDlg::OnInitDialog()
{
	NeedUpdateControls();
	CUiUpdatedDlg::OnInitDialog();
	UpdateDialogControls(this, FALSE);

	EnableDlgItem(IDC_RADIO_SELECTED_FILES, m_bEnableSelectedItems);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSaveFileListDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NeedUpdateControls();
	return CUiUpdatedDlg::OnCommand(wParam, lParam);
}

void CSaveFileListDlg::OnOK()
{

	if (!UpdateData(TRUE))
	{
		TRACE0("UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}
	// check if replacing the file
	FILE * file = _tfopen(m_sFilename, _T("rt"));
	if (NULL != file)
	{
		fclose(file);
		CString s;
		s.Format(IDS_REPLACEYESNO, LPCTSTR(m_sFilename));
		if (IDYES != AfxMessageBox(s , MB_YESNO | MB_DEFBUTTON2))
		{
			return;
		}
	}
	EndDialog(IDOK);
}
