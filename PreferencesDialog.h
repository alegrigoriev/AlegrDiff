#if !defined(AFX_PREFERENCESDIALOG_H__6B414FED_6394_4F4C_B09C_1978222E1249__INCLUDED_)
#define AFX_PREFERENCESDIALOG_H__6B414FED_6394_4F4C_B09C_1978222E1249__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreferencesDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFilesPreferencePage dialog

class CFilesPreferencePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CFilesPreferencePage)

// Construction
public:
	CFilesPreferencePage();
	~CFilesPreferencePage();

// Dialog Data
	//{{AFX_DATA(CFilesPreferencePage)
	enum { IDD = IDD_PROPPAGE_FILE_PREFERENCES };
	BOOL	m_bUseBinaryFilesFilter;
	BOOL	m_bUseCppFilter;
	BOOL	m_bUseIgnoreFilter;
	CString	m_sBinaryFilesFilter;
	CString	m_sCppFilesFilter;
	CString	m_sIgnoreFilesFilter;
	BOOL	m_AutoReloadChangedFiles;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CFilesPreferencePage)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CFilesPreferencePage)
	afx_msg void OnCheckBinaryFiles();
	afx_msg void OnCheckCCpp();
	afx_msg void OnCheckIgnore();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CComparisionPreferencesPage dialog

class CComparisionPreferencesPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CComparisionPreferencesPage)

// Construction
public:
	CComparisionPreferencesPage();
	~CComparisionPreferencesPage();

// Dialog Data
	//{{AFX_DATA(CComparisionPreferencesPage)
	enum { IDD = IDD_PROPPAGE_COMPARISION_PREFERENCES };
	UINT	m_MinimalLineLength;
	UINT	m_NumberOfIdenticalLines;
	UINT	m_PercentsOfLookLikeDifference;
	UINT	m_MinMatchingChars;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CComparisionPreferencesPage)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CComparisionPreferencesPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CViewPreferencesPage dialog

class CViewPreferencesPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CViewPreferencesPage)

// Construction
public:
	CViewPreferencesPage();
	~CViewPreferencesPage();

// Dialog Data
	//{{AFX_DATA(CViewPreferencesPage)
	enum { IDD = IDD_PROPPAGE_VIEW_PREFERENCES };
	CSpinButtonCtrl	m_Spin;
	UINT	m_nTabIndent;
	BOOL	m_bCancelSelectionOnMerge;
	//}}AFX_DATA

	LOGFONT m_NormalLogFont;
	CFont m_NormalFont;
	DWORD m_NormalTextColor;
	DWORD m_NormalTextBackground;

	LOGFONT m_ErasedLogFont;
	CFont m_ErasedFont;
	DWORD m_ErasedTextColor;
	DWORD m_ErasedTextBackground;

	LOGFONT m_AddedLogFont;
	CFont m_AddedFont;
	DWORD m_AddedTextColor;
	DWORD m_AddedTextBackground;

	int m_FontPointSize;

	bool m_bFontChanged;
	bool m_bColorChanged;
	void FontChanged();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CViewPreferencesPage)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CViewPreferencesPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonNormalFont();
	afx_msg void OnButtonInsertedFont();
	afx_msg void OnButtonErasedFont();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonAddedBackground();
	afx_msg void OnButtonErasedBackground();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CPreferencesPropertySheet

class CPreferencesPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CPreferencesPropertySheet)

// Construction
public:
	CPreferencesPropertySheet(UINT nIDCaption = IDS_PREFERENCES, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CPreferencesPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreferencesPropertySheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	CFilesPreferencePage m_FilesPage;
	CComparisionPreferencesPage m_ComparisionPage;
	CViewPreferencesPage m_ViewPage;

	virtual ~CPreferencesPropertySheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPreferencesPropertySheet)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREFERENCESDIALOG_H__6B414FED_6394_4F4C_B09C_1978222E1249__INCLUDED_)
