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
	CString Name;
	m_FirstCombo.GetWindowText(Name);
	if (IDOK != BrowseForFile(IDS_OPEN_FIRST_TITLE, Name, m_FileDir1))
	{
		return;
	}

	m_FirstCombo.SetWindowText(Name);
}

void CFilesCompareDialog::OnButtonBrowseSecondFile()
{
	CString Name;
	m_SecondCombo.GetWindowText(Name);
	if (IDOK != BrowseForFile(IDS_OPEN_SECOND_TITLE, Name, m_FileDir2))
	{
		return;
	}

	m_SecondCombo.SetWindowText(Name);
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
