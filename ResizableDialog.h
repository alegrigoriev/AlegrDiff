// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#if !defined(AFX_RESIZABLEDIALOG_H__1E590ECD_BEE6_4C1C_9E9C_A8F58A8FA54A__INCLUDED_)
#define AFX_RESIZABLEDIALOG_H__1E590ECD_BEE6_4C1C_9E9C_A8F58A8FA54A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResizableDialog.h : header file
//
#include "UiUpdatedDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CResizableDialog dialog

class CResizableDialog : public CUiUpdatedDlg
{
	DECLARE_DYNAMIC(CResizableDialog)
// Construction
protected:
	CResizableDialog(UINT id, CWnd* pParent);   // standard constructor
public:

// Dialog Data
	//{{AFX_DATA(CResizableDialog)
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizableDialog)
	//}}AFX_VIRTUAL
	virtual INT_PTR DoModal();

// Implementation
protected:
	CSize m_PrevSize;
	MINMAXINFO m_mmxi;
	int m_DlgWidth;
	int m_DlgHeight;

	enum
	{
		CenterHorizontally = 1,
		ExpandRight = 2,
		MoveRight = 4,
		ExpandDown = 8,
		MoveDown = 0x10,
	};
	struct ResizableDlgItem
	{
		UINT Id;
		UINT flags;
	};

	ResizableDlgItem const * m_pResizeItems;
	int m_pResizeItemsCount;

	virtual void OnMetricsChange();
	// Generated message map functions
	//{{AFX_MSG(CResizableDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg UINT OnNcHitTest(CPoint point);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESIZABLEDIALOG_H__1E590ECD_BEE6_4C1C_9E9C_A8F58A8FA54A__INCLUDED_)
