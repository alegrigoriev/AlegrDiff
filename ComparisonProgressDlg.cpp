// ComparisonProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "ComparisonProgressDlg.h"

// CComparisonProgressDlg dialog

IMPLEMENT_DYNAMIC(CComparisonProgressDlg, CProgressDialog)
CComparisonProgressDlg::CComparisonProgressDlg(CWnd* pParent /*=NULL*/)
	: CProgressDialog(CComparisonProgressDlg::IDD, pParent)
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
