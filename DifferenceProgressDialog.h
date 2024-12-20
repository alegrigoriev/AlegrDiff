#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "ProgressDialog.h"

// CDifferenceProgressDialog dialog

class CDifferenceProgressDialog : public CProgressDialog
{
public:
	CDifferenceProgressDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDifferenceProgressDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_DIFFERENCE_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	class CBinaryCompareDoc * m_pDoc;
	virtual unsigned ThreadProc();
	LONGLONG BeginAddr;
	LONGLONG EndAddr;

	virtual BOOL OnInitDialog();
};
