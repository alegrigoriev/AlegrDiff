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
};

class FilePairChangedArg : public CObject
{
public:
	class FilePair * pPair;
};

class CAlegrDiffApp : public CWinApp
{
	bool m_bIsWin9x;
public:

	void OpenPairOfPathnames(LPTSTR path1, LPTSTR path2);
	void CompareFiles(LPCTSTR name1, LPCTSTR name2);
	void CompareDirectories(LPCTSTR dir1, LPCTSTR dir2, LPCTSTR filter = NULL);
	void ParseCommandLine();
	void OpenSingleFile(LPCTSTR pName);
	void NotifyFilePairChanged(FilePair * pPair);
	CAlegrDiffApp();
	~CAlegrDiffApp();

	bool IsWindows9x() const { return m_bIsWin9x; }

	CApplicationProfile Profile;

	DWORD m_NormalTextColor;
	DWORD m_ErasedTextColor;
	DWORD m_AddedTextColor;
	DWORD m_TextBackgroundColor;
	DWORD m_AcceptedTextBackgroundColor;
	DWORD m_DiscardedTextBackgroundColor;
	DWORD m_SelectedTextColor;

	union
	{
		DWORD m_PreferencesFlags;
		struct
		{
			bool m_bRecurseSubdirs:1;
			bool m_bAdvancedCompareDialog:1;
			bool m_BinaryComparision:1;
			bool m_AutoReloadChangedFiles:1;
			bool m_bIgnoreWhitespaces:1;
			bool m_bShowLineNumbers:1;
			bool m_bFindBackward:1;
			bool m_bCaseSensitive:1;
			bool m_bCancelSelectionOnMerge:1;
			bool m_bUseMd5:1;
			bool m_bFindWholeWord:1;
		};
	};
	union
	{
		DWORD m_StatusFlags;
		struct
		{
			bool m_bShowToolbar:1;
			bool m_bShowStatusBar:1;
			bool m_bOpenMaximized:1;
			bool m_bOpenChildMaximized:1;
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

	CString m_sFilenameFilter;
	CString m_sBinaryFilesFilter;
	CString m_sCppFilesFilter;
	CString m_sIgnoreFilesFilter;
	CString m_sIgnoreFoldersFilter;

	CString m_FindString;

	DWORD	m_MinimalLineLength;
	DWORD	m_NumberOfIdenticalLines;
	DWORD	m_PercentsOfLookLikeDifference;
	DWORD	m_MinMatchingChars;
	int m_MaxSearchDistance;
	int m_MinIdenticalLines;
	int m_MinPercentWeakIdenticalLines; // default 10
	int m_TabIndent;
	int m_GoToLineFileSelection;
	int m_SearchScope;

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
	void CloseFilePairView(FilePair * pPair);
	void ReloadFilePairView(FilePair * pPair);
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
};

typedef CAlegrDiffApp CThisApp;
inline CThisApp * GetApp()
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
	TextPos begin;
	TextPos end;
};

void ModifyOpenFileMenu(CCmdUI* pCmdUI, class FileItem * pFile, UINT FormatID, UINT DisabledItemID);
void OpenFileForEditing(class FileItem * pFile);
void CopyFilesToFolder(FileItem ** ppFiles, int nCount, bool bAddSubdirToTarget);
CString FileTimeToStr(FILETIME FileTime, LCID locale = LOCALE_USER_DEFAULT);
CString UlonglongToStr(ULONGLONG Length, LCID locale = LOCALE_USER_DEFAULT);
CString FileLengthToStrKb(ULONGLONG Length);
void AFXAPI AbbreviateName(LPTSTR lpszCanon, int cchMax, BOOL bAtLeastName);

CString CreateCustomFilter(LPCTSTR Extension);
int BrowseForFile(int TitleID, CString & Name, CString & BrowseFolder,
				CStringHistory * pHistory);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_)
