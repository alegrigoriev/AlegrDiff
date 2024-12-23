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

#include "PathEx.h"
#include <Shlwapi.h>
#include <atlbase.h>
#include <locale.h>

#include "FileDialogWithHistory.h"

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
	ON_COMMAND(ID_WINDOW_CLOSEALL, OnWindowCloseall)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffApp construction

CAlegrDiffApp::CAlegrDiffApp()
	: m_MaxSearchDistance(256),
	m_pFileDiffTemplate(NULL),
	m_pListDiffTemplate(NULL),
	m_pBinaryDiffTemplate(NULL),
	m_TabIndent(4),
	m_FontPointSize(100),
	m_MinimalLineLength(2),
	m_MinMatchingChars(3),
	m_NumberOfIdenticalLines(5),
	m_MinPercentWeakIdenticalLines(10),
	m_PercentsOfLookLikeDifference(30),
	m_ShowFilesMask(0xFFFFFFFF & ~(1 << FilePair::DirectoriesBothPresent)),

	m_RecentFolders( & Profile, _T("History"), _T("dir%d"), 20),
	m_FindHistory( & Profile, _T("History"), _T("find%d"), 15, CStringHistory::CaseSensitive),
	m_FileFilters( & Profile, _T("History"), _T("filter%d"), 10),
	m_RecentFiles( & Profile, _T("History"), _T("file%d"), 20),

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
	m_bFindWholeWord = false;

	m_StatusFlags = 0;
	// then init subfields:
	m_bShowToolbar = true;
	m_bShowStatusBar = true;

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
	_tcsncpy_s(m_NormalLogFont.lfFaceName, countof(m_NormalLogFont.lfFaceName), _T("Courier New"), LF_FACESIZE);

	m_AddedLogFont = m_NormalLogFont;

	m_ErasedLogFont = m_NormalLogFont;

	EnableHtmlHelp();
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
	InitCommonControls();

	CWinApp::InitInstance();

	WCHAR CodePageStr[64] = L".";
	// Fetch CP_ACP (system default code page) because it's what Notepad uses to read and write text files
	if (GetLocaleInfoW(LOCALE_SYSTEM_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, CodePageStr+1, sizeof CodePageStr / sizeof CodePageStr[0] - 1))
	{
		_wsetlocale(LC_CTYPE, CodePageStr);
	}

	SetRegistryKey(_T("AleGr SoftWare"));

	//LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	Profile.AddItem(_T("Settings\\Color"), _T("NormalText"), m_TextColor.Normal.Text, m_TextColor.Normal.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("NormalBackground"), m_TextColor.Normal.BG, m_TextColor.Normal.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("SelectedText"), m_TextColor.Selected.Text, m_TextColor.Selected.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("SelectedBackground"), m_TextColor.Selected.BG, m_TextColor.Selected.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("ErasedLineText"), m_ErasedLineColor.Normal.Text, m_ErasedLineColor.Normal.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("ErasedLineBackground"), m_ErasedLineColor.Normal.BG, m_ErasedLineColor.Normal.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("ErasedText"), m_ErasedColor.Normal.Text, m_ErasedColor.Normal.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("ErasedBackground"), m_ErasedColor.Normal.BG, m_ErasedColor.Normal.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("ErasedSelLineText"), m_ErasedLineColor.Selected.Text, m_ErasedLineColor.Selected.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("ErasedSelLineBackground"), m_ErasedLineColor.Selected.BG, m_ErasedLineColor.Selected.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("ErasedSelText"), m_ErasedColor.Selected.Text, m_ErasedColor.Selected.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("ErasedSelBackground"), m_ErasedColor.Selected.BG, m_ErasedColor.Selected.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AddedLineText"), m_AddedLineColor.Normal.Text, m_AddedLineColor.Normal.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AddedLineBackground"), m_AddedLineColor.Normal.BG, m_AddedLineColor.Normal.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AddedText"), m_AddedColor.Normal.Text, m_AddedColor.Normal.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AddedBackground"), m_AddedColor.Normal.BG, m_AddedColor.Normal.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AddedSelLineText"), m_AddedLineColor.Selected.Text, m_AddedLineColor.Selected.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AddedSelLineBackground"), m_AddedLineColor.Selected.BG, m_AddedLineColor.Selected.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AddedSelText"), m_AddedColor.Selected.Text, m_AddedColor.Selected.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AddedSelBackground"), m_AddedColor.Selected.BG, m_AddedColor.Selected.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AcceptedText"), m_AcceptedColor.Normal.Text, m_AcceptedColor.Normal.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AcceptedBackground"), m_AcceptedColor.Normal.BG, m_AcceptedColor.Normal.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AcceptedText"), m_AcceptedColor.Selected.Text, m_AcceptedColor.Selected.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("AcceptedBackground"), m_AcceptedColor.Selected.BG, m_AcceptedColor.Selected.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("DiscardedText"), m_DiscardedColor.Normal.Text, m_DiscardedColor.Normal.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("DiscardedBackground"), m_DiscardedColor.Normal.BG, m_DiscardedColor.Normal.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("DiscardedText"), m_DiscardedColor.Selected.Text, m_DiscardedColor.Selected.Text, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("DiscardedBackground"), m_DiscardedColor.Selected.BG, m_DiscardedColor.Selected.BG, RGB(0, 0, 0), RGB(255, 255, 255));
	Profile.AddItem(_T("Settings\\Color"), _T("LineNumbers"), m_LineNumberTextColor, m_LineNumberTextColor, RGB(0, 0, 0), RGB(255, 255, 255));

	Profile.AddItem(_T("Settings"), _T("NormalFont"), m_NormalLogFont, m_NormalLogFont);
	Profile.AddItem(_T("Settings"), _T("AddedFont"), m_AddedLogFont, m_AddedLogFont);
	Profile.AddItem(_T("Settings"), _T("ErasedFont"), m_ErasedLogFont, m_ErasedLogFont);

	Profile.AddItem(_T("Settings"), _T("FontPointSize"), m_FontPointSize, 100, 30, 500);
	Profile.AddItem(_T("Settings"), _T("TabIndent"), m_TabIndent, 4, 1, 32);

	Profile.AddItem(_T("Settings"), _T("InitialDir1"), m_FileDir1, _T(""));
	Profile.AddItem(_T("Settings"), _T("InitialDir2"), m_FileDir2, _T(""));
	Profile.AddItem(_T("Settings"), _T("LastSaveMergedDir"), m_LastSaveMergedDir, _T("."));
	Profile.AddItem(_T("Settings"), _T("CopyFilesDir"), m_CopyFilesDir, _T("."));
	Profile.AddItem(_T("Settings"), _T("ShowFilesMask"), m_ShowFilesMask, 0xFFFFFFFF);

	//Profile.AddItem(_T("Settings"), _T("FilenameFilter"), m_sFilenameFilter, _T(""));
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
#else
	Profile.AddItem(_T("Settings"), _T("PreferencesFlags"), m_PreferencesFlags, m_PreferencesFlags, 0, 0xFFFFFFFF);
	Profile.AddItem(_T("Settings"), _T("StatusFlags"), m_StatusFlags, m_StatusFlags, 0, 0xFFFFFFFF);
