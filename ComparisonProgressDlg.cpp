// ComparisonProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "ComparisonProgressDlg.h"
#include "AlegrDiffDoc.h"

// CComparisonProgressDlg dialog

IMPLEMENT_DYNAMIC(CComparisonProgressDlg, CProgressDialog)
CComparisonProgressDlg::CComparisonProgressDlg(CWnd* pParent /*=NULL*/)
	: CProgressDialog(CComparisonProgressDlg::IDD, pParent)
	, m_pDoc(NULL)
{
}

CComparisonProgressDlg::~CComparisonProgressDlg()
{
}

void CComparisonProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CProgressDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CComparisonProgressDlg, CProgressDialog)
END_MESSAGE_MAP()


// CComparisonProgressDlg message handlers
unsigned CComparisonProgressDlg::ThreadProc()
{
	if (NULL != m_pDoc)
	{
		m_pDoc->CompareDirectoriesFunction(this);
	}
	if (NULL != m_hWnd)
	{
		::PostMessage(m_hWnd, WM_COMMAND, IDYES, 0);
	}
	return 0;
}
