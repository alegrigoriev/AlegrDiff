// FilesCompareDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "FilesCompareDialog.h"
#include <Dlgs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class COpenDiffDialog : public CFileDialog
{
public:
	COpenDiffDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
					LPCTSTR lpszDefExt = NULL,
					LPCTSTR lpszFileName = NULL,
					DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					LPCTSTR lpszFilter = NULL,
					CWnd* pParentWnd = NULL)
		: CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags,
					lpszFilter, pParentWnd)
	{}
	~COpenDiffDialog()
	{
	}

	virtual BOOL OnFileNameOK();

	virtual void OnInitDone();
	//{{AFX_MSG(COpenDiffDialog)
	afx_msg void OnComboSelendOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
BEGIN_MESSAGE_MAP(COpenDiffDialog, CFileDialog)
	//{{AFX_MSG_MAP(COpenDiffDialog)
	ON_CBN_SELENDOK(IDC_COMBO_RECENT, OnComboSelendOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void COpenDiffDialog::OnComboSelendOK()
{
	TRACE("COpenDiffDialog::OnComboSelendOK()\n");
	CString str;
	CComboBox * pCb = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_RECENT));
	if (NULL != pCb)
	{
		int sel = pCb->GetCurSel();
		if (-1 == sel
			|| sel >= pCb->GetCount())
		{
			return;
		}
		pCb->GetLBText(sel, str);
		TRACE("COpenDiffDialog::OnComboSelendOK: %s selected\n", str);
		if (str.IsEmpty())
		{
			return;
		}
		// check if the selected text is a folder
		// make sure we can find a file in the folder
		CString dir(str);
		TCHAR c = dir[dir.GetLength() - 1];
		if (c != ':'
			&& c != '\\'
			&& c != '/')
		{
			dir += '\\';
		}
		dir += '*';

		WIN32_FIND_DATA wfd;
		HANDLE hFind = FindFirstFile(dir, & wfd);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			DWORD error = GetLastError();
			TRACE("FindFirstFile failed, last error = %d\n", error);
			CString s;
			if (ERROR_ACCESS_DENIED == error)
			{
				s.Format(IDS_DIRECTORY_ACCESS_DENIED, LPCTSTR(str));
			}
			else if (1 || ERROR_DIRECTORY == error
					|| ERROR_PATH_NOT_FOUND == error
					|| ERROR_INVALID_NAME == error
					|| ERROR_BAD_NETPATH)
			{
				s.Format(IDS_DIRECTORY_NOT_FOUND, LPCTSTR(str));
			}
			AfxMessageBox(s);
			// delete the string from combobox
			pCb->DeleteString(sel);
			pCb->SetCurSel(-1); // no selection
			return;
		}
		else
		{
			TRACE("FindFirstFile success\n");
			FindClose(hFind);
			CWnd * pParent = GetParent();
			pParent->SendMessage(CDM_SETCONTROLTEXT, edt1, LPARAM(LPCTSTR(str)));
			pParent->SendMessage(WM_COMMAND, IDOK, 0);
			pParent->SendMessage(CDM_SETCONTROLTEXT, edt1, LPARAM(LPCTSTR("")));
			pParent->GetDlgItem(edt1)->SetFocus();
		}

	}
}

BOOL COpenDiffDialog::OnFileNameOK()
{
	// add the current directory name to MRU
	CThisApp * pApp = GetApp();
	CString sCurrDir;
	GetParent()->SendMessage(CDM_GETFOLDERPATH, MAX_PATH, LPARAM(sCurrDir.GetBuffer(MAX_PATH)));
	sCurrDir.ReleaseBuffer();
	TRACE("COpenDiffDialog::OnFileNameOK Folder Path=%s\n", sCurrDir);

	AddStringToHistory(sCurrDir, pApp->m_RecentFolders,
						sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0], false);

	return CFileDialog::OnFileNameOK();
}

