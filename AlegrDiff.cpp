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
#include "PreferencesDialog.h"
#include "FolderDialog.h"
#include <Dlgs.h>

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
	ON_COMMAND(ID_FILE_PREFERENCES, OnFilePreferences)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IGNORE_WHITESPACES, OnUpdateViewIgnoreWhitespaces)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_LINE_NUMBERS, OnUpdateViewShowLineNumbers)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ACCEPT, OnUpdateEditAccept)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DECLINE, OnUpdateEditDecline)
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
	m_NormalTextColor(0),
	m_ErasedTextColor(0x000000FF),  // red
	m_AddedTextColor(0x00FF0000),   // blue
	m_AcceptedTextBackgroundColor(0x0000FFFF),  // yellow
	m_DiscardedTextBackgroundColor(0x00C0C0C0),  // dark gray
	m_TextBackgroundColor(0xFFFFFF),
	m_bRecurseSubdirs(false),
	m_FontPointSize(100),
	m_UsedFilenameFilter(0),
	m_AutoReloadChangedFiles(false),
	m_bCaseSensitive(true),
	m_bIgnoreWhitespaces(true),
	m_bFindBackward(false),
	m_MinimalLineLength(2),
	m_MinMatchingChars(3),
	m_NumberOfIdenticalLines(5),
	m_PercentsOfLookLikeDifference(30),
	m_FileListSort(CAlegrDiffView::ColumnSubdir),
	m_MinIdenticalLines(5)
{
	m_NormalLogFont.lfCharSet = ANSI_CHARSET;
	m_NormalLogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_NormalLogFont.lfEscapement = 0;
	m_NormalLogFont.lfHeight = -12;
	m_NormalLogFont.lfWidth = 0;
	m_NormalLogFont.lfItalic = FALSE;
	m_NormalLogFont.lfStrikeOut = FALSE;
	m_NormalLogFont.lfUnderline = FALSE;
	m_NormalLogFont.lfOrientation = 0;
	m_NormalLogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_NormalLogFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
	m_NormalLogFont.lfQuality = DEFAULT_QUALITY;
	m_NormalLogFont.lfWeight = FW_NORMAL;
	_tcsncpy(m_NormalLogFont.lfFaceName, _T("Courier New"), LF_FACESIZE);

	m_AddedLogFont = m_NormalLogFont;
	m_AddedLogFont.lfUnderline = TRUE;

	m_ErasedLogFont = m_NormalLogFont;
	m_ErasedLogFont.lfStrikeOut = TRUE;
}

