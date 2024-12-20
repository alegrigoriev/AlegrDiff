// AlegrDiff.h : main header file for the ALEGRDIFF application
//

#if !defined(AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_)
#define AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WM_SETMESSAGESTRING_POST (WM_APP + 0xAA)

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "ApplicationProfile.h"
#include "FileListSupport.h"
/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffApp:
// See AlegrDiff.cpp for the implementation of this class
//
enum eColumns
{
	ColumnName,
	ColumnSubdir,
	ColumnDate1,
	ColumnDate2,
	ColumnLength1,
	ColumnLength2,
	ColumnComparisionResult,
	MaxColumns,
};

enum {
	UpdateViewsColorsChanged = 0x100000,
	UpdateViewsFilePairChanged = 0x200000,
	UpdateViewsMetricsChanged = 0x300000,
	UpdateViewsFilePairDeleteFromList = 0x400000,
	UpdateViewsFilePairDeleteView = 0x400000,
	UpdateViewsCloseOpenFiles = 0x500000,
	UpdateViewsReplaceFilePair = 0x600000,
};

class FilePairChangedArg : public CObject
{
public:
	class FilePair * m_pPair;
	class FilePair * m_pNewPair;
	FilePairChangedArg(FilePair * pPair, FilePair* pNewPair = nullptr)
		: m_pPair(pPair),
		m_pNewPair(pNewPair)
	{
	}
};

class CAlegrDiffApp : public CWinApp
{
public:

	CAlegrDiffApp();
	~CAlegrDiffApp();

	void OpenPairOfPathnames(LPTSTR path1, LPTSTR path2);
	void CompareFiles(LPCTSTR OPTIONAL pName1, LPCTSTR OPTIONAL pName2);
	void OpenPairOrFile(LPCTSTR pName1, LPCTSTR OPTIONAL pName2 = NULL);
	void CompareDirectories(LPCTSTR dir1, LPCTSTR dir2, LPCTSTR filter = NULL);
	void ParseCommandLine();
	void NotifyFilePairReplaced(FilePair * pPair, FilePair* pNewPair);
	void NotifyFilePairChanged(FilePair * pPair);

	CApplicationProfile Profile;

	struct COLOR_PAIR
	{
		COLORREF Text;
		COLORREF BG;
	};
	struct NORMAL_OR_SELECTED_COLOR
	{
		COLOR_PAIR Normal;
		COLOR_PAIR Selected;
	};

	NORMAL_OR_SELECTED_COLOR m_TextColor =
	{
		{ RGB(0, 0, 0), RGB(255, 255, 255) },
		{ RGB(0, 0, 0), RGB(153, 201, 239) },
	};
	NORMAL_OR_SELECTED_COLOR m_ErasedLineColor =
	{
		{ RGB(0, 0, 0), RGB(235, 153, 153) },
		{ RGB(0, 0, 0), RGB(153, 140, 208) },
	};
	NORMAL_OR_SELECTED_COLOR m_ErasedColor =
	{
		{ RGB(0, 0, 0), RGB(255, 204, 204) },
		{ RGB(0, 0, 0), RGB(153, 170, 208) },
	};
	NORMAL_OR_SELECTED_COLOR m_AddedLineColor =
	{
		{ RGB(0, 0, 0), RGB(215, 227, 188) },
		{ RGB(0, 0, 0), RGB(129, 184, 199) },
	};
	NORMAL_OR_SELECTED_COLOR m_AddedColor =
	{
		{ RGB(0, 0, 0), RGB(235, 241, 221) },
		{ RGB(0, 0, 0), RGB(141, 193, 219) },
	};
	NORMAL_OR_SELECTED_COLOR m_AcceptedColor =
	{
		{ RGB(0, 0, 0), RGB(240, 240, 0) },
		{ RGB(0, 0, 0), RGB(141, 141, 0) },
	};
	NORMAL_OR_SELECTED_COLOR m_DiscardedColor =
	{
		{ RGB(0, 0, 0), RGB(192, 192, 192) },
		{ RGB(0, 0, 0), RGB(128, 128, 128) },
	};

	COLORREF   m_LineNumberTextColor = RGB(43, 145, 175);

	union
	{
		DWORD m_PreferencesFlags;
		struct
		{
			bool m_bRecurseSubdirs : 1;
			bool m_bAdvancedCompareDialog : 1;
			bool m_BinaryComparision : 1;
			bool m_AutoReloadChangedFiles : 1;
			bool m_bIgnoreWhitespaces : 1;
			bool m_bShowLineNumbers : 1;
			bool m_bFindBackward : 1;
			bool m_bCaseSensitive : 1;
			bool m_bCancelSelectionOnMerge : 1;
			bool m_bUseMd5 : 1;
			bool m_bFindWholeWord : 1;
			bool m_bDoNotCompareFileContents : 1;
		};
	};
	union
	{
		DWORD m_StatusFlags;
		struct
		{
			bool m_bShowToolbar:1;
			bool m_bShowStatusBar:1;
			bool :1;
			bool :1;
		};
	};

