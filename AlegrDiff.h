// AlegrDiff.h : main header file for the ALEGRDIFF application
//

#if !defined(AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_)
#define AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

class CAlegrDiffApp : public CWinApp
{
public:
	void OpenPairOfPathnames(LPTSTR path1, LPTSTR path2);
	void CompareFiles(LPCTSTR name1, LPCTSTR name2);
	void CompareDirectories(LPCTSTR dir1, LPCTSTR dir2, LPCTSTR filter = NULL);
	void ParseCommandLine();
	void OpenSingleFile(LPCTSTR pName);
	void NotifyFilePairChanged(FilePair * pPair);
	CAlegrDiffApp();
	~CAlegrDiffApp();

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

	CString m_sFindHistory[15];
	CString m_RecentFolders[15];
	CString m_sFilters[10];
	CString m_RecentFiles[15];

	DWORD m_FileListSort;

	CString m_sFilenameFilter;
	CString m_sBinaryFilesFilter;
	CString m_sCppFilesFilter;
	CString m_sIgnoreFilesFilter;

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

	LOGFONT m_NormalLogFont;
	CFont m_NormalFont;
	LOGFONT m_AddedLogFont;
	CFont m_AddedFont;
	LOGFONT m_ErasedLogFont;
	CFont m_ErasedFont;
	int m_FontPointSize;
	void OnFontChanged();
	void UpdateAllDiffViews(LPARAM lHint = 0L, CObject* pHint = NULL);

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

void AddStringToHistory(const CString & str, CString history[], int NumItems, bool CaseSensitive = false);
void LoadHistory(CApplicationProfile & Profile, LPCTSTR szKey, LPCTSTR Format, CString history[], int NumItems, bool Trim);
void LoadHistoryCombo(CComboBox & Combo, CString history[], int NumItems);

CString CreateCustomFilter(LPCTSTR Extension);
int BrowseForFile(int TitleID, CString & Name, CString & BrowseFolder);
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_)