CAlegrDiffApp::~CAlegrDiffApp()
{
}

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
	Profile.AddItem(_T("Settings"), _T("AcceptedTextBackgroundColor"), m_AcceptedTextBackgroundColor, 0x0000FF00, 0, 0xFFFFFF);
	Profile.AddItem(_T("Settings"), _T("DiscardedTextBackgroundColor"), m_DiscardedTextBackgroundColor, 0x00C0C0C0, 0, 0xFFFFFF);

	Profile.AddItem(_T("Settings"), _T("NormalFont"), m_NormalLogFont, m_NormalLogFont);
	Profile.AddItem(_T("Settings"), _T("AddedFont"), m_AddedLogFont, m_AddedLogFont);
	Profile.AddItem(_T("Settings"), _T("ErasedFont"), m_ErasedLogFont, m_ErasedLogFont);

	Profile.AddItem(_T("Settings"), _T("FontPointSize"), m_FontPointSize, 100, 30, 500);
	Profile.AddItem(_T("Settings"), _T("TabIndent"), m_TabIndent, 4, 1, 32);
	Profile.AddItem(_T("Settings"), _T("UsedFilenameFilter"), m_UsedFilenameFilter, 0, 0, 8);

	Profile.AddItem(_T("Settings"), _T("FileListSort"), m_FileListSort,
					CAlegrDiffView::ColumnSubdir,
					~CAlegrDiffView::ColumnComparisionResult,
					CAlegrDiffView::ColumnComparisionResult);

	Profile.AddItem(_T("Settings"), _T("RecurseSubdirs"), m_bRecurseSubdirs, false);
	Profile.AddItem(_T("Settings"), _T("InitialDir1"), m_FileDir1, _T(""));
	Profile.AddItem(_T("Settings"), _T("InitialDir2"), m_FileDir2, _T(""));
	Profile.AddItem(_T("Settings"), _T("LastSaveMergedDir"), m_LastSaveMergedDir, _T("."));
	Profile.AddItem(_T("Settings"), _T("CopyFilesDir"), m_CopyFilesDir, _T("."));

	//Profile.AddItem(_T("Settings"), _T("FilenameFilter"), m_sFilenameFilter, _T("*"));
	Profile.AddItem(_T("Settings"), _T("UseBinaryFilesFilter"), m_bUseBinaryFilesFilter, true);
	Profile.AddItem(_T("Settings"), _T("UseCppFilter"), m_bUseCppFilter, true);
	Profile.AddItem(_T("Settings"), _T("UseIgnoreFilter"), m_bUseIgnoreFilter, true);
	Profile.AddItem(_T("Settings"), _T("AdvancedCompareDialog"), m_bAdvancedCompareDialog, false);
	Profile.AddItem(_T("Settings"), _T("BinaryComparision"), m_BinaryComparision, false);
	Profile.AddItem(_T("Settings"), _T("AutoReloadChangedFiles"), m_AutoReloadChangedFiles, false);
	Profile.AddItem(_T("Settings"), _T("IgnoreWhitespaces"), m_bIgnoreWhitespaces, true);
	Profile.AddItem(_T("Settings"), _T("bShowLineNumbers"), m_bShowLineNumbers, false);
	Profile.AddItem(_T("Settings"), _T("FindBackward"), m_bFindBackward, false);

	Profile.AddItem(_T("Settings"), _T("MinimalLineLength"), m_MinimalLineLength, 2, 1, 2048);
	Profile.AddItem(_T("Settings"), _T("NumberOfIdenticalLines"), m_NumberOfIdenticalLines, 5, 1, 50);
	Profile.AddItem(_T("Settings"), _T("PercentsOfLookLikeDifference"), m_PercentsOfLookLikeDifference, 30, 0, 99);
	Profile.AddItem(_T("Settings"), _T("MinMatchingChars"), m_MinMatchingChars, 3, 1, 32);

	Profile.AddItem(_T("Settings"), _T("BinaryFiles"), m_sBinaryFilesFilter,
					_T("*.exe;*.dll;*.sys;*.obj;*.pdb;*.zip"));
	Profile.AddItem(_T("Settings"), _T("CppFiles"), m_sCppFilesFilter,
					_T("*.c;*.cpp;*.h;*.hpp;*.inl;*.rc;*.h++"));
	Profile.AddItem(_T("Settings"), _T("IgnoreFiles"), m_sIgnoreFilesFilter,
					_T("*.ncb"));

	for (int i = 0; i < sizeof m_sFindHistory / sizeof m_sFindHistory[0]; i++)
	{
		CString s;
		s.Format("find%d", i);
		Profile.AddItem(_T("History\\Find"), s, m_sFindHistory[i]);
	}

	m_TextBackgroundColor = GetSysColor(COLOR_WINDOW);
	m_SelectedTextColor = 0xFFFFFF;
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	m_pListDiffTemplate = new CMultiDocTemplate(
												IDR_ALEGRDTYPE_LIST,
												RUNTIME_CLASS(CAlegrDiffDoc),
												RUNTIME_CLASS(CChildFrame), // custom MDI child frame
												RUNTIME_CLASS(CAlegrDiffView));
	AddDocTemplate(m_pListDiffTemplate);

	m_pFileDiffTemplate = new CMultiDocTemplate(
												IDR_ALEGRDTYPE,
												RUNTIME_CLASS(CFilePairDoc),
												RUNTIME_CLASS(CChildFrame), // custom MDI child frame
												RUNTIME_CLASS(CDiffFileView));
	AddDocTemplate(m_pFileDiffTemplate);

	OnFontChanged();
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
#else
#endif
	// The main window has been initialized, so show and update it.
	m_pMainWnd->DragAcceptFiles();
	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
	pMainFrame->UpdateWindow();
	// process names from the command line
	ParseCommandLine();

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
	Profile.UnloadSection(NULL);
	return CWinApp::ExitInstance();
}

void CAlegrDiffApp::OnFileComparedirectories()
{
	CompareDirectories(NULL, NULL);
}

void CAlegrDiffApp::OpenFilePairView(FilePair * pPair)
{
	// check if there is already a CFilePairDoc
	POSITION position = m_pFileDiffTemplate->GetFirstDocPosition();
	while(position)
	{
		CFilePairDoc * pDoc =
			dynamic_cast<CFilePairDoc *>(m_pFileDiffTemplate->GetNextDoc(position));
		if (NULL != pDoc
			&& pDoc->GetFilePair() == pPair)
		{
			POSITION viewpos = pDoc->GetFirstViewPosition();
			if (position)
			{
				CView * pView = pDoc->GetNextView(viewpos);
				pView->GetParentFrame()->ActivateFrame();
			}
			return;
		}
	}

	CFilePairDoc * pDoc = (CFilePairDoc *)m_pFileDiffTemplate->OpenDocumentFile(NULL);

	if (NULL != pDoc)
	{
		pDoc->SetFilePair(pPair);
	}
}

