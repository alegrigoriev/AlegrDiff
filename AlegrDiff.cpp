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
#include "FilesCompareDialog.h"
#include "PreferencesDialog.h"
#include "FolderDialog.h"
#include <locale.h>
#include <Dlgs.h>
#include "BinaryCompareDoc.h"
#include "BinaryCompareView.h"
#include "DirectoryFingerprintDlg.h"
#include "afxwin.h"
#include "AlegrDiffVer.h"
#include "DirectoryFingerpringCreateDlg.h"
#include "CheckFingerprintDlg.h"
#include "DirectoryFingerprintCheckDlg.h"

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
	ON_COMMAND(ID_HELP_USING, OnHelpUsing)
	//}}AFX_MSG_MAP
	// Standard file based document commands
//	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_FILE_CREATEDIRECTORYFINGERPRINT, OnFileCreatedirectoryfingerprint)
	ON_COMMAND(ID_FILE_CHECK_DIRECTORY_FINGERPRINT, OnFileCheckDirectoryFingerprint)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffApp construction

CAlegrDiffApp::CAlegrDiffApp()
	: m_MaxSearchDistance(256),
	m_pFileDiffTemplate(NULL),
	m_pListDiffTemplate(NULL),
	m_pBinaryDiffTemplate(NULL),
	m_TabIndent(4),
	m_NormalTextColor(0),
	m_ErasedTextColor(0x000000FF),  // red
	m_AddedTextColor(0x00FF0000),   // blue
	m_AcceptedTextBackgroundColor(0x0000FFFF),  // yellow
	m_DiscardedTextBackgroundColor(0x00C0C0C0),  // dark gray
	m_TextBackgroundColor(0xFFFFFF),
	m_FontPointSize(100),
	m_MinimalLineLength(2),
	m_MinMatchingChars(3),
	m_NumberOfIdenticalLines(5),
	m_MinPercentWeakIdenticalLines(10),
	m_PercentsOfLookLikeDifference(30),
	m_FileListSort(CAlegrDiffView::ColumnSubdir),
	m_MinIdenticalLines(5)
{
	m_PreferencesFlags = 0;
	// then init subfields:
	m_bRecurseSubdirs = false;
	m_bAdvancedCompareDialog = false;
	m_BinaryComparision = false;
	m_AutoReloadChangedFiles = false;
	m_bCaseSensitive = true;
	m_bIgnoreWhitespaces = true;
	m_bFindBackward = false;
	m_bShowLineNumbers = true;
	m_bCancelSelectionOnMerge = false;
	m_bUseMd5 = true;

	m_StatusFlags = 0;
	// then init subfields:
	m_bShowToolbar = true;
	m_bShowStatusBar = true;
	m_bOpenMaximized = true;
	m_bOpenChildMaximized = true;

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

/////////////////////////////////////////////////////////////////////////////
// The one and only CAlegrDiffApp object

CAlegrDiffApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffApp initialization

BOOL CAlegrDiffApp::InitInstance()
{
	char * Locale = setlocale(LC_ALL, ".ACP");
	TRACE("Locale set : %s\n", Locale);
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

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

	Profile.AddItem(_T("Settings"), _T("FileListSort"), m_FileListSort,
					CAlegrDiffView::ColumnSubdir, 0, ~0);

	Profile.AddItem(_T("Settings"), _T("InitialDir1"), m_FileDir1, _T(""));
	Profile.AddItem(_T("Settings"), _T("InitialDir2"), m_FileDir2, _T(""));
	Profile.AddItem(_T("Settings"), _T("LastSaveMergedDir"), m_LastSaveMergedDir, _T("."));
	Profile.AddItem(_T("Settings"), _T("CopyFilesDir"), m_CopyFilesDir, _T("."));

	//Profile.AddItem(_T("Settings"), _T("FilenameFilter"), m_sFilenameFilter, _T("*"));
#if 0
	Profile.AddItem(_T("Settings"), _T("UsedFilenameFilter"), m_UsedFilenameFilter, 0, 0, 8);
	Profile.AddItem(_T("Settings"), _T("RecurseSubdirs"), m_bRecurseSubdirs, false);
	Profile.AddItem(_T("Settings"), _T("UseBinaryFilesFilter"), m_bUseBinaryFilesFilter, false);
	Profile.AddItem(_T("Settings"), _T("UseCppFilter"), m_bUseCppFilter, true);
	Profile.AddItem(_T("Settings"), _T("UseIgnoreFilter"), m_bUseIgnoreFilter, true);
	Profile.AddItem(_T("Settings"), _T("AdvancedCompareDialog"), m_bAdvancedCompareDialog, false);
	Profile.AddItem(_T("Settings"), _T("BinaryComparision"), m_BinaryComparision, false);
	Profile.AddItem(_T("Settings"), _T("AutoReloadChangedFiles"), m_AutoReloadChangedFiles, false);
	Profile.AddItem(_T("Settings"), _T("IgnoreWhitespaces"), m_bIgnoreWhitespaces, true);
	Profile.AddItem(_T("Settings"), _T("ShowLineNumbers"), m_bShowLineNumbers, false);
	Profile.AddItem(_T("Settings"), _T("FindBackward"), m_bFindBackward, false);
	Profile.AddItem(_T("Settings"), _T("CancelSelectionOnMerge"), m_bCancelSelectionOnMerge, false);
	Profile.AddItem(_T("Settings"), _T("ShowToolbar"), m_bShowToolbar, true);
	Profile.AddItem(_T("Settings"), _T("ShowStatusBar"), m_bShowStatusBar, true);
	Profile.AddItem(_T("Settings"), _T("OpenChildMaximized"), m_bOpenChildMaximized, true);
	Profile.AddItem(_T("Settings"), _T("OpenMaximized"), m_bOpenMaximized, true);
#else
	Profile.AddItem(_T("Settings"), _T("PreferencesFlags"), m_PreferencesFlags, m_PreferencesFlags, 0, 0xFFFFFFFF);
	Profile.AddItem(_T("Settings"), _T("StatusFlags"), m_StatusFlags, m_StatusFlags, 0, 0xFFFFFFFF);
#endif


	Profile.AddItem(_T("Settings"), _T("MinimalLineLength"), m_MinimalLineLength, 2, 1, 2048);
	Profile.AddItem(_T("Settings"), _T("NumberOfIdenticalLines"), m_NumberOfIdenticalLines, 5, 1, 50);
	Profile.AddItem(_T("Settings"), _T("PercentsOfLookLikeDifference"), m_PercentsOfLookLikeDifference, 30, 0, 99);
	Profile.AddItem(_T("Settings"), _T("MinMatchingChars"), m_MinMatchingChars, 3, 1, 32);
	Profile.AddItem(_T("Settings"), _T("MinPercentWeakIdenticalLines"), m_MinPercentWeakIdenticalLines, 10, 0, 99);
	Profile.AddItem(_T("Settings"), _T("GoToLineFileSelection"), m_GoToLineFileSelection, 0, 0, 2);

	Profile.AddItem(_T("Settings"), _T("BinaryFiles"), m_sBinaryFilesFilter,
					_T("*.exe;*.dll;*.sys;*.obj;*.pdb;*.zip"));
	Profile.AddItem(_T("Settings"), _T("CppFiles"), m_sCppFilesFilter,
					_T("*.c;*.cpp;*.h;*.hpp;*.inl;*.rc;*.h++"));
	Profile.AddItem(_T("Settings"), _T("IgnoreFiles"), m_sIgnoreFilesFilter,
					_T("*.ncb"));

	LoadHistory(Profile, _T("History"), _T("find%d"), m_sFindHistory,
				sizeof m_sFindHistory / sizeof m_sFindHistory[0], false);
	LoadHistory(Profile, _T("History"), _T("dir%d"), m_RecentFolders,
				sizeof m_RecentFolders / sizeof m_RecentFolders[0], true);
	LoadHistory(Profile, _T("History"), _T("file%d"), m_RecentFiles,
				sizeof m_RecentFiles / sizeof m_RecentFiles[0], true);
	LoadHistory(Profile, _T("History"), _T("filter%d"), m_sFilters,
				sizeof m_sFilters / sizeof m_sFilters[0], true);

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

	m_pBinaryDiffTemplate = new CMultiDocTemplate(
												IDR_ALEGRDTYPE,
												RUNTIME_CLASS(CBinaryCompareDoc),
												RUNTIME_CLASS(CChildFrame), // custom MDI child frame
												RUNTIME_CLASS(CBinaryCompareView));
	AddDocTemplate(m_pBinaryDiffTemplate);

	OnFontChanged();
	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// The main window has been initialized, so show and update it.
	m_pMainWnd->DragAcceptFiles();
	int nCmdShow = SW_SHOWDEFAULT;
	if (m_bOpenMaximized)
	{
		nCmdShow = SW_SHOWMAXIMIZED;
	}
	pMainFrame->ShowWindow(nCmdShow);

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
	afx_msg void OnButtonMailto();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_sVersion;
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
	DDX_Control(pDX, IDC_STATIC_VERSION, m_sVersion);
	if ( ! pDX->m_bSaveAndValidate)
	{
		CString format, s;
		m_sVersion.GetWindowText(format);
		s.Format(format, ALEGR_DIFF_VERSION);
		m_sVersion.SetWindowText(s);
	}
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_BUTTON_MAILTO, OnButtonMailto)
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

CDocument * CAlegrDiffApp::OpenFilePairView(FilePair * pPair)
{
	if (pPair->m_ComparisionResult == pPair->ResultUnknown
		|| (pPair->pFirstFile != NULL && pPair->pFirstFile->IsFolder())
		|| (pPair->pSecondFile != NULL && pPair->pSecondFile->IsFolder()))
	{
		// the file not compared yet, can't open
		return NULL;
	}
	// check if there is already a CFilePairDoc
	POSITION position = m_pFileDiffTemplate->GetFirstDocPosition();
	while(position)
	{
		CDocument * pJustDoc = m_pFileDiffTemplate->GetNextDoc(position);
		CFilePairDoc * pDoc =
			dynamic_cast<CFilePairDoc *>(pJustDoc);
		if (NULL != pDoc
			&& pDoc->GetFilePair() == pPair)
		{
			POSITION viewpos = pDoc->GetFirstViewPosition();
			if (viewpos)
			{
				CView * pView = pDoc->GetNextView(viewpos);
				((CMDIChildWnd*)pView->GetParentFrame())->MDIActivate();
			}
			return pDoc;
		}
		else
		{
			CBinaryCompareDoc * pDoc =
				dynamic_cast<CBinaryCompareDoc *>(pJustDoc);
			if (NULL != pDoc
				&& pDoc->GetFilePair() == pPair)
			{
				POSITION viewpos = pDoc->GetFirstViewPosition();
				if (viewpos)
				{
					CView * pView = pDoc->GetNextView(viewpos);
					((CMDIChildWnd*)pView->GetParentFrame())->MDIActivate();
				}
				return pDoc;
			}
		}
	}

	if (pPair->pFirstFile->m_IsBinary
		|| pPair->pSecondFile->m_IsBinary)
	{
		CBinaryCompareDoc * pDoc = (CBinaryCompareDoc *)m_pBinaryDiffTemplate->OpenDocumentFile(NULL);

		if (NULL != pDoc)
		{
			pDoc->SetFilePair(pPair);
		}
		return pDoc;
	}
	else
	{
		CFilePairDoc * pDoc = (CFilePairDoc *)m_pFileDiffTemplate->OpenDocumentFile(NULL);

		if (NULL != pDoc)
		{
			pDoc->SetFilePair(pPair);
		}
		return pDoc;
	}
}

void CAlegrDiffApp::OnFileComparefiles()
{
	CompareFiles(NULL, NULL);
}

void CAlegrDiffApp::OnFilePreferences()
{
	CPreferencesPropertySheet dlg;
	dlg.m_FilesPage.m_sBinaryFilesFilter = m_sBinaryFilesFilter;

	dlg.m_FilesPage.m_sCppFilesFilter = m_sCppFilesFilter;

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
	dlg.m_ViewPage.m_bCancelSelectionOnMerge = m_bCancelSelectionOnMerge;

	dlg.m_ComparisionPage.m_MinimalLineLength = m_MinimalLineLength;
	dlg.m_ComparisionPage.m_NumberOfIdenticalLines = m_NumberOfIdenticalLines;
	dlg.m_ComparisionPage.m_PercentsOfLookLikeDifference = m_PercentsOfLookLikeDifference;
	dlg.m_ComparisionPage.m_MinMatchingChars = m_MinMatchingChars;
	dlg.m_ComparisionPage.m_bUseMd5 = m_bUseMd5;

	if (IDOK == dlg.DoModal())
	{
		m_sBinaryFilesFilter = dlg.m_FilesPage.m_sBinaryFilesFilter;

		m_sCppFilesFilter = dlg.m_FilesPage.m_sCppFilesFilter;

		m_sIgnoreFilesFilter = dlg.m_FilesPage.m_sIgnoreFilesFilter;

		m_AutoReloadChangedFiles = (0 != dlg.m_FilesPage.m_AutoReloadChangedFiles);

		m_TabIndent = dlg.m_ViewPage.m_nTabIndent;
		m_bCancelSelectionOnMerge = (0 != dlg.m_ViewPage.m_bCancelSelectionOnMerge);

		m_MinimalLineLength = dlg.m_ComparisionPage.m_MinimalLineLength;
		m_NumberOfIdenticalLines = dlg.m_ComparisionPage.m_NumberOfIdenticalLines;
		m_PercentsOfLookLikeDifference = dlg.m_ComparisionPage.m_PercentsOfLookLikeDifference;
		m_MinMatchingChars = dlg.m_ComparisionPage.m_MinMatchingChars;
		m_bUseMd5 = 0 != dlg.m_ComparisionPage.m_bUseMd5;

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

	UpdateAllDiffViews(CFilePairDoc::MetricsChanged);
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
		lstrcpy(lpszCanon, (bAtLeastName) ? lpszFileName : _T(""));
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
	memzero(shex);
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

	bool bCppFile = false;
	bool bBinaryFile = false;
	if ( ! m_sBinaryFilesFilter.IsEmpty())
	{
		bBinaryFile = MultiPatternMatches(wfd1.cFileName,
										PatternToMultiCString(m_sBinaryFilesFilter));
	}
	if ( ! bBinaryFile
		&& ! m_sCppFilesFilter.IsEmpty())
	{
		bCppFile = MultiPatternMatches(wfd1.cFileName,
										PatternToMultiCString(m_sCppFilesFilter));
	}

	FilePair * pPair = new FilePair;
	pPair->pFirstFile = new FileItem( & wfd1, FileDir1, "");
	pPair->pSecondFile = NULL;
	pPair->pFirstFile->m_C_Cpp = bCppFile;
	pPair->pFirstFile->m_IsBinary = bBinaryFile;

	if (bBinaryFile)
	{
		CBinaryCompareDoc * pDoc = (CBinaryCompareDoc *)m_pBinaryDiffTemplate->OpenDocumentFile(NULL);
		if (NULL != pDoc)
		{
			pDoc->SetFilePair(pPair);
			// SetFilePair references the pair, we need to compensate it
		}
		pPair->Dereference();
	}
	else
	{
		CFilePairDoc * pDoc = (CFilePairDoc *)m_pFileDiffTemplate->OpenDocumentFile(NULL);
		if (NULL != pDoc)
		{
			pDoc->SetFilePair(pPair);
			// SetFilePair references the pair, we need to compensate it
		}
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
	dlg.m_sBinaryFilesFilter = m_sBinaryFilesFilter;

	dlg.m_sCppFilesFilter = m_sCppFilesFilter;

	dlg.m_sIgnoreFilesFilter = m_sIgnoreFilesFilter;

	dlg.m_nTabIndent = m_TabIndent;

	dlg.m_BinaryComparision = m_BinaryComparision;

	dlg.m_bAdvanced = m_bAdvancedCompareDialog;
	dlg.m_bUseMd5 = m_bUseMd5;

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
	if (IDOK == dlg.DoModal())
	{
		m_sFilenameFilter = dlg.m_FilenameFilter;
		m_bRecurseSubdirs = (1 == dlg.m_bIncludeSubdirs);

		m_sBinaryFilesFilter = dlg.m_sBinaryFilesFilter;

		m_sCppFilesFilter = dlg.m_sCppFilesFilter;

		m_sIgnoreFilesFilter = dlg.m_sIgnoreFilesFilter;

		m_TabIndent = dlg.m_nTabIndent;

		m_BinaryComparision = (0 != dlg.m_BinaryComparision);

		m_bAdvancedCompareDialog = dlg.m_bAdvanced;

		m_bUseMd5 = dlg.m_bUseMd5 != 0;

		CAlegrDiffDoc * pDoc = (CAlegrDiffDoc *)
								m_pListDiffTemplate->OpenDocumentFile(NULL);
		if (NULL == pDoc)
		{
			return;
		}
		pDoc->SetTitle(_T(""));

		if (pDoc->BuildFilePairList(dlg.m_sFirstDir, dlg.m_sSecondDir,
									m_bRecurseSubdirs, m_BinaryComparision))
		{
			pDoc->RunComparisionThread();
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

	if (Name1.IsEmpty()
		|| Name2.IsEmpty())
	{
		CFilesCompareDialog dlg;

		dlg.m_sFirstFileName = Name1;
		dlg.m_sSecondFileName = Name2;
		dlg.m_bBinaryFile = m_BinaryComparision;

		if (IDOK != dlg.DoModal())
		{
			return;
		}

		m_BinaryComparision = (0 != dlg.m_bBinaryFile);
		Name1 = dlg.m_sFirstFileName;
		Name2 = dlg.m_sSecondFileName;
	}

	TCHAR FileDir1[MAX_PATH];
	LPTSTR pFileName1 = FileDir1;
	GetFullPathName(Name1, MAX_PATH, FileDir1, & pFileName1);

	TCHAR FileDir2[MAX_PATH];
	LPTSTR pFileName2 = FileDir2;
	GetFullPathName(Name2, MAX_PATH, FileDir2, & pFileName2);

	WIN32_FIND_DATA wfd1;
	WIN32_FIND_DATA wfd2;
	HANDLE hFind = FindFirstFile(Name1, & wfd1);
	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		CString s;
		s.Format(IDS_STRING_CANT_OPEN_FILE, LPCTSTR(Name1));
		AfxMessageBox(s);
		return;
	}
	FindClose(hFind);

	hFind = FindFirstFile(Name2, & wfd2);
	if (INVALID_HANDLE_VALUE == hFind
		|| NULL == hFind)
	{
		CString s;
		s.Format(IDS_STRING_CANT_OPEN_FILE, LPCTSTR(Name2));
		AfxMessageBox(s);
		return;
	}
	FindClose(hFind);

	AddStringToHistory(FileDir2, m_RecentFiles,
						sizeof m_RecentFiles / sizeof m_RecentFiles[0], false);
	AddStringToHistory(FileDir1, m_RecentFiles,
						sizeof m_RecentFiles / sizeof m_RecentFiles[0], false);

	*pFileName1 = 0;
	*pFileName2 = 0;
	FilePair * pPair = new FilePair;

	bool bFilesBinary = m_BinaryComparision;

	CString sCFilesPattern(PatternToMultiCString(m_sCppFilesFilter));
	CString sBinFilesPattern(PatternToMultiCString(m_sBinaryFilesFilter));

	if ( ! bFilesBinary
		&& ! m_sBinaryFilesFilter.IsEmpty())
	{
		bFilesBinary = MultiPatternMatches(wfd1.cFileName, sBinFilesPattern)
						|| MultiPatternMatches(wfd2.cFileName, sBinFilesPattern);
	}

	pPair->pFirstFile = new FileItem( & wfd1, FileDir1, "");
	pPair->pFirstFile->m_IsBinary = bFilesBinary;

	pPair->pSecondFile = new FileItem( & wfd2, FileDir2, "");
	pPair->pSecondFile->m_IsBinary = bFilesBinary;

	if ( ! bFilesBinary
		&& ! m_sCppFilesFilter.IsEmpty())
	{
		pPair->pFirstFile->m_C_Cpp =
			MultiPatternMatches(wfd1.cFileName, sCFilesPattern);
		pPair->pSecondFile->m_C_Cpp =
			MultiPatternMatches(wfd2.cFileName, sCFilesPattern);
	}

	if (bFilesBinary)
	{
		CBinaryCompareDoc * pDoc = (CBinaryCompareDoc *)m_pFileDiffTemplate->OpenDocumentFile(NULL);
		if (NULL != pDoc)
		{
			pDoc->SetFilePair(pPair);
			// SetFilePair references the pair, we need to compensate it
		}
		pPair->Dereference();
	}
	else
	{
		CFilePairDoc * pDoc = (CFilePairDoc *)m_pFileDiffTemplate->OpenDocumentFile(NULL);
		if (NULL != pDoc)
		{
			pDoc->SetFilePair(pPair);
			// SetFilePair references the pair, we need to compensate it
		}
		pPair->Dereference();
	}
}

void CAlegrDiffApp::OpenPairOfPathnames(LPTSTR Arg1, LPTSTR Arg2)
{
	// check if the string contains '*' or '?'
	TRACE(_T("CAlegrDiffApp::OpenPairOfPathnames: name1=%s, name2=%s\n"), Arg1, Arg2);
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

	// check if it's folder or file
	// don't use FildFirstFile, because it won't work for the root directory
	DWORD FileAttr1 = GetFileAttributes(Arg1);
	if (0xFFFFFFFF == FileAttr1)
	{
		// failed
		return;
	}
	if (FileAttr1 & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (Arg2 != NULL && Arg2[0] != 0)
		{
			DWORD FileAttr2 = GetFileAttributes(Arg2);
			if (0xFFFFFFFF == FileAttr2)
			{
				// failed
				return;
			}
			if (0 == (FileAttr2 & FILE_ATTRIBUTE_DIRECTORY))
			{
				return;
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
	LPTSTR pDstBuf = new TCHAR[DstBufLen];

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
	memzero(fo);
	fo.hwnd = AfxGetMainWnd()->m_hWnd;
	fo.wFunc = FO_COPY;
	fo.fFlags = FOF_MULTIDESTFILES | FOF_NOCONFIRMMKDIR;
	fo.pFrom = pSrcBuf;
	fo.pTo = pDstBuf;
	SHFileOperation( & fo);

	delete[ ] pSrcBuf;
	delete[ ] pDstBuf;

}

CString FileTimeToStr(FILETIME FileTime, LCID locale)
{
	int const TimeBufSize = 256;
	TCHAR str[TimeBufSize] = {0};
	SYSTEMTIME SystemTime;
	SYSTEMTIME LocalTime;
	memzero(LocalTime);
	FileTimeToSystemTime( & FileTime, & SystemTime);
	SystemTimeToTzSpecificLocalTime(NULL, & SystemTime, & LocalTime);

	GetDateFormat(locale, DATE_SHORTDATE, & LocalTime, NULL, str, TimeBufSize - 1);
	CString result = str;
	result += ' ';

	GetTimeFormat(locale, TIME_NOSECONDS, & LocalTime, NULL, str, TimeBufSize - 1);
	result += str;
	return result;
}


void CAboutDlg::OnButtonMailto()
{
	SHELLEXECUTEINFO shex;
	memzero(shex);
	shex.cbSize = sizeof shex;
	shex.hwnd = NULL;//AfxGetMainWnd()->m_hWnd;

	CString Subj;
	CWnd * pWnd = GetDlgItem(IDC_STATIC_VERSION);
	if (NULL != pWnd)
	{
		pWnd->GetWindowText(Subj);
	}
	else
	{
		Subj = _T("AlegrDiff");
	}
	CString file(_T("mailto:alegr@earthlink.net?Subject="));
	file += Subj;
	shex.lpFile = file;
	shex.nShow = SW_SHOWDEFAULT;
	ShellExecuteEx( & shex);
	EndDialog(IDOK);
}

void AddStringToHistory(const CString & str, CString history[], int NumItems, bool CaseSensitive)
{
	// remove those that match the currently selected dirs
	int i, j;
	for (i = 0, j = 0; i < NumItems; i++)
	{
		if (CaseSensitive)
		{
			if (0 == str.Compare(history[i])
				// check if previous string is the same
				|| j > 0
				&& 0 == history[j - 1].Compare(history[i]))
			{
				continue;
			}
		}
		else
		{
			if (0 == str.CompareNoCase(history[i])
				// check if previous string is the same
				|| j > 0
				&& 0 == history[j - 1].CompareNoCase(history[i]))
			{
				continue;
			}
		}
		if (i != j)
		{
			history[j] = history[i];
		}
		j++;
	}
	for (; j < NumItems; j++)
	{
		history[j].Empty();
	}

	// remove last dir from the list
	for (i = NumItems - 1; i >= 1; i--)
	{
		history[i] = history[i - 1];
	}
	history[0] = str;
}

void LoadHistory(CApplicationProfile & Profile, LPCTSTR szKey, LPCTSTR Format, CString history[], int NumItems, bool Trim)
{
	for (int i = 0; i < NumItems; i++)
	{
		CString s;
		s.Format(Format, i);
		Profile.AddItem(szKey, s, history[i]);
		if (Trim)
		{
			history[i].Trim();
		}
	}
}

void LoadHistoryCombo(CComboBox & Combo, CString history[], int NumItems)
{
	for (int i = 0; i < NumItems; i++)
	{
		if (! history[i].IsEmpty())
		{
			Combo.AddString(history[i]);
		}
	}
}

void CAlegrDiffApp::OnHelpUsing()
{
	TCHAR ModuleName[MAX_PATH] = {0};
	TCHAR FullPathName[MAX_PATH];
	LPTSTR FilePart = FullPathName;
	GetModuleFileName(NULL, ModuleName, MAX_PATH);
	GetFullPathName(ModuleName, MAX_PATH, FullPathName, & FilePart);
	*FilePart = 0;
	CString HelpfileName(FullPathName);
	HelpfileName += _T("AlegrDiff.mht");

	SHELLEXECUTEINFO shex;
	memzero(shex);
	shex.cbSize = sizeof shex;
	shex.hwnd = NULL;
	//shex.lpVerb = _T("Open");
	shex.lpFile = HelpfileName;
	shex.nShow = SW_SHOWDEFAULT;
	ShellExecuteEx( & shex);

}

CString CreateCustomFilter(LPCTSTR Extension)
{
	// DotExt points to "ext" string
	if ('*' == Extension[0]
		&& '.' == Extension[1])
	{
		Extension += 2;
	}
	else if ('.' == Extension[0])
	{
		Extension++;
	}
	CString HccrRegistryPath('.');
	HccrRegistryPath += Extension;
	CString WildCard("*" + HccrRegistryPath);
	CString Filter(" (" + WildCard + ")|" + WildCard + "|");
	CString ReturnValue(HccrRegistryPath + " file" + Filter);

	HKEY hKey = NULL;
	TCHAR data[512];
	DWORD ValueType;
	DWORD DataSize = sizeof data - 2;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, HccrRegistryPath, 0, KEY_READ,
					&hKey) == ERROR_SUCCESS)
	{
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, NULL, 0, & ValueType, LPBYTE(data), & DataSize)
			&& REG_SZ == ValueType)
		{
			data[DataSize / sizeof data[0]] = 0;
			HKEY hKey1 = NULL;
			if (RegOpenKeyEx(HKEY_CLASSES_ROOT, data, 0, KEY_READ,
							&hKey1) == ERROR_SUCCESS)
			{
				// read file description
				DataSize = sizeof data - 2;
				if (ERROR_SUCCESS == RegQueryValueEx(hKey1, NULL, 0, & ValueType, LPBYTE(data), & DataSize)
					&& REG_SZ == ValueType)
				{
					data[DataSize / sizeof data[0]] = 0;
					ReturnValue = data + Filter;
				}
				RegCloseKey(hKey1);
			}

		}
		RegCloseKey(hKey);
	}
	return ReturnValue;
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
		TRACE(_T("COpenDiffDialog::OnComboSelendOK: %s selected\n"), LPCTSTR(str));
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
			CWnd * pTmp = pParent->GetDlgItem(edt1);
			if (NULL == pTmp)
			{
				// new style dialog
				pTmp = pParent->GetDlgItem(cmb13);
			}
			if (NULL != pTmp)
			{
				pTmp->SetFocus();
			}
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
	TRACE(_T("COpenDiffDialog::OnFileNameOK Folder Path=%s\n"), LPCTSTR(sCurrDir));

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

int BrowseForFile(int TitleID, CString & Name, CString & BrowseFolder,
				CString const * pHistory, int HistorySize)
{
	CThisApp * pApp = GetApp();

	// Filter string provided by the program (AllFiles (*.*))
	CString AllFilesFilter;
	AllFilesFilter.LoadString(IDS_FILENAME_FILTER);

	vector<CString> Filters;
	CString Filter;
	// Filter string generated from the current file name
	// it will then be stored to pApp->m_CustomFileOpenFilter
	CString CurrentCustomFilterString;
	// Filter string used from the previous time
	CString PrevCustomFilterString;
	if ( ! pApp->m_CustomFileOpenFilter.IsEmpty())
	{
		Filters.push_back(CreateCustomFilter(pApp->m_CustomFileOpenFilter));
	}
	// get file name extension
	// create custom filter

	CString title;
	title.LoadString(TitleID);

	TCHAR FullPath[MAX_PATH + 1] = {0};
	LPTSTR FileNamePart = FullPath;
	TCHAR OfnCustomFilter[MAX_PATH] = {0, 0};
	CString LastFileName;

	if ( ! Name.IsEmpty()
		&& GetFullPathName(Name, MAX_PATH, FullPath, & FileNamePart))
	{
		LastFileName = FileNamePart;
		// find extension
		LPCTSTR pExt = _tcsrchr(FileNamePart, '.');
		if (pExt)
		{
			CurrentCustomFilterString = CreateCustomFilter(pExt);
			if ( ! Filters.empty()
				&& 0 != Filters[0].CompareNoCase(CurrentCustomFilterString))
			{
				Filters.push_back(CurrentCustomFilterString);
			}
		}
		*FileNamePart = 0;
	}

	if (0 != pHistory)
	{
		for (int i = 0; i < HistorySize; i++)
		{
			TCHAR FullPath[MAX_PATH];
			LPTSTR FileNamePart;
			if ( ! pHistory[i].IsEmpty()
				&& GetFullPathName(pHistory[i], MAX_PATH, FullPath, & FileNamePart))
			{
				// find extension
				LPCTSTR pExt = _tcsrchr(FileNamePart, '.');
				if (pExt)
				{
					CurrentCustomFilterString = CreateCustomFilter(pExt);
					unsigned j;
					for (j = 0; j < Filters.size(); j++)
					{
						if (0 == Filters[0].CompareNoCase(CurrentCustomFilterString))
						{
							break;
						}
					}
					if (j == Filters.size())
					{
						Filters.push_back(CurrentCustomFilterString);
					}
				}
			}

		}
	}

	for (unsigned j = 0; j < Filters.size(); j++)
	{
		Filter += Filters[j];
	}

	COpenDiffDialog dlg(TRUE, NULL, NULL,
						OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLETEMPLATE,
						Filter);
	// copy initial file name
	_tcsncpy(dlg.m_ofn.lpstrFile, LastFileName, dlg.m_ofn.nMaxFile - 1);
	dlg.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_OPEN_TEMPLATE);

	if (FullPath[0] == 0)
	{
		dlg.m_ofn.lpstrInitialDir = BrowseFolder;
	}
	else
	{
		dlg.m_ofn.lpstrInitialDir = FullPath;
	}

	dlg.m_ofn.lpstrTitle = title;
	dlg.m_ofn.nFilterIndex = 1;
	dlg.m_ofn.lpstrCustomFilter = OfnCustomFilter;
	dlg.m_ofn.nMaxCustFilter = sizeof OfnCustomFilter / sizeof OfnCustomFilter[0];

	if (IDOK != dlg.DoModal())
	{
		return IDCANCEL;
	}

	TCHAR CurrDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, CurrDir);
	BrowseFolder = CurrDir;

	LPTSTR pFilter = OfnCustomFilter;
	// skip first string (it's usually of sero length
	while (*(pFilter++) != 0) {}
	if (0 != _tcscmp(pFilter, _T("*"))
		&& 0 != _tcscmp(pFilter, _T("*.*")))
	{
		int len = _tcslen(pFilter);
		// file open dialog adds an asterisk to the filter:
		// *.c becomes *.c*
		// we want to remove it
		if (len > 3
			&& '*' == pFilter[len-1])
		{
			pFilter[len-1] = 0;
			if (NULL == _tcschr(pFilter, '*')
				&& NULL == _tcschr(pFilter, '?'))
			{
				// put the asterisk back
				pFilter[len-1] = '*';
			}
		}
		pApp->m_CustomFileOpenFilter = pFilter;
	}

	Name = dlg.GetPathName();
	return IDOK;
}

void CAlegrDiffApp::OnFileCreatedirectoryfingerprint()
{
	CDirectoryFingerprintDlg dlg;
	dlg.m_bIncludeSubdirectories = m_bRecurseSubdirs;

	dlg.m_sIgnoreFiles = m_sIgnoreFilesFilter;
	dlg.m_sFilenameFilter = m_sFilenameFilter;

	if (IDOK != dlg.DoModal())
	{
		return;
	}

	m_bRecurseSubdirs = dlg.m_bIncludeSubdirectories;
	m_sFilenameFilter = dlg.m_sFilenameFilter;

	m_sIgnoreFilesFilter = dlg.m_sIgnoreFiles;

	CDirectoryFingerpringCreateDlg dlg1;

	dlg1.m_bIncludeDirectoryStructure = dlg.m_bIncludeDirectoryStructure;
	dlg1.m_bIncludeSubdirectories = dlg.m_bIncludeSubdirectories;
	dlg1.m_sFilenameFilter = dlg.m_sFilenameFilter;
	dlg1.m_sIgnoreFiles = dlg.m_sIgnoreFiles;
	dlg1.m_sDirectory = dlg.m_sDirectory;
	dlg1.m_FingerprintFilename = dlg.m_sSaveFilename;
	dlg1.m_bSaveAsUnicode = dlg.m_bSaveAsUnicode;

	dlg1.DoModal();
}


void CAlegrDiffApp::OnFileCheckDirectoryFingerprint()
{
	CCheckFingerprintDlg dlg;
	if (IDOK != dlg.DoModal())
	{
		return;
	}

	CAlegrDiffDoc * pDoc = (CAlegrDiffDoc *)
							m_pListDiffTemplate->OpenDocumentFile(NULL);
	if (NULL == pDoc)
	{
		return;
	}
	pDoc->SetTitle(_T(""));
	CDirectoryFingerprintCheckDlg dlg1(pDoc);

	dlg1.m_sDirectory = dlg.m_sDirectory;
	dlg1.m_FingerprintFilename = dlg.m_sFilename;

	if (IDOK != dlg1.DoModal())
	{
		delete pDoc;
		return;
	}
}
