// ComparisonProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "ComparisonProgressDlg.h"
#include "AlegrDiffDoc.h"

// CComparisonProgressDlg dialog

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
		UINT result = m_pDoc->CompareDirectoriesFunction(this);
		return SignalDialogEnd(result);
	}
	return 0;
}

void CComparisonProgressDlg::OnCancel()
{
	if (m_pDoc->CanCancelComparison())
	{
		EndDialog(IDCANCEL);
	}
}
