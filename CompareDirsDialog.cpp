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
	m_sSecondDirCombo = _T("");
	m_sFirstDirCombo = _T("");
	//}}AFX_DATA_INIT
}


void CCompareDirsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCompareDirsDialog)
	DDX_Control(pDX, IDC_COMBO_FIRST_DIR, m_FirstDirCombo);
	DDX_Control(pDX, IDC_COMBO_SECOND_DIR, m_SecondDirCombo);
	DDX_Check(pDX, IDC_CHECK_INCLUDE_SUBDIRS, m_bIncludeSubdirs);
	//}}AFX_DATA_MAP
	DDX_CBString(pDX, IDC_COMBO_FIRST_DIR, m_sFirstDirCombo);
	DDX_CBString(pDX, IDC_COMBO_SECOND_DIR, m_sSecondDirCombo);
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
	m_FirstDirCombo.GetWindowText(m_sFirstDirCombo);
	CFolderDialog dlg("Select First Folder To Compare",
					m_sFirstDirCombo);
	if (IDOK == dlg.DoModal())
	{
		m_FirstDirCombo.SetWindowText(dlg.GetFolderPath());
	}
}

void CCompareDirsDialog::OnButtonBrowseSecondDir()
{
	m_SecondDirCombo.GetWindowText(m_sSecondDirCombo);
	CFolderDialog dlg("Select Second Folder To Compare",
					m_sSecondDirCombo);
	if (IDOK == dlg.DoModal())
	{
		m_SecondDirCombo.SetWindowText(dlg.GetFolderPath());
	}
}