void CAlegrDiffApp::OnFileComparefiles()
{
	CompareFiles(NULL, NULL);
}

void CAlegrDiffApp::OnFilePreferences()
{
	CPreferencesPropertySheet dlg;
	dlg.m_FilesPage.m_bUseBinaryFilesFilter = m_bUseBinaryFilesFilter;
	dlg.m_FilesPage.m_sBinaryFilesFilter = m_sBinaryFilesFilter;

	dlg.m_FilesPage.m_bUseCppFilter = m_bUseCppFilter;
	dlg.m_FilesPage.m_sCppFilesFilter = m_sCppFilesFilter;

	dlg.m_FilesPage.m_bUseIgnoreFilter = m_bUseIgnoreFilter;
	dlg.m_FilesPage.m_sIgnoreFilesFilter = m_sIgnoreFilesFilter;
	dlg.m_FilesPage.m_AutoReloadChangedFiles = m_AutoReloadChangedFiles;

	dlg.m_ViewPage.m_nTabIndent = m_TabIndent;

	dlg.m_ViewPage.m_NormalLogFont = m_NormalLogFont;
	dlg.m_ViewPage.m_NormalTextColor = m_NormalTextColor;
	dlg.m_ViewPage.m_AddedLogFont = m_AddedLogFont;
	dlg.m_ViewPage.m_AddedTextColor = m_AddedTextColor;
	dlg.m_ViewPage.m_ErasedLogFont = m_ErasedLogFont;
	dlg.m_ViewPage.m_ErasedTextColor = m_ErasedTextColor;
	dlg.m_ViewPage.m_FontPointSize = m_FontPointSize;
	dlg.m_ViewPage.m_NormalTextBackground = m_TextBackgroundColor;
	dlg.m_ViewPage.m_AddedTextBackground = m_AcceptedTextBackgroundColor;
	dlg.m_ViewPage.m_ErasedTextBackground = m_DiscardedTextBackgroundColor;

	dlg.m_ComparisionPage.m_MinimalLineLength = m_MinimalLineLength;
	dlg.m_ComparisionPage.m_NumberOfIdenticalLines = m_NumberOfIdenticalLines;
	dlg.m_ComparisionPage.m_PercentsOfLookLikeDifference = m_PercentsOfLookLikeDifference;
	dlg.m_ComparisionPage.m_MinMatchingChars = m_MinMatchingChars;

	if (IDOK == dlg.DoModal())
	{
		m_bUseBinaryFilesFilter = (0 != dlg.m_FilesPage.m_bUseBinaryFilesFilter);
		m_sBinaryFilesFilter = dlg.m_FilesPage.m_sBinaryFilesFilter;

		m_bUseCppFilter = (0 != dlg.m_FilesPage.m_bUseCppFilter);
		m_sCppFilesFilter = dlg.m_FilesPage.m_sCppFilesFilter;

		m_bUseIgnoreFilter = (0 != dlg.m_FilesPage.m_bUseIgnoreFilter);
		m_sIgnoreFilesFilter = dlg.m_FilesPage.m_sIgnoreFilesFilter;

		m_AutoReloadChangedFiles = dlg.m_FilesPage.m_AutoReloadChangedFiles;

		m_TabIndent = dlg.m_ViewPage.m_nTabIndent;
		m_MinimalLineLength = dlg.m_ComparisionPage.m_MinimalLineLength;
		m_NumberOfIdenticalLines = dlg.m_ComparisionPage.m_NumberOfIdenticalLines;
		m_PercentsOfLookLikeDifference = dlg.m_ComparisionPage.m_PercentsOfLookLikeDifference;
		m_MinMatchingChars = dlg.m_ComparisionPage.m_MinMatchingChars;

		if (dlg.m_ViewPage.m_bFontChanged)
		{
			m_NormalLogFont = dlg.m_ViewPage.m_NormalLogFont;
			m_NormalTextColor = dlg.m_ViewPage.m_NormalTextColor;

			m_AddedLogFont = dlg.m_ViewPage.m_AddedLogFont;
			m_AddedTextColor = dlg.m_ViewPage.m_AddedTextColor;

			m_ErasedLogFont = dlg.m_ViewPage.m_ErasedLogFont;
			m_ErasedTextColor = dlg.m_ViewPage.m_ErasedTextColor;

			m_FontPointSize = dlg.m_ViewPage.m_FontPointSize;
			OnFontChanged();
		}
		else if (dlg.m_ViewPage.m_bColorChanged)
		{
			m_TextBackgroundColor = dlg.m_ViewPage.m_NormalTextBackground;
			m_AcceptedTextBackgroundColor = dlg.m_ViewPage.m_AddedTextBackground;
			m_DiscardedTextBackgroundColor = dlg.m_ViewPage.m_ErasedTextBackground;

			UpdateAllDiffViews();
		}
	}
}

