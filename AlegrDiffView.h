// AlegrDiffView.h : interface of the CAlegrDiffView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_)
#define AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afxcview.h>
#include <vector>
using namespace std;

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
	void AddListViewItem(FilePair * pPair, int item);
	BOOL CopySelectedFiles(bool bSecondDir);
	eColumns m_SortColumn;
	eColumns m_PrevSortColumn;

	// keeps order of columns (even those non-visible)
	UCHAR m_ColumnArray[MaxColumns];
	//UCHAR m_SubitemArray[MaxColumns];
	USHORT m_ColumnWidthArray[MaxColumns];
	int m_ColumnToItem[MaxColumns];
	int m_ItemToColumn[MaxColumns];
	void ResetColumnsArray();

	bool m_bAscendingOrder;
	bool m_bPrevAscendingOrder;
	void UpdateAppSort();

	void PrintColumnOrder();

	virtual ~CAlegrDiffView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	vector<FilePair *> m_PairArray;

	void BuildSortedPairArray(vector<FilePair *> & PairArray, FilePair * pPairs, int nCount);

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
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnListviewOpen();
	afx_msg void OnUpdateListviewOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileCopyFirstDir(CCmdUI* pCmdUI);
	afx_msg void OnFileCopyFirstDir();
	afx_msg void OnUpdateFileCopySecondDir(CCmdUI* pCmdUI);
	afx_msg void OnFileCopySecondDir();
	afx_msg void OnFileSaveList();
	afx_msg void OnUpdateFileSaveList(CCmdUI* pCmdUI);
	afx_msg void OnViewHideselectedfiles();
	afx_msg void OnUpdateViewHideselectedfiles(CCmdUI* pCmdUI);
	afx_msg void OnViewShowallfiles();
	afx_msg void OnUpdateViewShowallfiles(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnListviewFilelength();
	afx_msg void OnUpdateListviewFilelength(CCmdUI *pCmdUI);
	afx_msg void OnListviewModificationtime();
	afx_msg void OnUpdateListviewModificationtime(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortby1stlength();
	afx_msg void OnUpdateListviewSortby1stlength(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortby1stmodificationdate();
	afx_msg void OnUpdateListviewSortby1stmodificationdate(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortby2ndlength();
	afx_msg void OnUpdateListviewSortby2ndlength(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortbyComparisonresult();
	afx_msg void OnUpdateListviewSortbyComparisonresult(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortbyFolder();
	afx_msg void OnUpdateListviewSortbyFolder(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortbyName();
	afx_msg void OnUpdateListviewSortbyName(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortbyDescendingorder();
	afx_msg void OnUpdateListviewSortbyDescendingorder(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortby2ndmodificationdate();
	afx_msg void OnUpdateListviewSortby2ndmodificationdate(CCmdUI *pCmdUI);
	afx_msg void OnHdnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnEnddrag(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // debug version in AlegrDiffView.cpp
inline CAlegrDiffDoc* CAlegrDiffView::GetDocument()
{ return (CAlegrDiffDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_)
