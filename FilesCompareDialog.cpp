// FilesCompareDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "FilesCompareDialog.h"

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
					lpszFilter, pParentWnd), m_bBinaryMode(false)
	{}
	~COpenDiffDialog()
	{
		GetApp()->Profile.RemoveSection(_T("History"));
	}

	CString m_RecentFolders[15];
	bool m_bBinaryMode;
	virtual BOOL OnFileNameOK();
	//virtual void OnLBSelChangedNotify(UINT nIDBox, UINT iCurSel, UINT nCode);

	virtual void OnInitDone();
	//{{AFX_MSG(COpenDiffDialog)
	afx_msg void OnComboSelendOK();
	afx_msg void OnCheckBinary();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
BEGIN_MESSAGE_MAP(COpenDiffDialog, CFileDialog)
	//{{AFX_MSG_MAP(COpenDiffDialog)
	ON_BN_CLICKED(IDC_CHECK_BINARY, OnCheckBinary)
	ON_CBN_SELENDOK(IDC_COMBO_RECENT, OnComboSelendOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void COpenDiffDialog::OnCheckBinary()
{
	CButton * pBinary = (CButton *)GetDlgItem(IDC_CHECK_BINARY);
	if (NULL != pBinary)
	{
		m_bBinaryMode = (0 != pBinary->GetCheck());
	}
}

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
	CString sCurrDir;
	GetParent()->SendMessage(CDM_GETFOLDERPATH, MAX_PATH, LPARAM(sCurrDir.GetBuffer(MAX_PATH)));
	sCurrDir.ReleaseBuffer();
	TRACE("COpenDiffDialog::OnFileNameOK Folder Path=%s\n", sCurrDir);

	for (i = 0, j = 0; i < sizeof m_RecentFolders / sizeof m_RecentFolders[0]; i++)
	{
		if (m_RecentFolders[i].IsEmpty()
			|| 0 == sCurrDir.CompareNoCase(m_RecentFolders[i]))
		{
			continue;
		}
		if (i != j)
		{
			m_RecentFolders[j] = m_RecentFolders[i];
		}
		j++;
	}
	for ( ; j < sizeof m_RecentFolders / sizeof m_RecentFolders[0]; j++)
	{
		m_RecentFolders[j].Empty();
	}
	// remove the last dir from the list
	for (i = (sizeof m_RecentFolders / sizeof m_RecentFolders[0]) - 1; i >= 1; i--)
	{
		m_RecentFolders[i] = m_RecentFolders[i - 1];
	}
	m_RecentFolders[0] = sCurrDir;
	GetApp()->Profile.UnloadSection(_T("History"));

	return CFileDialog::OnFileNameOK();
}

void COpenDiffDialog::OnInitDone()
{
	CFileDialog::OnInitDone();

	CButton * pBinary = (CButton *)GetDlgItem(IDC_CHECK_BINARY);
	if (NULL != pBinary)
	{
		pBinary->SetCheck(m_bBinaryMode);
	}

	CComboBox * pCb = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_RECENT));
	if (NULL != pCb)
	{
		pCb->SetExtendedUI();
		CThisApp * pApp = GetApp();
		for (int i = 0; i < sizeof m_RecentFolders / sizeof m_RecentFolders[0]; i++)
		{
			CString s;
			s.Format("dir%d", i);
			TRACE("Added reg item %s\n", LPCTSTR(s));
			pApp->Profile.AddItem(_T("History"), s, m_RecentFolders[i]);
			m_RecentFolders[i].TrimLeft();
			m_RecentFolders[i].TrimRight();
			if ( ! m_RecentFolders[i].IsEmpty())
			{
				pCb->AddString(m_RecentFolders[i]);
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
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFilesCompareDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilesCompareDialog)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
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
	// TODO: Add your control notification handler code here

}

void CFilesCompareDialog::OnButtonBrowseSecondFile()
{
	// TODO: Add your control notification handler code here

}