void CAlegrDiffApp::OnFontChanged()
{
	// recalculate font size
	CDC * pDC = CWnd::GetDesktopWindow()->GetWindowDC();
	m_NormalLogFont.lfHeight = -MulDiv(m_FontPointSize, pDC->GetDeviceCaps(LOGPIXELSY), 720);
	CWnd::GetDesktopWindow()->ReleaseDC(pDC);

	m_AddedLogFont.lfHeight = m_NormalLogFont.lfHeight;
	m_ErasedLogFont.lfHeight = m_NormalLogFont.lfHeight;

	m_NormalFont.DeleteObject();
	m_NormalFont.CreateFontIndirect( & m_NormalLogFont);

	m_AddedFont.DeleteObject();
	m_AddedFont.CreateFontIndirect( & m_AddedLogFont);

	m_ErasedFont.DeleteObject();
	m_ErasedFont.CreateFontIndirect( & m_ErasedLogFont);

	UpdateAllDiffViews(CFilePairDoc::FontChanged);
}

void CAlegrDiffApp::NotifyFilePairChanged(FilePair *pPair)
{
	POSITION position = m_pFileDiffTemplate->GetFirstDocPosition();
	while(position)
	{
		CFilePairDoc * pDoc =
			dynamic_cast<CFilePairDoc *>(m_pFileDiffTemplate->GetNextDoc(position));
		if (NULL != pDoc
			&& pPair == pDoc->GetFilePair())
		{
			pDoc->UpdateAllViews(NULL, CFilePairDoc::FileLoaded);
		}
	}
}

void CAlegrDiffApp::UpdateAllDiffViews(LPARAM lHint, CObject* pHint)
{
	POSITION position = m_pFileDiffTemplate->GetFirstDocPosition();
	while(position)
	{
		CFilePairDoc * pDoc =
			dynamic_cast<CFilePairDoc *>(m_pFileDiffTemplate->GetNextDoc(position));
		if (NULL != pDoc)
		{
			pDoc->UpdateAllViews(NULL, lHint, pHint);
		}
	}
}

UINT AFXAPI AfxGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
static void AFXAPI _AfxAbbreviateName(LPTSTR lpszCanon, int cchMax, BOOL bAtLeastName)
{
	int cchFullPath, cchFileName, cchVolName;
	const TCHAR* lpszCur;
	const TCHAR* lpszBase;
	const TCHAR* lpszFileName;

	lpszBase = lpszCanon;
	cchFullPath = lstrlen(lpszCanon);

	cchFileName = AfxGetFileName(lpszCanon, NULL, 0) - 1;
	lpszFileName = lpszBase + (cchFullPath-cchFileName);

	// If cchMax is more than enough to hold the full path name, we're done.
	// This is probably a pretty common case, so we'll put it first.
	if (cchMax >= cchFullPath)
		return;

	// If cchMax isn't enough to hold at least the basename, we're done
	if (cchMax < cchFileName)
	{
		lstrcpy(lpszCanon, (bAtLeastName) ? lpszFileName : &afxChNil);
		return;
	}

	// Calculate the length of the volume name.  Normally, this is two characters
	// (e.g., "C:", "D:", etc.), but for a UNC name, it could be more (e.g.,
	// "\\server\share").
	//
	// If cchMax isn't enough to hold at least <volume_name>\...\<base_name>, the
	// result is the base filename.

	lpszCur = lpszBase + 2;                 // Skip "C:" or leading "\\"

	if (lpszBase[0] == '\\' && lpszBase[1] == '\\') // UNC pathname
	{
		// First skip to the '\' between the server name and the share name,
		while (*lpszCur != '\\')
		{
			lpszCur = _tcsinc(lpszCur);
			ASSERT(*lpszCur != '\0');
		}
	}
	// if a UNC get the share name, if a drive get at least one directory
	ASSERT(*lpszCur == '\\');
	// make sure there is another directory, not just c:\filename.ext
	if (cchFullPath - cchFileName > 3)
	{
		lpszCur = _tcsinc(lpszCur);
		while (*lpszCur != '\\')
		{
			lpszCur = _tcsinc(lpszCur);
			ASSERT(*lpszCur != '\0');
		}
	}
	ASSERT(*lpszCur == '\\');

	cchVolName = lpszCur - lpszBase;
	if (cchMax < cchVolName + 5 + cchFileName)
	{
		lstrcpy(lpszCanon, lpszFileName);
		return;
	}

	// Now loop through the remaining directory components until something
	// of the form <volume_name>\...\<one_or_more_dirs>\<base_name> fits.
	//
	// Assert that the whole filename doesn't fit -- this should have been
	// handled earlier.

	ASSERT(cchVolName + (int)lstrlen(lpszCur) > cchMax);
	while (cchVolName + 4 + (int)lstrlen(lpszCur) > cchMax)
	{
		do
		{
			lpszCur = _tcsinc(lpszCur);
			ASSERT(*lpszCur != '\0');
		}
		while (*lpszCur != '\\');
	}

	// Form the resultant string and we're done.
	lpszCanon[cchVolName] = '\0';
	lstrcat(lpszCanon, _T("\\..."));
	lstrcat(lpszCanon, lpszCur);
}

