#if !defined(AFX_SAVEFILELISTDLG_H__01A887BA_44C9_43AE_9427_ED9D1A32CFCC__INCLUDED_)
#define AFX_SAVEFILELISTDLG_H__01A887BA_44C9_43AE_9427_ED9D1A32CFCC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveFileListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaveFileListDlg dialog

class CSaveFileListDlg : public CDialog
{
// Construction
public:
	CSaveFileListDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSaveFileListDlg)
	enum { IDD = IDD_DIALOG_SAVE_FILE_LIST };
	CEdit	m_eFilename;
	BOOL	m_bIncludeComparisonResult;
	BOOL	m_bIncludeDifferentFiles;
	BOOL	m_bIncludeDifferentInBlanksFiles;
	BOOL	m_bIncludeFolder1OnlyFiles;
	BOOL	m_bIncludeFolder2OnlyFiles;
	BOOL	m_bIncludeIdenticalFiles;
	BOOL	m_bIncludeSubdirectoryName;
	BOOL	m_bIncludeTimestamp;
	CString	m_sFilename;
	int		m_IncludeFilesSelect;
	//}}AFX_DATA

	bool m_bNeedUpdateControls;
	CApplicationProfile m_Profile;
// Overrides
	// ClassWizard generated virtual function overrides
	LRESULT OnKickIdle(WPARAM, LPARAM);
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
