// AlegrDiffView.h : interface of the CAlegrDiffView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_)
#define AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CAlegrDiffView : public CView
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
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAlegrDiffView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAlegrDiffView)
	// NOTE - the ClassWizard will add and remove member functions here.
	//    DO NOT EDIT what you see in these blocks of generated code !
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