void ModifyOpenFileMenu(CCmdUI* pCmdUI, class FileItem * pFile, UINT FormatID, UINT DisabledItemID)
{
	if (NULL == pFile)
	{
		CString s;
		s.LoadString(DisabledItemID);
		pCmdUI->SetText(s);
		// this works, too, but is a bit obscure
		//pCmdUI->SetText(LPCTSTR(CString().LoadString(DisabledItemID)));
		pCmdUI->Enable(FALSE);
		return;
	}
	CString name(pFile->GetFullName());
	_AfxAbbreviateName(name.GetBuffer(MAX_PATH * 2), 50, true);
	// duplicate all '&' in name
	for (int i = 0; i < name.GetLength(); i++)
	{
		if (name[i] == '&')
		{
			name.Insert(i, '&');
			i++;
		}
	}

	CString s;
	s.Format(FormatID, LPCTSTR(name));
	pCmdUI->SetText(s);
	pCmdUI->Enable(TRUE);
}

void OpenFileForEditing(class FileItem * pFile)
{
	if (NULL == pFile)
	{
		return;
	}
	CString name = pFile->GetFullName();
	SHELLEXECUTEINFO shex;
	memset( & shex, 0, sizeof shex);
	shex.cbSize = sizeof shex;
	shex.hwnd = AfxGetMainWnd()->m_hWnd;
	//shex.lpVerb = _T("Open");
	shex.lpFile = name;
	shex.nShow = SW_SHOWDEFAULT;
	ShellExecuteEx( & shex);
}

void CAlegrDiffApp::OpenSingleFile(LPCTSTR pName)
{
	WIN32_FIND_DATA wfd1;
	HANDLE hFind = FindFirstFile(pName, & wfd1);
	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		return;
	}
	FindClose(hFind);

	TCHAR FileDir1[MAX_PATH];
	LPTSTR pFileName1 = FileDir1;
	GetFullPathName(pName, MAX_PATH, FileDir1, & pFileName1);
	*pFileName1 = 0;

	CFilePairDoc * pDoc = (CFilePairDoc *)m_pFileDiffTemplate->OpenDocumentFile(NULL);
	if (NULL != pDoc)
	{
		FilePair * pPair = new FilePair;

		CString sCFilesPattern;
		if (m_bUseCppFilter)
		{
			sCFilesPattern = PatternToMultiCString(m_sCppFilesFilter);
		}
		pPair->pFirstFile = new FileItem( & wfd1, FileDir1, "",
										MultiPatternMatches(wfd1.cFileName, sCFilesPattern));

		pPair->pSecondFile = NULL;


		pDoc->SetFilePair(pPair);
		// SetFilePair references the pair, we need to compensate it
		pPair->Dereference();
	}
}

void CAlegrDiffApp::OnUpdateViewIgnoreWhitespaces(CCmdUI* pCmdUI)
{
	// if there is no handler, disable and uncheck
	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck(0);
}

void CAlegrDiffApp::OnUpdateViewShowLineNumbers(CCmdUI* pCmdUI)
{
	// if there is no handler, disable and uncheck
	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck(0);
}

void CAlegrDiffApp::OnUpdateEditAccept(CCmdUI* pCmdUI)
{
	// if there is no handler, disable and uncheck
	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck(0);
}

void CAlegrDiffApp::OnUpdateEditDecline(CCmdUI* pCmdUI)
{
	// if there is no handler, disable and uncheck
	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck(0);
}

