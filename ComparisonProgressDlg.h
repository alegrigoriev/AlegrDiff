#pragma once
#include "ProgressDialog.h"


// CComparisonProgressDlg dialog

class CComparisonProgressDlg : public CProgressDialog
{
	DECLARE_DYNAMIC(CComparisonProgressDlg)

public:
	CComparisonProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CComparisonProgressDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_COMPARISON_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
