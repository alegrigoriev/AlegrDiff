#if !defined(AFX_FILEDIALOGWITHHISTORY_H__F63C5621_0671_4E36_A5E8_E3182EB884BB__INCLUDED_)
#define AFX_FILEDIALOGWITHHISTORY_H__F63C5621_0671_4E36_A5E8_E3182EB884BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileDialogWithHistory.h : header file
//
#include "ApplicationProfile.h"
////////////////////////////////////////////////////////////////////////////
// CFileDialogWithHistory dialog

class CResizableFileDialog : public CFileDialog
{
	DECLARE_DYNAMIC(CResizableFileDialog)

public:
	CResizableFileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
						LPCTSTR lpszDefExt = NULL,
						LPCTSTR lpszFileName = NULL,
						DWORD dwFlags = OFN_HIDEREADONLY,
						LPCTSTR lpszFilter = NULL,
						CWnd* pParentWnd = NULL)
		: CFileDialog(bOpenFileDialog, lpszDefExt,
					lpszFileName, dwFlags, lpszFilter, pParentWnd,
					OpenfilenameSize()),
		m_pResizeItems(NULL),
		m_ResizeItemsCount(0)
	{
		m_PrevSize.cx = -1;
		m_PrevSize.cy = -1;
		m_ofn.Flags |= OFN_ENABLESIZING;
	}
	CString GetNextPathName(POSITION& pos) const;
protected:
	static size_t OpenfilenameSize();

	CSize m_PrevSize;
	enum
	{
		CenterHorizontally = 1,
		ExpandRight = 2,
		MoveRight = 4,
	};
	struct ResizableDlgItem
	{
		UINT Id;
		UINT flags;
	};

	ResizableDlgItem const * m_pResizeItems;
	int m_ResizeItemsCount;
	//{{AFX_MSG(CResizableFileDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CFileDialogWithHistory dialog

class CFileDialogWithHistory : public CResizableFileDialog
{
	DECLARE_DYNAMIC(CFileDialogWithHistory)

public:
	CFileDialogWithHistory(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
							LPCTSTR lpszDefExt = NULL,
							LPCTSTR lpszFileName = NULL,
							DWORD dwFlags = OFN_HIDEREADONLY,
							LPCTSTR lpszFilter = NULL,
							CWnd* pParentWnd = NULL, LPCTSTR Section = _T("RecentOpenDirs"),
							LPCTSTR KeyFormat = _T("Dir%d"), int NumStrings = 15);
	CFileDialogWithHistory(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
							CStringHistory * pSourceHistory, LPCTSTR lpszDefExt = NULL,
							LPCTSTR lpszFileName = NULL,
							DWORD dwFlags = OFN_HIDEREADONLY,
							LPCTSTR lpszFilter = NULL,
							CWnd* pParentWnd = NULL);

	virtual INT_PTR DoModal();

protected:
	CApplicationProfile m_Profile;
	CStringHistory m_RecentFolders;
	CString m_SubstituteInitialFolder;

	virtual void OnFolderChange();
	virtual void OnInitDone();
	virtual BOOL OnFileNameOK();
	//{{AFX_MSG(CFileDialogWithHistory)
	afx_msg void OnComboSelendOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEDIALOGWITHHISTORY_H__F63C5621_0671_4E36_A5E8_E3182EB884BB__INCLUDED_)