void CAlegrDiffApp::ParseCommandLine()
{
	LPTSTR Arg1 = NULL;
	LPTSTR Arg2 = NULL;

	for (int i = 1; i < __argc; i++)
	{
		LPTSTR pszParam = __targv[i];
		if (pszParam[0] == '-' || pszParam[0] == '/')
		{
			// remove flag specifier
			++pszParam;
		}
		else
		{
			if (NULL == Arg1 || 0 == Arg1[0])
			{
				Arg1 = pszParam;
			}
			else if (NULL == Arg2 || 0 == Arg2[0])
			{
				Arg2 = pszParam;
			}
			else
			{
				//return;
			}
		}
	}
	if (NULL == Arg1 || 0 == Arg1[0])
	{
		return;
	}
	OpenPairOfPathnames(Arg1, Arg2);
}

		// compare folders. If only one folder specified, open dialog
void CAlegrDiffApp::CompareDirectories(LPCTSTR dir1, LPCTSTR dir2, LPCTSTR filter)
{
	CCompareDirsDialog dlg;
	dlg.m_bIncludeSubdirs = m_bRecurseSubdirs;
	dlg.m_bUseBinaryFilesFilter = m_bUseBinaryFilesFilter;
	dlg.m_sBinaryFilesFilter = m_sBinaryFilesFilter;

	dlg.m_bUseCppFilter = m_bUseCppFilter;
	dlg.m_sCppFilesFilter = m_sCppFilesFilter;

	dlg.m_bUseIgnoreFilter = m_bUseIgnoreFilter;
	dlg.m_sIgnoreFilesFilter = m_sIgnoreFilesFilter;

	dlg.m_nTabIndent = m_TabIndent;

	dlg.m_BinaryComparision = m_BinaryComparision;

	dlg.m_bAdvanced = m_bAdvancedCompareDialog;

	if (NULL != filter)
	{
		dlg.m_FilenameFilter = m_sFilenameFilter;
	}

	if (NULL != dir1)
	{
		dlg.m_sFirstDir = dir1;
	}

	if (NULL != dir2)
	{
		dlg.m_sSecondDir = dir2;
	}

	// run the dialog only if one of the directoriees is not specified
	if ( ! (dlg.m_sFirstDir.IsEmpty() || dlg.m_sSecondDir.IsEmpty())
		|| IDOK == dlg.DoModal())
	{
		m_sFilenameFilter = dlg.m_FilenameFilter;
		m_bRecurseSubdirs = (1 == dlg.m_bIncludeSubdirs);

		m_bUseBinaryFilesFilter = (0 != dlg.m_bUseBinaryFilesFilter);
		m_sBinaryFilesFilter = dlg.m_sBinaryFilesFilter;

		m_bUseCppFilter = (0 != dlg.m_bUseCppFilter);
		m_sCppFilesFilter = dlg.m_sCppFilesFilter;

		m_bUseIgnoreFilter = (0 != dlg.m_bUseIgnoreFilter);
		m_sIgnoreFilesFilter = dlg.m_sIgnoreFilesFilter;

		m_TabIndent = dlg.m_nTabIndent;

		m_BinaryComparision = dlg.m_BinaryComparision;

		m_bAdvancedCompareDialog = dlg.m_bAdvanced;

		CAlegrDiffDoc * pDoc = (CAlegrDiffDoc *)
								m_pListDiffTemplate->OpenDocumentFile(NULL);
		if (NULL == pDoc)
		{
			return;
		}
		pDoc->SetTitle("");

		if (pDoc->BuildFilePairList(dlg.m_sFirstDir, dlg.m_sSecondDir, m_bRecurseSubdirs))
		{
			pDoc->UpdateAllViews(NULL);
		}
		else
		{
			pDoc->OnCloseDocument();
		}
	}
}

