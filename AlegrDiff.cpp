// AlegrDiff.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "AlegrDiffDoc.h"
#include "AlegrDiffView.h"

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
	m_MinIdenticalLines(3)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
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

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
										IDR_ALEGRDTYPE,
										RUNTIME_CLASS(CAlegrDiffDoc),
										RUNTIME_CLASS(CChildFrame), // custom MDI child frame
										RUNTIME_CLASS(CAlegrDiffView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
	pMainFrame->UpdateWindow();

	return TRUE;
}

CString FilePair::GetComparisionResult()
{
	if (NULL == pFirstFile)
	{
		if (NULL == pSecondFile)
		{
			return CString();
		}
		CString s;
		s.Format("File exists only in \"%s%s\"",
				pSecondFile->GetBasedir(), pSecondFile->GetSubdir());
		return s;
	}
	if (NULL == pSecondFile)
	{
		CString s;
		s.Format("File exists only in \"%s%s\"",
				pFirstFile->GetBasedir(), pFirstFile->GetSubdir());
		return s;
	}
	return CString();
}

DWORD FilePair::CompareFiles(bool bCompareAll)
{
	if (NULL == pFirstFile
		|| NULL == pSecondFile)
	{
		return 0;
	}
	// TODO: different function for binary comparision
	if (! pFirstFile->Load()
		|| ! pSecondFile->Load())
	{
		pFirstFile->Unload();
		return 0;
	}
	// different comparision for different modes
	return CompareTextFiles(bCompareAll);
	return 1;
}

DWORD FilePair::CompareTextFiles(bool bCompareAll)
{
	// find similar lines
	CThisApp * pApp = GetApp();
	int nLine1 = 0;
	int nLine2 = 0;
	int NumLines1 = pFirstFile->GetNumLines();
	int NumLines2 = pSecondFile->GetNumLines();
	// build list of equal sections
	while (nLine1 < NumLines1
			&& nLine2 < NumLines2)
	{
		// find the beginning of the section
		// find a few identical lines
		while (nLine1 < NumLines1
				&& nLine2 < NumLines2)
		{
			for (int dist = 0; dist < pApp->m_MaxSearchDistance; dist++)
			{
				const FileLine * Line1 = pFirstFile->GetLine(nLine1+dist);
				if ( ! Line1->IsBlank())
				{
					// check the lines in file2 in range Line2 to Line2+dist
					for (int i = 0; i < dist && i + nLine2 < NumLines2; i++)
					{
						const FileLine * Line2 = pSecondFile->GetLine(nLine2 + i);
						if (Line1->IsEqual(Line2))
						{
							int n1 = nLine1 + 1;
							int n2 = nLine2 + i + 1;
							int NumEqual = 1;
							// check if a few non-blank lines more are the same
							while(n1 < NumLines1 && n2 < NumLines2
								&& NumEqual < pApp->m_MinIdenticalLines
								&& n1 - nLine1 < pApp->m_MaxSearchDistance)
							{
								const FileLine * L1 = pFirstFile->GetLine(n1);
								if (L1->IsBlank())
								{
									n1++;
									continue;
								}
								const FileLine * L2 = pSecondFile->GetLine(n2);
								if (L2->IsBlank())
								{
									n2++;
									continue;
								}
								if ( ! L1->IsEqual(L2))
								{
									break;
								}
								n1++;
								n2++;
								NumEqual++;
							}
							if (NumEqual >= pApp->m_MinIdenticalLines)
							{
								break;
							}
						}
					}
				}
			}
		}
		int Line1Begin = nLine1;
		int Line2Begin = nLine2;

		while (nLine1 < NumLines1
				&& nLine2 < NumLines2)
		{
			const FileLine * Line1 = pFirstFile->GetLine(nLine1);
			const FileLine * Line2 = pSecondFile->GetLine(nLine2);
			if (Line1->IsEqual(Line2))
			{
				nLine1++;
				nLine2++;
			}
			else
			{
				// the lines are different
				if (! bCompareAll)
				{
					// if we don't need to compare the while file (just scanning)
					// return now
					return 1;
				}
				// check if the lines are similar enough
				// the lines can be considered similar if < 1/4 of the characters is different,
				// or the only difference is in whitespaces
				if (Line1->LooksLike(Line2, 25))
				{
					nLine1++;
					nLine2++;
				}
				else
				{
					break;
				}
			}
		}
		FileSection * pSection = new FileSection;
		pSection->File1LineBegin = Line1Begin;

		pSection->File2LineBegin = Line2Begin;
		pSection->File1LineEnd = nLine1 - 1;
		pSection->File2LineEnd = nLine2 - 1;

	}
	return 1;
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


