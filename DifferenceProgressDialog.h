#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CDifferenceProgressDialog dialog

class CDifferenceProgressDialog : public CDialog
{
	DECLARE_DYNAMIC(CDifferenceProgressDialog)

public:
	CDifferenceProgressDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDifferenceProgressDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_DIFFERENCE_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_Progress;
	CStatic m_Percent;
	class CBinaryCompareView * m_pView;
	LONGLONG BeginAddr;
	LONGLONG EndAddr;
	int m_PercentCompleted;
	int m_PercentDisplayed;
	BOOL m_StopSearch;
	BOOL m_SearchCompleted;
	virtual BOOL OnInitDialog();
	LRESULT OnKickIdle(WPARAM, LPARAM);
protected:
	virtual void OnCancel();
	virtual void OnOK();
};