void CAlegrDiffApp::CompareFiles(LPCTSTR pName1, LPCTSTR pName2)
{
	// TODO: check if there is already a CFilePairDoc
	CString Name1;
	CString Name2;
	if (pName1 != NULL)
	{
		Name1 = pName1;
	}

	if (pName2 != NULL)
	{
		Name2 = pName2;
	}


	CString Filter;
	Filter.LoadString(IDS_FILENAME_FILTER);

	CString title1;
	title1.LoadString(IDS_OPEN_FIRST_TITLE);
	COpenDiffDialog dlg1(TRUE, NULL, NULL,
						OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLETEMPLATE,
						Filter);
	dlg1.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_OPEN_TEMPLATE);

	dlg1.m_ofn.lpstrInitialDir = m_FileDir1;
	dlg1.m_ofn.lpstrTitle = title1;
	dlg1.m_ofn.nFilterIndex = m_UsedFilenameFilter;

	dlg1.m_bBinaryMode = m_BinaryComparision;

	if (Name1.IsEmpty())
	{
		if (IDOK != dlg1.DoModal())
		{
			return;
		}
		Name1 = dlg1.GetPathName();
		TCHAR CurrDir1[MAX_PATH] = {0};
		GetCurrentDirectory(MAX_PATH, CurrDir1);
		m_FileDir1 = CurrDir1;
	}

	CString title2;
	title2.LoadString(IDS_OPEN_SECOND_TITLE);

	COpenDiffDialog dlg2(TRUE, NULL, NULL,
						OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLETEMPLATE,
						Filter);

	dlg2.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_OPEN_TEMPLATE);
	dlg2.m_ofn.lpstrInitialDir = m_FileDir2;
	dlg2.m_ofn.lpstrTitle = title2;
	dlg2.m_ofn.nFilterIndex = dlg1.m_ofn.nFilterIndex;

	dlg2.m_bBinaryMode = dlg1.m_bBinaryMode;
	if (Name2.IsEmpty())
	{

		if (IDOK != dlg2.DoModal())
		{
			return;
		}


		TCHAR CurrDir2[MAX_PATH] = {0};
		GetCurrentDirectory(MAX_PATH, CurrDir2);
		m_FileDir2 = CurrDir2;

		Name2 = dlg2.GetPathName();
	}

	m_UsedFilenameFilter = dlg2.m_ofn.nFilterIndex;
	m_BinaryComparision = dlg2.m_bBinaryMode;

	TCHAR FileDir1[MAX_PATH];
	LPTSTR pFileName1 = FileDir1;
	GetFullPathName(Name1, MAX_PATH, FileDir1, & pFileName1);
	*pFileName1 = 0;

	TCHAR FileDir2[MAX_PATH];
	LPTSTR pFileName2 = FileDir2;
	GetFullPathName(Name2, MAX_PATH, FileDir2, & pFileName2);
	*pFileName2 = 0;


	WIN32_FIND_DATA wfd1;
	WIN32_FIND_DATA wfd2;
	HANDLE hFind = FindFirstFile(Name1, & wfd1);
	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		CString s;
		s.Format("Couldn't open \"%s\"", LPCTSTR(Name1));
		AfxMessageBox(s);
		return;
	}
	FindClose(hFind);

	hFind = FindFirstFile(Name2, & wfd2);
	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		CString s;
		s.Format("Couldn't open \"%s\"", LPCTSTR(Name2));
		AfxMessageBox(s);
		return;
	}
	FindClose(hFind);

	CFilePairDoc * pDoc = (CFilePairDoc *)m_pFileDiffTemplate->OpenDocumentFile(NULL);
	if (NULL != pDoc)
	{
		FilePair * pPair = new FilePair;

		CString sCFilesPattern;
		if (m_bUseCppFilter)
		{
			sCFilesPattern = PatternToMultiCString(m_sCppFilesFilter);
		}
		pPair->pFirstFile = new FileItem( & wfd1, FileDir1, "",
										MultiPatternMatches(wfd1.cFileName, sCFilesPattern));

		pPair->pSecondFile = new FileItem( & wfd2, FileDir2, "",
											MultiPatternMatches(wfd2.cFileName, sCFilesPattern));

		pDoc->SetFilePair(pPair);
		// SetFilePair references the pair, we need to compensate it
		pPair->Dereference();
	}
}

void CAlegrDiffApp::OpenPairOfPathnames(LPTSTR Arg1, LPTSTR Arg2)
{
	// check if the string contains '*' or '?'
	TRACE("CAlegrDiffApp::OpenPairOfPathnames: name1=%s, name2=%s\n", Arg1, Arg2);
	LPTSTR pArg1 = Arg1;
	CString filter;
	// find end of line
	while (*pArg1 != 0) { pArg1++; }

	bool hasWildcard = false;
	while (pArg1 >= Arg1)
	{
		if ('*' == *pArg1
			|| '?' == *pArg1)
		{
			hasWildcard = true;
		}
		if ('\\' == *pArg1
			|| ':' == *pArg1)
		{
			if (hasWildcard)
			{
				filter = pArg1+1;
				pArg1[1] = 0;
			}
			if ('\\' == *pArg1)
			{
				//*pArg = 0;
			}
			break;
		}
		pArg1--;
	}

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(Arg1, & wfd);
	if (NULL == hFind || INVALID_HANDLE_VALUE == hFind)
	{
		return;
	}
	FindClose(hFind);
	// check if it's folder or file
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (Arg2 != NULL && Arg2[0] != 0)
		{
			WIN32_FIND_DATA wfd2;
			hFind = FindFirstFile(Arg2, & wfd2);
			if (NULL == hFind
				|| INVALID_HANDLE_VALUE == hFind)
			{
				// TODO: show error or open dialog
				return;
			}
			FindClose(hFind);
			if (! (wfd2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				return;
			}
			if ( ! hasWildcard)
			{
				// find which folder is older
				if (0) if (wfd2.ftLastWriteTime.dwHighDateTime < wfd.ftLastWriteTime.dwHighDateTime
							|| (wfd2.ftLastWriteTime.dwHighDateTime == wfd.ftLastWriteTime.dwHighDateTime
								&& wfd2.ftLastWriteTime.dwLowDateTime < wfd.ftLastWriteTime.dwLowDateTime))
					{
						LPTSTR tmp = Arg1;
						Arg1 = Arg2;
						Arg2 = tmp;
					}
			}
		}
		// TODO: process /B (binary) option
		CompareDirectories(Arg1, Arg2, filter);
	}
	else
	{
		if (hasWildcard)
		{
			return;
		}
		// TODO: find which file is older
		// TODO: process /B (binary) option
		CompareFiles(Arg1, Arg2);
	}
}

