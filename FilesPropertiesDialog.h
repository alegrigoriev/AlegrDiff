#if !defined(AFX_FILESPROPERTIESDIALOG_H__893E7CD5_4075_4147_A979_EAE4FE0B3470__INCLUDED_)
#define AFX_FILESPROPERTIESDIALOG_H__893E7CD5_4075_4147_A979_EAE4FE0B3470__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FilesPropertiesDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFilesPropertiesDialog dialog

class CFilesPropertiesDialog : public CDialog
{
// Construction
public:
	CFilesPropertiesDialog(class FilePair * pPair, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFilesPropertiesDialog)
	enum { IDD = IDD_DIALOG_PROPERTIES };
	CString	m_FirstFileName;
	CString	m_SecondFileName;
	CString	m_FirstTime;
	CString	m_SecondTime;
	CString	m_ComparisonResult;
	//}}AFX_DATA

	CString m_FirstLength;  // 10KB (10,987)
	CString m_SecondLength;

	class FilePair * m_pPair;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilesPropertiesDialog)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFilesPropertiesDialog)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILESPROPERTIESDIALOG_H__893E7CD5_4075_4147_A979_EAE4FE0B3470__INCLUDED_)
