// AlegrDiff.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "AlegrDiffDoc.h"
#include "AlegrDiffView.h"
#include "DiffFileView.h"
#include "CompareDirsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffApp

BEGIN_MESSAGE_MAP(CAlegrDiffApp, CWinApp)
	//{{AFX_MSG_MAP(CAlegrDiffApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_COMPAREDIRECTORIES, OnFileComparedirectories)
	ON_COMMAND(ID_FILE_COMPAREFILES, OnFileComparefiles)
	//}}AFX_MSG_MAP
	// Standard file based document commands
//	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffApp construction

CAlegrDiffApp::CAlegrDiffApp()
	: m_MaxSearchDistance(256),
	m_pFileDiffTemplate(NULL),
	m_pListDiffTemplate(NULL),
	m_TabIndent(4),
	m_MinIdenticalChars(3),
	m_NormalTextColor(0),
	m_ErasedTextColor(0x000000FF),  // red
	m_AddedTextColor(0x00FF0000),   // blue
	m_bRecurseSubdirs(false),
	m_MinIdenticalLines(5)
{
}

CAlegrDiffApp::~CAlegrDiffApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CAlegrDiffApp object

CAlegrDiffApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffApp initialization

BOOL CAlegrDiffApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("AleGr SoftWare"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	Profile.AddItem(_T("Settings"), _T("NormalTextColor"), m_NormalTextColor, 0, 0, 0xFFFFFF);
	Profile.AddItem(_T("Settings"), _T("AddedTextColor"), m_AddedTextColor, 0x00FF0000, 0, 0xFFFFFF);
	Profile.AddItem(_T("Settings"), _T("ErasedTextColor"), m_ErasedTextColor, 0x000000FF, 0, 0xFFFFFF);
	Profile.AddItem(_T("Settings"), _T("TabIndent"), m_TabIndent, 4, 1, 32);
	Profile.AddItem(_T("Settings"), _T("RecurseSubdirs"), m_bRecurseSubdirs, false);
	Profile.AddItem(_T("Settings"), _T("InitialDir1"), m_FileDir1, _T(""));
	Profile.AddItem(_T("Settings"), _T("InitialDir2"), m_FileDir2, _T(""));
	Profile.AddItem(_T("Settings"), _T("FilenameFilter"), m_sFilenameFilter, _T("*"));
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	m_pListDiffTemplate = new CMultiDocTemplate(
												IDR_ALEGRDTYPE,
												RUNTIME_CLASS(CAlegrDiffDoc),
												RUNTIME_CLASS(CChildFrame), // custom MDI child frame
												RUNTIME_CLASS(CAlegrDiffView));
	AddDocTemplate(m_pListDiffTemplate);

	m_pFileDiffTemplate = new CMultiDocTemplate(
												IDR_FILEDIFFTYPE,
												RUNTIME_CLASS(CFilePairDoc),
												RUNTIME_CLASS(CChildFrame), // custom MDI child frame
												RUNTIME_CLASS(CDiffFileView));
	AddDocTemplate(m_pFileDiffTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
#if 0
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
#endif
	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
	pMainFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CAlegrDiffApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffApp message handlers

int CAlegrDiffApp::ExitInstance()
{
	Profile.UnloadSection(_T("Settings"));
	return CWinApp::ExitInstance();
}

void CAlegrDiffApp::OnFileComparedirectories()
{
	CCompareDirsDialog dlg;
	dlg.m_bIncludeSubdirs = m_bRecurseSubdirs;
	dlg.m_FilenameFilter = m_sFilenameFilter;

	if (IDOK == dlg.DoModal())
	{
		m_sFilenameFilter = dlg.m_FilenameFilter;
		m_bRecurseSubdirs = (1 == dlg.m_bIncludeSubdirs);

		CAlegrDiffDoc * pDoc = (CAlegrDiffDoc *)
								m_pListDiffTemplate->OpenDocumentFile(NULL);
		if (NULL == pDoc)
		{
			return;
		}
//        pDoc->
		if (pDoc->BuildFilePairList(dlg.m_sFirstDir, dlg.m_sSecondDir, m_bRecurseSubdirs))
		{
			pDoc->UpdateAllViews(NULL);
		}
		else
		{
			delete pDoc;
		}
	}
}

void CAlegrDiffApp::OpenFilePairView(FilePair * pPair)
{
	// check if there is already a CFilePairDoc
	CFilePairDoc * pDoc = (CFilePairDoc *)m_pFileDiffTemplate->OpenDocumentFile(NULL);

	if (NULL != pDoc)
	{
		pDoc->SetFilePair(pPair);
	}
}



void CAlegrDiffApp::OnFileComparefiles()
{
	// check if there is already a CFilePairDoc
	CString Name1;
	CString Name2;
	CString Filter;
	Filter.LoadString(IDS_FILENAME_FILTER);
	CFileDialog dlg1(TRUE, NULL, NULL,
					OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING,
					Filter);
	dlg1.m_ofn.lpstrInitialDir = m_FileDir1;

	if (IDOK != dlg1.DoModal())
	{
		return;
	}
	TCHAR CurrDir1[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, CurrDir1);

	CFileDialog dlg2(TRUE, NULL, NULL,
					OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING,
					Filter);

	dlg2.m_ofn.lpstrInitialDir = m_FileDir2;
	if (m_FileDir2.IsEmpty())
	{
	}
	if (IDOK != dlg2.DoModal())
	{
		return;
	}
	TCHAR CurrDir2[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, CurrDir2);

	Name1 = dlg1.GetPathName();
	Name2 = dlg2.GetPathName();

	TCHAR FileDir1[MAX_PATH];
	LPTSTR pFileName1 = FileDir1;
	GetFullPathName(Name1, MAX_PATH, FileDir1, & pFileName1);
	*pFileName1 = 0;

	TCHAR FileDir2[MAX_PATH];
	LPTSTR pFileName2 = FileDir2;
	GetFullPathName(Name2, MAX_PATH, FileDir2, & pFileName2);
	*pFileName2 = 0;

	m_FileDir1 = CurrDir1;
	m_FileDir2 = CurrDir2;

	WIN32_FIND_DATA wfd1;
	WIN32_FIND_DATA wfd2;
	HANDLE hFind = FindFirstFile(Name1, & wfd1);
	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		AfxMessageBox("Couldn't open first source file");
		return;
	}
	FindClose(hFind);

	hFind = FindFirstFile(Name2, & wfd2);
	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		AfxMessageBox("Couldn't open second source file");
		return;
	}
	FindClose(hFind);

	CFilePairDoc * pDoc = (CFilePairDoc *)m_pFileDiffTemplate->OpenDocumentFile(NULL);
	if (NULL != pDoc)
	{
		FilePair * pPair = new FilePair;
		pPair->pFirstFile = new FileItem( & wfd1, FileDir1, "");
		pPair->pSecondFile = new FileItem( & wfd2, FileDir2, "");

		pDoc->SetFilePair(pPair);
		// SetFilePair references the pair, we need to compensate it
		pPair->Dereference();
	}

}