#endif
	Profile.AddItem(_T("Settings"), _T("ComparisionMode"), m_ComparisonMode, FileComparisonModeDefault, FileComparisonModeMin, FileComparisonModeMax);
	Profile.AddItem(_T("Settings"), _T("NumberOfPanes"), m_NumberOfPanes, 1, 1, 2);


	Profile.AddItem(_T("Settings"), _T("MinimalLineLength"), m_MinimalLineLength, 2, 1, 2048);
	Profile.AddItem(_T("Settings"), _T("NumberOfIdenticalLines"), m_NumberOfIdenticalLines, 5, 1, 50);
	Profile.AddItem(_T("Settings"), _T("PercentsOfLookLikeDifference"), m_PercentsOfLookLikeDifference, 30, 0, 99);
	Profile.AddItem(_T("Settings"), _T("MinMatchingChars"), m_MinMatchingChars, 3, 1, 32);
	Profile.AddItem(_T("Settings"), _T("MinPercentWeakIdenticalLines"), m_MinPercentWeakIdenticalLines, 10, 0, 99);
	Profile.AddItem(_T("Settings"), _T("GoToLineFileSelection"), m_GoToLineFileSelection, 0, 0, 2);
	Profile.AddItem(_T("Settings"), _T("SearchScope"), m_SearchScope, 0, 0, 2);

	Profile.AddItem(_T("Settings"), _T("BinaryFiles"), m_sBinaryFilesFilter,
					_T("*.exe;*.dll;*.sys;*.obj;*.pdb;*.zip"));
	Profile.AddItem(_T("Settings"), _T("CppFiles"), m_sCppFilesFilter,
					_T("*.c;*.cpp;*.h;*.hpp;*.inl;*.rc;*.h++"));
	Profile.AddItem(_T("Settings"), _T("IgnoreFiles"), m_sIgnoreFilesFilter,
					_T("*.ncb"));
	//Profile.AddItem(_T("Settings"), _T("IgnoreFolders"), m_sIgnoreFoldersFilter, _T(""));

	static UCHAR DefaultColumnArray[MaxColumns] =
	{
		ColumnName,
		ColumnSubdir,
		ColumnDate1,
		ColumnDate2,
		ColumnLength1,
		ColumnLength2,
		ColumnComparisionResult,
	};

	static SHORT DefaultColumnWidthArray[MaxColumns] =
	{
		200, 200,
		150, 150,
		150, 150,
		400,
	};

	Profile.AddItem(_T("Settings"), _T("Columns"), m_ColumnArray,
					DefaultColumnArray);
	Profile.AddItem(_T("Settings"), _T("ColumnsWidth"), m_ColumnWidthArray,
					DefaultColumnWidthArray);

	static UCHAR DefaultSortOrder[MaxColumns] =
	{
		ColumnSubdir | 0x80,
		ColumnName | 0x80,
		ColumnDate1,
		ColumnDate2,
		ColumnLength1 | 0x80,
		ColumnLength2 | 0x80,
		ColumnComparisionResult,
	};

	Profile.AddItem(_T("Settings"), _T("SortOrder"), m_ColumnSort,
					DefaultSortOrder);

	//Profile.RemoveFromRegistry(_T("Settings"), _T("FileListSort"));

	m_RecentFiles.Load();
	m_FileFilters.Load();
	m_FindHistory.Load();
	m_RecentFolders.Load();

	//m_sFilenameFilter = m_FileFilters[0];

	// set the default folder directory to My Documents
	TCHAR MyDocuments[MAX_PATH] = { 0};
	char MyDocumentsA[MAX_PATH] = { 0};

	if (SHGetSpecialFolderPath(NULL, MyDocuments, CSIDL_PERSONAL, FALSE))
	{
		SetCurrentDirectory(MyDocuments);
	}
	else
	{
		if (SHGetSpecialFolderPathA(NULL, MyDocumentsA, CSIDL_PERSONAL, FALSE))
		{
			SetCurrentDirectoryA(MyDocumentsA);
		}
	}

	m_TextColor.Normal.BG = GetSysColor(COLOR_WINDOW);
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
												RUNTIME_CLASS(CTextFilePairDoc),
												RUNTIME_CLASS(CChildFrame), // custom MDI child frame
												RUNTIME_CLASS(CDiffFileView));
	AddDocTemplate(m_pFileDiffTemplate);

	m_pBinaryDiffTemplate = new CMultiDocTemplate(
												IDR_ALEGRDTYPE_BIN,
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

	pMainFrame->InitialShowWindow(SW_SHOWDEFAULT);

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
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnButtonMailto();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
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
	if (!pPair->HasContents())
	{
		// the file is not a real file, can't open
		return NULL;
	}

	POSITION DocTemplatePos = GetFirstDocTemplatePosition();
	CDocTemplate* DocTemplate = nullptr;

	while (DocTemplatePos)
	{
		DocTemplate = GetNextDocTemplate(DocTemplatePos);
		// check if there is already a CTextFilePairDoc
		POSITION position = DocTemplate->GetFirstDocPosition();
		while (position)
		{
			CDocument * pDocument = DocTemplate->GetNextDoc(position);
			CFilePairDoc * pDoc =
				dynamic_cast<CFilePairDoc *>(pDocument);
			if (nullptr != pDoc
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

	if (pPair->NeedBinaryComparison())
	{
		DocTemplate = m_pBinaryDiffTemplate;
	}
	else
	{
		DocTemplate = m_pFileDiffTemplate;
	}

	CFilePairDoc* pDoc = dynamic_cast<CFilePairDoc*>(DocTemplate->OpenDocumentFile(NULL));
	if (nullptr != pDoc)
	{
		pDoc->SetFilePair(pPair);
	}
	return pDoc;
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
	dlg.m_ViewPage.m_NormalTextColor = m_TextColor.Normal.Text;
	dlg.m_ViewPage.m_AddedLogFont = m_AddedLogFont;
	dlg.m_ViewPage.m_AddedTextColor = m_AddedColor.Normal.Text;
	dlg.m_ViewPage.m_ErasedLogFont = m_ErasedLogFont;
	dlg.m_ViewPage.m_ErasedTextColor = m_ErasedColor.Normal.Text;
	dlg.m_ViewPage.m_FontPointSize = m_FontPointSize;
	dlg.m_ViewPage.m_NormalTextBackground = m_TextColor.Normal.BG;
	dlg.m_ViewPage.m_AddedTextBackground = m_AcceptedColor.Normal.BG;
	dlg.m_ViewPage.m_ErasedTextBackground = m_DiscardedColor.Normal.BG;
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
			m_TextColor.Normal.Text = dlg.m_ViewPage.m_NormalTextColor;

			m_AddedLogFont = dlg.m_ViewPage.m_AddedLogFont;
			m_AddedColor.Normal.Text = dlg.m_ViewPage.m_AddedTextColor;

			m_ErasedLogFont = dlg.m_ViewPage.m_ErasedLogFont;
			m_ErasedColor.Normal.Text = dlg.m_ViewPage.m_ErasedTextColor;

			m_FontPointSize = dlg.m_ViewPage.m_FontPointSize;
			OnFontChanged();
		}
		else if (dlg.m_ViewPage.m_bColorChanged)
		{
			m_TextColor.Normal.BG = dlg.m_ViewPage.m_NormalTextBackground;
			m_AcceptedColor.Normal.BG = dlg.m_ViewPage.m_AddedTextBackground;
			m_DiscardedColor.Normal.BG = dlg.m_ViewPage.m_ErasedTextBackground;

			UpdateAllViews(UpdateViewsColorsChanged);
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

	UpdateAllViews(UpdateViewsMetricsChanged);
}

void CAlegrDiffApp::NotifyFilePairReplaced(FilePair *pPair, FilePair* pNewPair)
{
	FilePairChangedArg arg(pPair, pNewPair);
	UpdateAllViews(UpdateViewsFilePairChanged, &arg);
	if (!pPair->IsAlone())
	{
		pNewPair->Reference();
		pPair->InsertAsNextItem(pNewPair);
		pPair->RemoveFromList();
		pPair->Dereference();
	}
}

void CAlegrDiffApp::NotifyFilePairChanged(FilePair *pPair)
{
	FilePairChangedArg arg(pPair);
	UpdateAllViews(UpdateViewsFilePairChanged, & arg);
}

void CAlegrDiffApp::UpdateAllViews(LPARAM lHint, CObject* pHint)
{
	POSITION DocTempPos = m_pDocManager->GetFirstDocTemplatePosition();
	while (DocTempPos)
	{
		CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(DocTempPos);
		POSITION position = pTemplate->GetFirstDocPosition();
		while(position)
		{
			CDocument * pDoc = pTemplate->GetNextDoc(position);
			CAlegrDiffBaseDoc * pBaseDoc = dynamic_cast<CAlegrDiffBaseDoc *>(pDoc);
			if (NULL != pBaseDoc)
			{
				pBaseDoc->OnUpdateAllViews(NULL, lHint, pHint);
			}
			else
			{
				pDoc->UpdateAllViews(NULL, lHint, pHint);
			}
		}
	}
}

UINT AFXAPI AfxGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
// from _AfxAbbreviateName
void AFXAPI AbbreviateName(LPTSTR lpszCanon, int cchMax, BOOL bAtLeastName)
{
	int cchFullPath, cchFileName;
	ptrdiff_t cchVolName;
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
	if (!pFile->HasContents())
	{
		CString s;
		s.LoadString(DisabledItemID);
		pCmdUI->SetText(s);
		pCmdUI->Enable(FALSE);
		return;
	}
	CString name(pFile->GetFullName());
	AbbreviateName(name.GetBuffer(MAX_PATH * 2), 50, true);
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
	// try different verbs. (open first)
	//shex.lpVerb = _T("Open");
	shex.lpFile = name;
	shex.nShow = SW_SHOWDEFAULT;
	ShellExecuteEx( & shex);
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
	dlg.m_bDoNotCompareFileContents = m_bDoNotCompareFileContents;

	if (NULL != filter)
	{
		dlg.m_FilenameFilter = filter;
	}

	if (NULL != dir1)
	{
		dlg.m_sFirstDir = dir1;
	}

	if (NULL != dir2)
	{
		dlg.m_sSecondDir = dir2;
	}

	// if both directories are specified and Shift is not held, then skip the dialog
	if ((NULL != dir1 && 0 != dir1[0] && NULL != dir2 && 0 != dir2[0] && 0 == (0x8000 & GetKeyState(VK_SHIFT)))
		|| IDOK == dlg.DoModal())
	{
		m_bRecurseSubdirs = (1 == dlg.m_bIncludeSubdirs);

		m_sBinaryFilesFilter = dlg.m_sBinaryFilesFilter;

		m_sCppFilesFilter = dlg.m_sCppFilesFilter;

		m_sIgnoreFilesFilter = dlg.m_sIgnoreFilesFilter;

		m_TabIndent = dlg.m_nTabIndent;

		m_BinaryComparision = (0 != dlg.m_BinaryComparision);

		m_bAdvancedCompareDialog = dlg.m_bAdvanced;

		m_bUseMd5 = dlg.m_bUseMd5 != 0;
		m_bDoNotCompareFileContents = dlg.m_bDoNotCompareFileContents != 0;

		CAlegrDiffDoc * pDoc = (CAlegrDiffDoc *)
								m_pListDiffTemplate->OpenDocumentFile(NULL);
		if (NULL == pDoc)
		{
			return;
		}
		pDoc->SetTitle(_T(""));

		if ( ! pDoc->RunDirectoriesComparison(dlg.m_sFirstDir, dlg.m_sSecondDir,
											dlg.m_FilenameFilter, dlg.m_sIgnoreFoldersFilter,
											m_bRecurseSubdirs, m_BinaryComparision, m_bDoNotCompareFileContents))
		{
			pDoc->OnCloseDocument();
		}
	}
}

void CAlegrDiffApp::CompareFiles(LPCTSTR OPTIONAL pName1, LPCTSTR OPTIONAL pName2)
{
	CFilesCompareDialog dlg;
	if (pName1 != NULL && 0 != pName1[0])
	{
		dlg.m_sFirstFileName = pName1;
	}

	if (pName2 != NULL && 0 != pName2[0])
	{
		dlg.m_sSecondFileName = pName2;
	}

	if (dlg.m_sFirstFileName.IsEmpty()
		|| dlg.m_sSecondFileName.IsEmpty()
		|| 0 != (0x8000 & GetKeyState(VK_SHIFT)))
	{
		dlg.m_ComparisonMode = m_ComparisonMode;

		if (IDOK != dlg.DoModal())
		{
			return;
		}

		m_ComparisonMode = dlg.m_ComparisonMode;
	}
	OpenPairOrFile(dlg.m_sFirstFileName, dlg.m_sSecondFileName);
}

void CAlegrDiffApp::OpenPairOrFile(LPCTSTR Name1, LPCTSTR OPTIONAL Name2)
{
	// TODO: check if there is already a CTextFilePairDoc
	CPathEx FileDir1;
	CString FileName1;
	WIN32_FIND_DATA wfd1 = { 0 };
	FileItem* FileItem1 = nullptr;

	CPathEx FileDir2;
	CString FileName2;
	WIN32_FIND_DATA wfd2 = { 0 };
	FileItem* FileItem2 = nullptr;

	CString sCFilesPattern(PatternToMultiCString(m_sCppFilesFilter));
	CString sBinFilesPattern(PatternToMultiCString(m_sBinaryFilesFilter));

	int ComparisonMode = m_ComparisonMode;

	if (Name1 == nullptr || Name1[0] == 0)
	{
		// The first name must be specified
		return;
	}

	FileDir1.MakeFullPath(Name1);

	HANDLE hFind = FindFirstFile(FileDir1, &wfd1);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		CString s;
		s.Format(IDS_STRING_CANT_OPEN_FILE, Name1);
		AfxMessageBox(s);
		return;
	}
	FindClose(hFind);

	m_RecentFiles.AddString(FileDir1);

	FileDir1.RemoveFileSpec();
	FileDir1.AddBackslash();

	FileItem1 = new FileItem(&wfd1, FileDir1, _T(""), NULL);

	if (FileComparisonModeDefault == ComparisonMode
		&& MultiPatternMatches(wfd1.cFileName, sBinFilesPattern))
	{
		ComparisonMode = FileComparisonModeBinary;
	}

	if (Name2 != nullptr && Name2[0] != 0)
	{
		FileDir2.MakeFullPath(Name2);

		hFind = FindFirstFile(FileDir2, &wfd2);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			CString s;
			s.Format(IDS_STRING_CANT_OPEN_FILE, LPCTSTR(Name2));
			AfxMessageBox(s);

			delete FileItem1;
			return;
		}
		FindClose(hFind);

		m_RecentFiles.AddString(FileDir2, 1);
		FileDir2.RemoveFileSpec();
		FileDir2.AddBackslash();

		FileItem2 = new FileItem(&wfd2, FileDir2, _T(""), NULL);

		if (FileComparisonModeDefault == ComparisonMode
			&& MultiPatternMatches(wfd2.cFileName, sBinFilesPattern))
		{
			ComparisonMode = FileComparisonModeBinary;
		}
	}

	FilePair* pPair = nullptr;
	CDocTemplate* DocTemplate = nullptr;

	if (FileComparisonModeBinary == m_ComparisonMode)
	{
		pPair = new BinaryFilePair(FileItem1, FileItem2);
		FileItem1->SetBinary();
		if (nullptr != FileItem2) FileItem2->SetBinary();

		DocTemplate = m_pBinaryDiffTemplate;
	}
	else
	{
		pPair = new TextFilePair(FileItem1, FileItem2);

		if (MultiPatternMatches(wfd1.cFileName, sCFilesPattern))
		{
			FileItem1->SetCCpp();
		}
		else
		{
			FileItem1->SetText();
		}

		if (FileItem2 == nullptr)
		{
			// skip
		}
		else if (MultiPatternMatches(wfd2.cFileName, sCFilesPattern))
		{
			FileItem2->SetCCpp();
		}
		else
		{
			FileItem2->SetText();
		}
		DocTemplate = m_pFileDiffTemplate;
	}

	CFilePairDoc* pDoc = dynamic_cast<CFilePairDoc*>(DocTemplate->OpenDocumentFile(NULL));
	if (NULL != pDoc)
	{
		pDoc->SetFilePair(pPair);
	}
	// SetFilePair references the pair
	pPair->Dereference();
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
	if (INVALID_FILE_ATTRIBUTES == FileAttr1)
	{
		// failed
		return;
	}
	if (FileAttr1 & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (Arg2 != NULL && Arg2[0] != 0)
		{
			DWORD FileAttr2 = GetFileAttributes(Arg2);
			if (INVALID_FILE_ATTRIBUTES == FileAttr2)
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

	CString TargetDir = pApp->m_CopyFilesDir;
	CString DlgTitle;
	DlgTitle.LoadString(IDS_COPY_FILES_TITLE);

	CFolderDialog dlg(DlgTitle, TargetDir, true,
					BIF_RETURNFSANCESTORS |
					BIF_RETURNONLYFSDIRS |
					BIF_NEWDIALOGSTYLE, NULL,
					& GetApp()->m_RecentFolders);

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

	int SrcBufLen = 1;
	int DstBufLen = 1;
	for (int i = 0; i < nCount; i++)
	{
		FileItem * pFile = ppFiles[i];
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

	for (int i = 0; i < nCount; i++)
	{
		FileItem * pFile = ppFiles[i];
		_tcsncpy_s(pSrcBuf + SrcBufIdx, SrcBufLen - SrcBufIdx, pFile->GetBasedir(), pFile->GetBasedirLength());
		SrcBufIdx += pFile->GetBasedirLength();

		_tcsncpy_s(pSrcBuf + SrcBufIdx, SrcBufLen - SrcBufIdx, pFile->GetSubdir(), pFile->GetSubdirLength());
		SrcBufIdx += pFile->GetSubdirLength();

		_tcsncpy_s(pSrcBuf + SrcBufIdx, SrcBufLen - SrcBufIdx, pFile->GetName(), pFile->GetNameLength());
		SrcBufIdx += pFile->GetNameLength();

		pSrcBuf[SrcBufIdx] = 0;
		SrcBufIdx++;

		_tcscpy_s(pDstBuf + DstBufIdx, DstBufLen - DstBufIdx, TargetDir);
		DstBufIdx += TargetDir.GetLength();

		if (bAddSubdirToTarget)
		{
			_tcsncpy_s(pDstBuf + DstBufIdx, DstBufLen - DstBufIdx, pFile->GetSubdir(), pFile->GetSubdirLength());
			DstBufIdx += pFile->GetSubdirLength();
		}

		_tcsncpy_s(pDstBuf + DstBufIdx, DstBufLen - DstBufIdx, pFile->GetName(), pFile->GetNameLength());
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
	if ( ! SystemTimeToTzSpecificLocalTime(NULL, & SystemTime, & LocalTime))
	{
		FILETIME LocalFileTime;
		FileTimeToLocalFileTime( & FileTime, & LocalFileTime);
		FileTimeToSystemTime( & LocalFileTime, & LocalTime);
	}

	GetDateFormat(locale, DATE_SHORTDATE, & LocalTime, NULL, str, TimeBufSize - 1);
	CString result = str;
	result += ' ';

	GetTimeFormat(locale, TIME_NOSECONDS, & LocalTime, NULL, str, TimeBufSize - 1);
	result += str;
	return result;
}

void FileTimeToStr(FILETIME FileTime, TCHAR str[256], LCID locale)
{
	int const TimeBufSize = 256;
	SYSTEMTIME SystemTime;
	SYSTEMTIME LocalTime;
	memzero(LocalTime);

	FileTimeToSystemTime(&FileTime, &SystemTime);
	if (!SystemTimeToTzSpecificLocalTime(NULL, &SystemTime, &LocalTime))
	{
		FILETIME LocalFileTime;
		FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
		FileTimeToSystemTime(&LocalFileTime, &LocalTime);
	}

	int len = GetDateFormat(locale, DATE_SHORTDATE, &LocalTime, NULL, str, TimeBufSize - 1);
	if (len)
	{
		str[len-1] = ' ';

		GetTimeFormat(locale, TIME_NOSECONDS, &LocalTime, NULL, str + len, TimeBufSize - (len + 1));
	}
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

void CAlegrDiffApp::OnHelpUsing()
{
	TCHAR ModuleName[MAX_PATH] = {0};
	TCHAR FullPathName[MAX_PATH];
	LPTSTR FilePart = FullPathName;
	GetModuleFileName(NULL, ModuleName, MAX_PATH);
	GetFullPathName(ModuleName, MAX_PATH, FullPathName, & FilePart);
	if (NULL != FilePart)
	{
		*FilePart = 0;
	}
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

int BrowseForFile(int TitleID, CString & Name, CString & BrowseFolder,
				CStringHistory * pHistory)
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
		&& GetFullPathName(Name, MAX_PATH, FullPath, & FileNamePart)
		&& NULL != FileNamePart)
	{
		LastFileName = FileNamePart;
		// find extension
		LPCTSTR pExt = _tcsrchr(FileNamePart, '.');
		if (pExt)
		{
			CurrentCustomFilterString = CreateCustomFilter(pExt);
			if (Filters.empty()
				|| 0 != Filters[0].CompareNoCase(CurrentCustomFilterString))
			{
				Filters.push_back(CurrentCustomFilterString);
			}
		}
		*FileNamePart = 0;
	}

	if (0 != pHistory)
	{
		for (int i = 0; i < pHistory->Size(); i++)
		{
			TCHAR HistoryFullPath[MAX_PATH];
			LPTSTR HistoryFileNamePart;
			if ( ! (*pHistory)[i].IsEmpty()
				&& GetFullPathName((*pHistory)[i], MAX_PATH, HistoryFullPath, &HistoryFileNamePart)
				&& NULL != HistoryFileNamePart)
			{
				// find extension
				LPCTSTR pExt = _tcsrchr(HistoryFileNamePart, '.');
				if (pExt)
				{
					CurrentCustomFilterString = CreateCustomFilter(pExt);
					unsigned j;
					for (j = 0; j < Filters.size(); j++)
					{
						if (0 == Filters[j].CompareNoCase(CurrentCustomFilterString))
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

	Filter += AllFilesFilter;

	CFileDialogWithHistory dlg(TRUE, & pApp->m_RecentFolders, NULL, LastFileName,
								OFN_HIDEREADONLY | OFN_FILEMUSTEXIST
								| OFN_NOCHANGEDIR | OFN_EXPLORER,
								Filter);

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
	dlg.m_ofn.nMaxCustFilter = countof(OfnCustomFilter);

	if (IDOK != dlg.DoModal())
	{
		return IDCANCEL;
	}

	LPTSTR pFilter = OfnCustomFilter;
	// skip first string (it's usually of sero length
	while (*(pFilter++) != 0) {}
	if (0 != _tcscmp(pFilter, _T("*"))
		&& 0 != _tcscmp(pFilter, _T("*.*")))
	{
		size_t len = _tcslen(pFilter);
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

	BrowseFolder = dlg.GetLastFolder();
	return IDOK;
}

void CAlegrDiffApp::OnFileCreatedirectoryfingerprint()
{
	CDirectoryFingerprintDlg dlg(m_sIgnoreFilesFilter, m_bRecurseSubdirs);

	if (IDOK != dlg.DoModal())
	{
		return;
	}

	m_bRecurseSubdirs = dlg.DoIncludeSubdirectories();

	m_sIgnoreFilesFilter = dlg.GetIgnoreFiles();

	CDirectoryFingerpringCreateDlg
	(dlg.GetDirectory(),
		dlg.GetFingerprintName(), dlg.GetFilenameFilter(),
		dlg.GetIgnoreFiles(), dlg.GetIgnoreFolders(),
		dlg.DoIncludeSubdirectories(), dlg.DoIncludeDirectoryStructure()).DoModal();
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

	pDoc->SetFingerprintCheckingMode(dlg.GetDirectory(),
									dlg.GetFingerprintFilename());

	CDirectoryFingerprintCheckDlg dlg1(pDoc,
										dlg.GetDirectory(), dlg.GetFingerprintFilename());

	if (IDOK != dlg1.DoModal())
	{
		pDoc->OnCloseDocument();
		return;
	}
	pDoc->UpdateAllViews(NULL, OnUpdateRebuildListView);
}

CString UlonglongToStr(ULONGLONG Num, LCID locale)
{
	int const NumBufSize = 50;
	TCHAR str1[NumBufSize] = {0};
	TCHAR str2[NumBufSize] = {0};

	_stprintf_s(str1, countof(str1), _T("%I64u"), Num);

	GetNumberFormat(locale, 0, str1, NULL, str2, NumBufSize);

	GetLocaleInfo(locale, LOCALE_SDECIMAL, str1, NumBufSize);
	TCHAR * pDot = _tcsstr(str2, str1);
	if (pDot)
	{
		* pDot = 0;
	}
	return str2;
}

CString FileLengthToStrKb(ULONGLONG Length)
{
	TCHAR buf[32]={0};
	StrFormatByteSize(Length, buf, countof(buf));
	return CString(buf);
}

void FileLengthToStrKb(ULONGLONG Length, TCHAR buf[64])
{
	StrFormatByteSize(Length, buf, 64);
}

void CAlegrDiffApp::OnWindowCloseall()
{
	CloseAllDocuments(FALSE);
}