void CopyFilesToFolder(FileItem **ppFiles, int nCount, bool bAddSubdirToTarget)
{
	CThisApp * pApp = GetApp();

	FileItem * pFile;

	CString TargetDir = pApp->m_CopyFilesDir;
	CString DlgTitle;
	DlgTitle.LoadString(IDS_COPY_FILES_TITLE);

	CFolderDialog dlg(DlgTitle, TargetDir, true);

	if (IDOK != dlg.DoModal())
	{
		return;
	}
	TargetDir = dlg.GetFolderPath();
	pApp->m_CopyFilesDir = TargetDir;

	if ( ! TargetDir.IsEmpty())
	{
		TCHAR c;
		c = TargetDir[TargetDir.GetLength() - 1];
		if (':' != c
			&& '\\' != c
			&& '/' != c)
		{
			TargetDir += _T("\\");
		}
	}
	int i;
	int SrcBufLen = 1;
	int DstBufLen = 1;
	for (i = 0; i < nCount; i++)
	{
		pFile = ppFiles[i];
		SrcBufLen += pFile->GetBasedirLength() + pFile->GetNameLength() + pFile->GetSubdirLength() + 1;
		DstBufLen += TargetDir.GetLength() + pFile->GetNameLength() + 1;
		if (bAddSubdirToTarget)
		{
			DstBufLen += pFile->GetSubdirLength();
		}
	}


	LPTSTR pSrcBuf = new TCHAR[SrcBufLen];
	LPTSTR pDstBuf = new TCHAR[SrcBufLen];

	if (NULL == pSrcBuf || NULL == pDstBuf)
	{
		delete[] pSrcBuf;
		delete[] pDstBuf;
		return;
	}
	int SrcBufIdx = 0;
	int DstBufIdx = 0;

	for (i = 0; i < nCount; i++)
	{
		pFile = ppFiles[i];
		_tcsncpy(pSrcBuf + SrcBufIdx, pFile->GetBasedir(), pFile->GetBasedirLength());
		SrcBufIdx += pFile->GetBasedirLength();

		_tcsncpy(pSrcBuf + SrcBufIdx, pFile->GetSubdir(), pFile->GetSubdirLength());
		SrcBufIdx += pFile->GetSubdirLength();

		_tcsncpy(pSrcBuf + SrcBufIdx, pFile->GetName(), pFile->GetNameLength());
		SrcBufIdx += pFile->GetNameLength();

		pSrcBuf[SrcBufIdx] = 0;
		SrcBufIdx++;

		_tcscpy(pDstBuf + DstBufIdx, TargetDir);
		DstBufIdx += TargetDir.GetLength();

		if (bAddSubdirToTarget)
		{
			_tcsncpy(pDstBuf + DstBufIdx, pFile->GetSubdir(), pFile->GetSubdirLength());
			DstBufIdx += pFile->GetSubdirLength();
		}

		_tcsncpy(pDstBuf + DstBufIdx, pFile->GetName(), pFile->GetNameLength());
		DstBufIdx += pFile->GetNameLength();

		pDstBuf[DstBufIdx] = 0;
		DstBufIdx++;
	}

	pSrcBuf[SrcBufIdx] = 0;
	pDstBuf[DstBufIdx] = 0;

	SHFILEOPSTRUCT fo;
	memset( & fo, 0, sizeof fo);
	fo.hwnd = AfxGetMainWnd()->m_hWnd;
	fo.wFunc = FO_COPY;
	fo.fFlags = FOF_MULTIDESTFILES | FOF_NOCONFIRMMKDIR;
	fo.pFrom = pSrcBuf;
	fo.pTo = pDstBuf;
	SHFileOperation( & fo);

	delete[ ] pSrcBuf;
	delete[ ] pDstBuf;

}