void COpenDiffDialog::OnInitDone()
{
	CFileDialog::OnInitDone();
	CThisApp * pApp = GetApp();

	CComboBox * pCb = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_RECENT));
	if (NULL != pCb)
	{
		CString dir(m_ofn.lpstrInitialDir);
		if (dir.GetLength() > 1
			&& dir[dir.GetLength() - 1] == '\\')
		{
			dir.SetAt(dir.GetLength() - 1, 0);
		}
		pCb->SetExtendedUI();
		CThisApp * pApp = GetApp();
		int sel = -1;
		for (int i = 0; i < sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0]; i++)
		{
			if ( ! pApp->m_RecentFolders[i].IsEmpty())
			{
				pCb->AddString(pApp->m_RecentFolders[i]);
				if (0 == pApp->m_RecentFolders[i].CompareNoCase(dir))
				{
					sel = i;
				}
			}
		}
		if (-1 != sel)
		{
			pCb->SetCurSel(sel);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFilesCompareDialog dialog


CFilesCompareDialog::CFilesCompareDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFilesCompareDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFilesCompareDialog)
	m_bBinaryFile = FALSE;
	//}}AFX_DATA_INIT
	//m_bCCppFile = FALSE;
	m_sFirstFileName = _T("");
	m_sSecondFileName = _T("");
}


void CFilesCompareDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CThisApp * pApp = GetApp();

	// get file names from history
	if ( ! pDX->m_bSaveAndValidate)
	{
		if (m_sFirstFileName.IsEmpty())
		{
			m_sFirstFileName = pApp->m_RecentFiles[0];
		}
		if (m_sSecondFileName.IsEmpty())
		{
			m_sSecondFileName = pApp->m_RecentFiles[1];
		}
	}
	//{{AFX_DATA_MAP(CFilesCompareDialog)
	DDX_Control(pDX, IDC_COMBO_SECOND_FILE, m_SecondCombo);
	DDX_Control(pDX, IDC_COMBO_FIRST_FILE, m_FirstCombo);
	DDX_Check(pDX, IDC_CHECK_BINARY, m_bBinaryFile);
	//}}AFX_DATA_MAP
	//DDX_Check(pDX, IDC_CHECK_COMPARE_C_CPP, m_bCCppFile);
	DDX_CBString(pDX, IDC_COMBO_FIRST_FILE, m_sFirstFileName);
	DDX_CBString(pDX, IDC_COMBO_SECOND_FILE, m_sSecondFileName);
	// save file names to history
	if (pDX->m_bSaveAndValidate)
	{
		AddStringToHistory(m_sSecondFileName, pApp->m_RecentFiles,
							sizeof pApp->m_RecentFiles / sizeof pApp->m_RecentFiles[0], false);
		AddStringToHistory(m_sFirstFileName, pApp->m_RecentFiles,
							sizeof pApp->m_RecentFiles / sizeof pApp->m_RecentFiles[0], false);
	}
}


BEGIN_MESSAGE_MAP(CFilesCompareDialog, CDialog)
	//{{AFX_MSG_MAP(CFilesCompareDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_FILE, OnButtonBrowseFirstFile)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SECOND_FILE, OnButtonBrowseSecondFile)
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilesCompareDialog message handlers