	CString m_FileDir1;
	CString m_FileDir2;
	CString m_LastSaveMergedDir;
	CString m_CopyFilesDir;
	CString m_CustomFileOpenFilter;

	CStringHistory m_FindHistory;
	CStringHistory m_RecentFolders;
	CStringHistory m_FileFilters;
	CStringHistory m_RecentFiles;

	UCHAR m_ColumnArray[MaxColumns];
	UCHAR m_ColumnSort[MaxColumns];
	SHORT m_ColumnWidthArray[MaxColumns];

	CString m_sBinaryFilesFilter;
	CString m_sCppFilesFilter;
	CString m_sIgnoreFilesFilter;

	CString m_FindString;

	DWORD	m_MinimalLineLength;
	DWORD	m_NumberOfIdenticalLines;
	DWORD	m_PercentsOfLookLikeDifference;
	DWORD	m_MinMatchingChars;
	DWORD m_ShowFilesMask;
	int m_MaxSearchDistance;
	int m_MinIdenticalLines;
	int m_MinPercentWeakIdenticalLines; // default 10
	int m_TabIndent;
	int m_GoToLineFileSelection;
	int m_SearchScope;
	enum FileComparisonMode
	{
		FileComparisonModeDefault = 0,
		FileComparisonModeText = 1,
		FileComparisonModeBinary = 2,

		FileComparisonModeMin = FileComparisonModeDefault,
		FileComparisonModeMax = FileComparisonModeBinary,
	};
	int m_ComparisonMode;
	int m_NumberOfPanes;

	LOGFONT m_NormalLogFont;
	CFont m_NormalFont;
	LOGFONT m_AddedLogFont;
	CFont m_AddedFont;
	LOGFONT m_ErasedLogFont;
	CFont m_ErasedFont;
	int m_FontPointSize;
	void OnFontChanged();
	void UpdateAllViews(LPARAM lHint = 0L, CObject* pHint = NULL);

	CDocument * OpenFilePairView(FilePair * pPair);
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlegrDiffApp)
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL
// Implementation
	CMultiDocTemplate * m_pFileDiffTemplate;
	CMultiDocTemplate * m_pBinaryDiffTemplate;
	CMultiDocTemplate * m_pListDiffTemplate;

	//{{AFX_MSG(CAlegrDiffApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileComparedirectories();
	afx_msg void OnFileComparefiles();
	afx_msg void OnFilePreferences();
	afx_msg void OnUpdateViewIgnoreWhitespaces(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewShowLineNumbers(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditAccept(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDecline(CCmdUI* pCmdUI);
	afx_msg void OnHelpUsing();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileCreatedirectoryfingerprint();
	afx_msg void OnFileCheckDirectoryFingerprint();
	afx_msg void OnWindowCloseall();
};

typedef CAlegrDiffApp CThisApp;
inline CThisApp * GetApp() noexcept
{
	return (CThisApp *) AfxGetApp();
}

enum {
	SetPositionMakeVisible = 1,
	SetPositionMakeCentered = 2,
	SetPositionCancelSelection = 4,
	MoveCaretPositionAlways = 8,
	SetWordSelectionMode = 0x10,
	OnUpdateListViewItem = 0x100,
};

class InvalidatedRange : public CObject
{
public:
	TextPosLine begin;  // line position
	TextPosLine end;
};

void ModifyOpenFileMenu(CCmdUI* pCmdUI, class FileItem * pFile, UINT FormatID, UINT DisabledItemID);
void OpenFileForEditing(class FileItem * pFile);
void CopyFilesToFolder(FileItem ** ppFiles, int nCount, bool bAddSubdirToTarget);
CString FileTimeToStr(FILETIME FileTime, LCID locale = LOCALE_USER_DEFAULT);
inline CString FileTimeToStr(ULONGLONG const &FileTime, LCID locale = LOCALE_USER_DEFAULT)
{
	return FileTimeToStr(reinterpret_cast<FILETIME const&>(FileTime), locale);
}
void FileTimeToStr(FILETIME FileTime, TCHAR str[256], LCID locale);
inline void FileTimeToStr(ULONGLONG const& FileTime, TCHAR str[256], LCID locale = LOCALE_USER_DEFAULT)
{
	FileTimeToStr(reinterpret_cast<FILETIME const&>(FileTime), str, locale);
}
CString UlonglongToStr(ULONGLONG Length, LCID locale = LOCALE_USER_DEFAULT);
CString FileLengthToStrKb(ULONGLONG Length);
void FileLengthToStrKb(ULONGLONG Length, TCHAR buf[64]);
void AFXAPI AbbreviateName(LPTSTR lpszCanon, int cchMax, BOOL bAtLeastName);

CString CreateCustomFilter(LPCTSTR Extension);
int BrowseForFile(int TitleID, CString & Name, CString & BrowseFolder,
				CStringHistory * pHistory);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_)
