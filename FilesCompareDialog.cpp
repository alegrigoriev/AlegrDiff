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
	int i, j;
	CThisApp * pApp = GetApp();
	CString sCurrDir;
	GetParent()->SendMessage(CDM_GETFOLDERPATH, MAX_PATH, LPARAM(sCurrDir.GetBuffer(MAX_PATH)));
	sCurrDir.ReleaseBuffer();
	TRACE("COpenDiffDialog::OnFileNameOK Folder Path=%s\n", sCurrDir);

	for (i = 0, j = 0; i < sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0]; i++)
	{
		if (pApp->m_RecentFolders[i].IsEmpty()
			|| 0 == sCurrDir.CompareNoCase(pApp->m_RecentFolders[i]))
		{
			continue;
		}
		if (i != j)
		{
			pApp->m_RecentFolders[j] = pApp->m_RecentFolders[i];
		}
		j++;
	}
	for ( ; j < sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0]; j++)
	{
		pApp->m_RecentFolders[j].Empty();
	}
	// remove the last dir from the list
	for (i = (sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0]) - 1; i >= 1; i--)
	{
		pApp->m_RecentFolders[i] = pApp->m_RecentFolders[i - 1];
	}
	pApp->m_RecentFolders[0] = sCurrDir;

	return CFileDialog::OnFileNameOK();
}

void COpenDiffDialog::OnInitDone()
{
	CFileDialog::OnInitDone();
	CThisApp * pApp = GetApp();

	CComboBox * pCb = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_RECENT));
	if (NULL != pCb)
	{
		pCb->SetExtendedUI();
		CThisApp * pApp = GetApp();
		for (int i = 0; i < sizeof pApp->m_RecentFolders / sizeof pApp->m_RecentFolders[0]; i++)
		{
			if ( ! pApp->m_RecentFolders[i].IsEmpty())
			{
				pCb->AddString(pApp->m_RecentFolders[i]);
			}
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
	m_bCCppFile = FALSE;
	//}}AFX_DATA_INIT
	m_sFirstFileName = _T("");
	m_sSecondFileName = _T("");
}


void CFilesCompareDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilesCompareDialog)
	DDX_Control(pDX, IDC_COMBO_SECOND_FILE, m_SecondCombo);
	DDX_Control(pDX, IDC_COMBO_FIRST_FILE, m_FirstCombo);
	DDX_Check(pDX, IDC_CHECK_BINARY, m_bBinaryFile);
	DDX_Check(pDX, IDC_CHECK_COMPARE_C_CPP, m_bCCppFile);
	//}}AFX_DATA_MAP
	DDX_CBString(pDX, IDC_COMBO_FIRST_FILE, m_sFirstFileName);
	DDX_CBString(pDX, IDC_COMBO_SECOND_FILE, m_sSecondFileName);
}


BEGIN_MESSAGE_MAP(CFilesCompareDialog, CDialog)
	//{{AFX_MSG_MAP(CFilesCompareDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_FILE, OnButtonBrowseFirstFile)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SECOND_FILE, OnButtonBrowseSecondFile)
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

	dlg.m_ofn.lpstrInitialDir = m_FileDir1;
	dlg.m_ofn.lpstrTitle = title;
	dlg.m_ofn.nFilterIndex = m_UsedFilenameFilter;

	if (IDOK != dlg.DoModal())
	{
		return;
	}
	TCHAR CurrDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, CurrDir);
	m_FileDir1 = CurrDir;

	m_FirstCombo.SetWindowText(dlg.GetPathName());
}

void CFilesCompareDialog::OnButtonBrowseSecondFile()
{

}
