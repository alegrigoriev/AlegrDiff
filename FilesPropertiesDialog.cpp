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


CFilesPropertiesDialog::CFilesPropertiesDialog(class FilePair * pPair, CWnd* pParent /*=NULL*/)
	: CDialog(CFilesPropertiesDialog::IDD, pParent)
	, m_FirstLength(_T(""))
	, m_SecondLength(_T(""))
{
	//{{AFX_DATA_INIT(CFilesPropertiesDialog)
	m_FirstFileName = _T("");
	m_SecondFileName = _T("");
	m_FirstTime = _T("");
	m_SecondTime = _T("");
	m_ComparisonResult = _T("");
	//}}AFX_DATA_INIT
	m_pPair = pPair;

	if (m_pPair != NULL)
	{
		if (NULL != m_pPair->pFirstFile)
		{
			m_FirstFileName = m_pPair->pFirstFile->GetFullName();
			m_FirstTime = FileTimeToStr(m_pPair->pFirstFile->GetLastWriteTime());
			m_FirstLength.Format(_T("%s (%s)"),
								LPCTSTR(FileLengthToStrKb(m_pPair->pFirstFile->GetFileLength())),
								LPCTSTR(UlonglongToStr(m_pPair->pFirstFile->GetFileLength())));
		}

		if (NULL != m_pPair->pSecondFile)
		{
			m_SecondFileName = m_pPair->pSecondFile->GetFullName();
			m_SecondTime = FileTimeToStr(m_pPair->pSecondFile->GetLastWriteTime());
			m_SecondLength.Format(_T("%s (%s)"),
								LPCTSTR(FileLengthToStrKb(m_pPair->pSecondFile->GetFileLength())),
								LPCTSTR(UlonglongToStr(m_pPair->pSecondFile->GetFileLength())));
		}

		m_ComparisonResult = m_pPair->GetComparisonResultStr();
	}
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
	DDX_Text(pDX, IDC_EDIT_FIRST_LENGTH, m_FirstLength);
	DDX_Text(pDX, IDC_EDIT_SECOND_LENGTH, m_SecondLength);
}


BEGIN_MESSAGE_MAP(CFilesPropertiesDialog, CDialog)
	//{{AFX_MSG_MAP(CFilesPropertiesDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
//    ON_STN_CLICKED(IDC_EDIT_SECOND_LAST_MODIFIED, OnStnClickedEditSecondLastModified)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilesPropertiesDialog message handlers

