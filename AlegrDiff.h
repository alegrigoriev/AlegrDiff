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
	void NotifyFilePairChanged(FilePair * pPair);
	CAlegrDiffApp();
	~CAlegrDiffApp();

	CApplicationProfile Profile;
	int m_MaxSearchDistance;
	int m_MinIdenticalLines;
	int m_MinIdenticalChars;
	int m_TabIndent;

	DWORD m_NormalTextColor;
	DWORD m_ErasedTextColor;
	DWORD m_AddedTextColor;
	DWORD m_TextBackgroundColor;
	DWORD m_AcceptedTextBackgroundColor;
	DWORD m_DiscardedTextBackgroundColor;
	DWORD m_SelectedTextColor;

	bool m_bRecurseSubdirs;
	bool m_bUseBinaryFilesFilter;
	bool m_bUseCppFilter;
	bool m_bUseIgnoreFilter;
	bool m_bAdvancedCompareDialog;
	bool m_BinaryComparision;
	bool m_AutoReloadChangedFiles;
	bool m_bIgnoreWhitespaces;

	CString m_FileDir1;
	CString m_FileDir2;
	int m_UsedFilenameFilter;

	CString m_sFilenameFilter;
	CString m_sBinaryFilesFilter;
	CString m_sCppFilesFilter;
	CString m_sIgnoreFilesFilter;

	CString m_FindString;
	bool m_bCaseSensitive;

	DWORD	m_MinimalLineLength;
	DWORD	m_NumberOfIdenticalLines;
	DWORD	m_PercentsOfLookLikeDifference;

	LOGFONT m_NormalLogFont;
	CFont m_NormalFont;
	LOGFONT m_AddedLogFont;
	CFont m_AddedFont;
	LOGFONT m_ErasedLogFont;
	CFont m_ErasedFont;
	int m_FontPointSize;
	void OnFontChanged();
	void UpdateAllDiffViews();

	void OpenFilePairView(FilePair * pPair);
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlegrDiffApp)
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL
// Implementation
	CMultiDocTemplate * m_pFileDiffTemplate;
	CMultiDocTemplate * m_pListDiffTemplate;

	//{{AFX_MSG(CAlegrDiffApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileComparedirectories();
	afx_msg void OnFileComparefiles();
	afx_msg void OnFilePreferences();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

typedef CAlegrDiffApp CThisApp;
inline CThisApp * GetApp()
{
	return (CThisApp *) AfxGetApp();
}

void ModifyOpenFileMenu(CCmdUI* pCmdUI, class FileItem * pFile, LPCTSTR Prefix);
void OpenFileForEditing(class FileItem * pFile);
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_)
