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
	: CDialog(CCompareDirsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCompareDirsDialog)
	m_bIncludeSubdirs = FALSE;
	m_sSecondDir = _T("");
	m_sFirstDir = _T("");
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
	//}}AFX_DATA_MAP
	DDX_CBString(pDX, IDC_COMBO_FIRST_DIR, m_sFirstDir);
	DDX_CBString(pDX, IDC_COMBO_SECOND_DIR, m_sSecondDir);
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
