#if !defined(AFX_DIFFFILEVIEW_H__7F32E3F1_2705_407E_8A76_BC606A6B88D1__INCLUDED_)
#define AFX_DIFFFILEVIEW_H__7F32E3F1_2705_407E_8A76_BC606A6B88D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DiffFileView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDiffFileView view

class CDiffFileView : public CView
{
protected:
	CDiffFileView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDiffFileView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiffFileView)
protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CDiffFileView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CDiffFileView)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIFFFILEVIEW_H__7F32E3F1_2705_407E_8A76_BC606A6B88D1__INCLUDED_)
