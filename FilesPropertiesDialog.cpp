// FilesPropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "FilesPropertiesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFilesPropertiesDialog dialog


CFilesPropertiesDialog::CFilesPropertiesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFilesPropertiesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFilesPropertiesDialog)
	m_FirstFileName = _T("");
	m_SecondFileName = _T("");
	m_FirstTime = _T("");
	m_SecondTime = _T("");
	m_ComparisonResult = _T("");
	//}}AFX_DATA_INIT
}


void CFilesPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilesPropertiesDialog)
	DDX_Text(pDX, IDC_EDIT_FIRST_FILENAME, m_FirstFileName);
	DDX_Text(pDX, IDC_EDIT_SECOND_FILENAME, m_SecondFileName);
	DDX_Text(pDX, IDC_EDIT_FIRST_LAST_MODIFIED, m_FirstTime);
	DDX_Text(pDX, IDC_EDIT_SECOND_LAST_MODIFIED, m_SecondTime);
	DDX_Text(pDX, IDC_STATIC_COMPARISON_RESULT, m_ComparisonResult);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFilesPropertiesDialog, CDialog)
	//{{AFX_MSG_MAP(CFilesPropertiesDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilesPropertiesDialog message handlers