void CFilesCompareDialog::OnButtonBrowseFirstFile()
{
	CString Filter;
	Filter.LoadString(IDS_FILENAME_FILTER);

	CString title;
	title.LoadString(IDS_OPEN_FIRST_TITLE);

	COpenDiffDialog dlg(TRUE, NULL, NULL,
						OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLETEMPLATE,
						Filter);
	dlg.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_OPEN_TEMPLATE);

	CString Name;
	m_FirstCombo.GetWindowText(Name);
	TCHAR FullPath[MAX_PATH + 1] = {0};
	LPTSTR FileNamePart = FullPath;

	if ( ! Name.IsEmpty())
	{
		if (GetFullPathName(Name, MAX_PATH, FullPath, & FileNamePart))
		{
			_tcsncpy(dlg.m_ofn.lpstrFile, FileNamePart, dlg.m_ofn.nMaxFile - 1);
			*FileNamePart = 0;
		}
	}

	if (FullPath[0] == 0)
	{
		dlg.m_ofn.lpstrInitialDir = m_FileDir1;
	}
	else
	{
		dlg.m_ofn.lpstrInitialDir = FullPath;
	}

	dlg.m_ofn.lpstrTitle = title;
	dlg.m_ofn.nFilterIndex = m_UsedFilenameFilter;

	if (IDOK != dlg.DoModal())
	{
		return;
	}

	TCHAR CurrDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, CurrDir);
	m_FileDir1 = CurrDir;
	m_UsedFilenameFilter = dlg.m_ofn.nFilterIndex;

	m_FirstCombo.SetWindowText(dlg.GetPathName());
}

void CFilesCompareDialog::OnButtonBrowseSecondFile()
{
	CString Filter;
	Filter.LoadString(IDS_FILENAME_FILTER);

	CString title;
	title.LoadString(IDS_OPEN_SECOND_TITLE);

	COpenDiffDialog dlg(TRUE, NULL, NULL,
						OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLETEMPLATE,
						Filter);
	dlg.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_OPEN_TEMPLATE);

	CString Name;
	m_SecondCombo.GetWindowText(Name);
	TCHAR FullPath[MAX_PATH + 1] = {0};
	LPTSTR FileNamePart = FullPath;

	if ( ! Name.IsEmpty())
	{
		if (GetFullPathName(Name, MAX_PATH, FullPath, & FileNamePart))
		{
			_tcsncpy(dlg.m_ofn.lpstrFile, FileNamePart, dlg.m_ofn.nMaxFile - 1);
			*FileNamePart = 0;
		}
	}

	if (FullPath[0] == 0)
	{
		dlg.m_ofn.lpstrInitialDir = m_FileDir2;
	}
	else
	{
		dlg.m_ofn.lpstrInitialDir = FullPath;
	}

	dlg.m_ofn.lpstrTitle = title;
	dlg.m_ofn.nFilterIndex = m_UsedFilenameFilter;

	if (IDOK != dlg.DoModal())
	{
		return;
	}

	TCHAR CurrDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, CurrDir);
	m_FileDir2 = CurrDir;
	m_UsedFilenameFilter = dlg.m_ofn.nFilterIndex;

	m_SecondCombo.SetWindowText(dlg.GetPathName());
}

BOOL CFilesCompareDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	CThisApp * pApp = GetApp();
	// set comboboxes
	for (int i = 0; i < sizeof pApp->m_RecentFiles / sizeof pApp->m_RecentFiles[0]; i++)
	{
		if ( ! pApp->m_RecentFiles[i].IsEmpty())
		{
			m_FirstCombo.AddString(pApp->m_RecentFiles[i]);
			m_SecondCombo.AddString(pApp->m_RecentFiles[i]);
		}
	}
	DragAcceptFiles();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFilesCompareDialog::OnDropFiles(HDROP hDropInfo)
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
	m_FirstCombo.GetWindowRect( & r1);
	ScreenToClient( & r1);

	m_SecondCombo.GetWindowRect( & r2);
	ScreenToClient( & r2);

	// if drop occured outside edit controls, put the file name to the empty field
	if (r1.PtInRect(point))
	{
		m_FirstCombo.SetWindowText(szFileName1);
	}
	else if (r2.PtInRect(point))
	{
		m_SecondCombo.SetWindowText(szFileName1);
	}
	else
	{
		if (0 == m_FirstCombo.GetWindowTextLength())
		{
			m_FirstCombo.SetWindowText(szFileName1);
		}
		else if (0 == m_SecondCombo.GetWindowTextLength())
		{
			m_SecondCombo.SetWindowText(szFileName1);
		}
		else
		{
			// TODO
		}
	}
}
