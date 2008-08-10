#pragma once
#include "ProgressDialog.h"


// CComparisonProgressDlg dialog

class CComparisonProgressDlg : public CProgressDialog
{
public:
	CComparisonProgressDlg(class CAlegrDiffDoc * pDoc, CWnd* pParent = NULL);   // standard constructor
	virtual ~CComparisonProgressDlg();
	class CAlegrDiffDoc * m_pDoc;
// Dialog Data
	enum { IDD = IDD_DIALOG_COMPARISON_PROGRESS };

protected:
	//virtual LRESULT OnKickIdle(WPARAM, LPARAM);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual unsigned ThreadProc();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
};
