#if !defined(AFX_SAVEFILELISTDLG_H__01A887BA_44C9_43AE_9427_ED9D1A32CFCC__INCLUDED_)
#define AFX_SAVEFILELISTDLG_H__01A887BA_44C9_43AE_9427_ED9D1A32CFCC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveFileListDlg.h : header file
//
#include "UiUpdatedDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CSaveFileListDlg dialog

class CSaveFileListDlg : public CUiUpdatedDlg
{
// Construction
public:
	CSaveFileListDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_DIALOG_SAVE_FILE_LIST };
protected:
	//{{AFX_DATA(CSaveFileListDlg)
	CEdit	m_eFilename;
	CString	m_sFilename;
	BOOL	m_bIncludeComparisonResult;
	BOOL	m_bIncludeDifferentFiles;
	BOOL	m_bIncludeDifferentInBlanksFiles;
	BOOL	m_bIncludeDifferentVersionFiles;
	BOOL	m_bIncludeFolder1OnlyFiles;
	BOOL	m_bIncludeFolder2OnlyFiles;
	BOOL	m_bIncludeIdenticalFiles;
	BOOL	m_bIncludeSubdirectoryName;
	BOOL	m_bIncludeTimestamp;
	BOOL	m_bIncludeLength;
	int		m_IncludeFilesSelect;
	//}}AFX_DATA
	BOOL    m_bEnableSelectedItems;
public:
	enum FilesFilter
	{
		IncludeAllFiles = 0,
		IncludeSelectedFiles = 1,
		IncludeGroups = 2,
	};

	FilesFilter GetFilesFilter() const
	{
		return (FilesFilter) m_IncludeFilesSelect;
	}
	LPCTSTR GetFilename() const
	{
		return m_sFilename;
	}

	bool IncludeIdenticalFiles() const
	{
		return FALSE != m_bIncludeIdenticalFiles;
	}

	bool IncludeDifferentFiles() const
	{
		return FALSE != m_bIncludeDifferentFiles;
	}

	bool IncludeDifferentInSpacesFiles() const
	{
		return FALSE != m_bIncludeDifferentInBlanksFiles;
	}

	bool IncludeVersionInfoDifferentFiles() const
	{
		return FALSE != m_bIncludeDifferentVersionFiles;
	}

	bool IncludeFolder1Files() const
	{
		return FALSE != m_bIncludeFolder1OnlyFiles;
	}

	bool IncludeFolder2Files() const
	{
		return FALSE != m_bIncludeFolder2OnlyFiles;
	}

	bool IncludeSubdirectoryName() const
	{
		return FALSE != m_bIncludeSubdirectoryName;
	}

	bool IncludeTimestamp() const
	{
		return FALSE != m_bIncludeTimestamp;
	}

	bool IncludeLength() const
	{
		return FALSE != m_bIncludeLength;
	}

	bool IncludeComparisonResult() const
	{
		return FALSE != m_bIncludeComparisonResult;
	}

	void EnableSelectedItems()
	{
		m_bEnableSelectedItems = TRUE;
	}

	CApplicationProfile m_Profile;
// Overrides
	// ClassWizard generated virtual function overrides
	void OnUpdateOk(CCmdUI * pCmdUI);
	void OnUpdateCheckIncludeGroup(CCmdUI * pCmdUI);
	//{{AFX_VIRTUAL(CSaveFileListDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSaveFileListDlg)
	afx_msg void OnButtonBrowse();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEFILELISTDLG_H__01A887BA_44C9_43AE_9427_ED9D1A32CFCC__INCLUDED_)
