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
					lpszFileName, dwFlags | OFN_EXPLORER, lpszFilter, pParentWnd,
					OpenfilenameSize()),
		m_pResizeItems(NULL),
		m_ResizeItemsCount(0)
	{
		m_ParentWnd.m_pSubDialog = this;
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

	virtual INT_PTR DoModal();
	//{{AFX_MSG(CResizableFileDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	class CParentWnd : public CWnd
	{
	public:
		DECLARE_DYNAMIC(CParentWnd)
		CResizableFileDialog * m_pSubDialog;

		LRESULT OnGetFilePath(WPARAM, LPARAM);
		LRESULT OnGetFolderPath(WPARAM, LPARAM);
		LRESULT OnGetSpec(WPARAM, LPARAM);
		LRESULT OnSetControlText(WPARAM, LPARAM);
		LRESULT OnSetDefExt(WPARAM, LPARAM);
		DECLARE_MESSAGE_MAP()
	};
	static UINT_PTR CALLBACK ResizableFileDialogHook(
													HWND hdlg,      // handle to child dialog box
													UINT uiMsg,     // message identifier
													WPARAM wParam,  // message parameter
													LPARAM lParam   // message parameter
													);
	CParentWnd  m_ParentWnd;

	void UpdateOfn(OPENFILENAMEA & ofn);
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
