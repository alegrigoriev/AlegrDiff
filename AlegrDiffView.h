// AlegrDiffView.h : interface of the CAlegrDiffView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_)
#define AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afxcview.h>

class CAlegrDiffView : public CListView
{
protected: // create from serialization only
	CAlegrDiffView();
	DECLARE_DYNCREATE(CAlegrDiffView)

// Attributes
public:
	CAlegrDiffDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlegrDiffView)
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	enum eColumns {ColumnName, ColumnSubdir, ColumnDate1, ColumnDate2, ColumnComparisionResult };
	eColumns m_SortColumn;
	bool m_bAscendingOrder;
	virtual ~CAlegrDiffView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CArray<FilePair *,FilePair *> m_PairArray;

	void BuildSortedPairArray(CArray<FilePair *,FilePair *> & PairArray, FilePair * pPairs, int nCount);

// Generated message map functions
protected:
	//{{AFX_MSG(CAlegrDiffView)
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileEditFirst();
	afx_msg void OnUpdateFileEditFirst(CCmdUI* pCmdUI);
	afx_msg void OnFileEditSecond();
	afx_msg void OnUpdateFileEditSecond(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in AlegrDiffView.cpp
inline CAlegrDiffDoc* CAlegrDiffView::GetDocument()
{ return (CAlegrDiffDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_)
