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
	: CResizableDialog(CFilesCompareDialog::IDD, pParent)
	, m_ComparisonMode(0)
{
	//{{AFX_DATA_INIT(CFilesCompareDialog)
	//}}AFX_DATA_INIT
	static const ResizableDlgItem items[] =
	{
		IDC_COMBO_FIRST_FILE, ExpandRight,
		IDC_COMBO_SECOND_FILE, ExpandRight,
		IDC_BUTTON_BROWSE_FIRST_FILE, MoveRight,
		IDC_BUTTON_BROWSE_SECOND_FILE, MoveRight,
		IDOK, CenterHorizontally,
		IDCANCEL, CenterHorizontally,
	};
	SetResizeableItems(items, countof(items));
}


void CFilesCompareDialog::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
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
		m_DlgWidth = m_PrevWidth;
	}
	else
	{
		m_PrevWidth = m_DlgWidth;
	}
	//{{AFX_DATA_MAP(CFilesCompareDialog)
	DDX_Control(pDX, IDC_COMBO_SECOND_FILE, m_SecondCombo);
	DDX_Control(pDX, IDC_COMBO_FIRST_FILE, m_FirstCombo);
	//}}AFX_DATA_MAP
	DDX_CBString(pDX, IDC_COMBO_FIRST_FILE, m_sFirstFileName);
	DDX_CBString(pDX, IDC_COMBO_SECOND_FILE, m_sSecondFileName);
	// save file names to history
	if (pDX->m_bSaveAndValidate)
	{
		pApp->m_RecentFiles.AddString(m_sFirstFileName);
		pApp->m_RecentFiles.AddString(m_sSecondFileName, 1);
	}
	DDX_Radio(pDX, IDC_RADIO3, m_ComparisonMode);
}


BEGIN_MESSAGE_MAP(CFilesCompareDialog, CResizableDialog)
	//{{AFX_MSG_MAP(CFilesCompareDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FIRST_FILE, OnButtonBrowseFirstFile)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SECOND_FILE, OnButtonBrowseSecondFile)
	ON_WM_DROPFILES()
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
	ON_CBN_EDITCHANGE(IDC_COMBO_FIRST_FILE, OnChangeFile)
	ON_CBN_SELCHANGE(IDC_COMBO_FIRST_FILE, OnChangeFile)
	ON_CBN_EDITCHANGE(IDC_COMBO_SECOND_FILE, OnChangeFile)
	ON_CBN_SELCHANGE(IDC_COMBO_SECOND_FILE, OnChangeFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilesCompareDialog message handlers

void CFilesCompareDialog::OnChangeFile()
{
	NeedUpdateControls();
}

void CFilesCompareDialog::OnButtonBrowseFirstFile()
{
	CThisApp * pApp = GetApp();
	CString Name;
	m_FirstCombo.GetWindowText(Name);
	if (IDOK != BrowseForFile(IDS_OPEN_FIRST_TITLE, Name, m_FileDir1,
							& pApp->m_RecentFiles))
	{
		return;
	}

	m_FirstCombo.SetWindowText(Name);
	NeedUpdateControls();
}

void CFilesCompareDialog::OnButtonBrowseSecondFile()
{
	CThisApp * pApp = GetApp();
	CString Name;
	m_SecondCombo.GetWindowText(Name);
	if (IDOK != BrowseForFile(IDS_OPEN_SECOND_TITLE, Name, m_FileDir2,
							& pApp->m_RecentFiles))
	{
		return;
	}

	m_SecondCombo.SetWindowText(Name);
	NeedUpdateControls();
}

BOOL CFilesCompareDialog::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	CThisApp * pApp = GetApp();
	// set comboboxes
	m_FirstCombo.LimitText(MAX_PATH);
	m_SecondCombo.LimitText(MAX_PATH);

	pApp->m_RecentFiles.LoadCombo( & m_FirstCombo);
	pApp->m_RecentFiles.LoadCombo( & m_SecondCombo);

	DragAcceptFiles();

	HICON hIcon = (HICON) LoadImage(AfxFindResourceHandle
									(MAKEINTRESOURCE(IDI_COMPARE_FILES), RT_GROUP_ICON),
									MAKEINTRESOURCE(IDI_COMPARE_FILES),
									IMAGE_ICON,
									GetSystemMetrics(SM_CXICON),
									GetSystemMetrics(SM_CYICON), 0);
	SetIcon(hIcon, TRUE);			// Set big icon

	hIcon = (HICON) LoadImage(AfxFindResourceHandle
							(MAKEINTRESOURCE(IDI_COMPARE_FILES), RT_GROUP_ICON),
							MAKEINTRESOURCE(IDI_COMPARE_FILES),
							IMAGE_ICON,
							GetSystemMetrics(SM_CXSMICON),
							GetSystemMetrics(SM_CYSMICON), 0);
	SetIcon(hIcon, FALSE);			// Set small icon

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
	NeedUpdateControls();
}

void CFilesCompareDialog::OnUpdateOk(CCmdUI * pCmdUI)
{
	CString s1, s2;
	m_FirstCombo.GetWindowText(s1);
	s1.Trim();
	m_SecondCombo.GetWindowText(s2);
	s2.Trim();
	pCmdUI->Enable(! s1.IsEmpty() && ! s2.IsEmpty());
}

void CFilesCompareDialog::OnMetricsChange()
{
	CResizableDialog::OnMetricsChange();
	m_mmxi.ptMaxTrackSize.y = m_mmxi.ptMinTrackSize.y;
	m_mmxi.ptMaxSize.y = m_mmxi.ptMinTrackSize.y;
}

int CFilesCompareDialog::m_PrevWidth;
