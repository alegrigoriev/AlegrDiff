#pragma once


// CBinaryCompareView view

class CBinaryCompareView : public CView
{
	DECLARE_DYNCREATE(CBinaryCompareView)

protected:
	CBinaryCompareView();           // protected constructor used by dynamic creation
	virtual ~CBinaryCompareView();

public:
	class CBinaryCompareDoc * GetDocument();

	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in AlegrDiffView.cpp
inline CBinaryCompareDoc * CBinaryCompareView::GetDocument()
{ return (CBinaryCompareDoc*)m_pDocument; }
#endif

