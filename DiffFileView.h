#if !defined(AFX_DIFFFILEVIEW_H__7F32E3F1_2705_407E_8A76_BC606A6B88D1__INCLUDED_)
#define AFX_DIFFFILEVIEW_H__7F32E3F1_2705_407E_8A76_BC606A6B88D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DiffFileView.h : header file
//
#include "FileListSupport.h"
#include "AlegrDiffDoc.h"
/////////////////////////////////////////////////////////////////////////////
// CDiffFileView view

class CDiffFileView : public CView
{
protected:
	CDiffFileView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDiffFileView)

// Attributes
public:
	CAlegrDiffDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiffFileView)
public:
	virtual void OnInitialUpdate();
protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	FilePair * m_pFilePair;
	bool m_UseLinePairArray;
	bool m_BaseOnFirstFile;
	int m_FirstLineSeen;
	int m_FirstPosSeen;
	int m_CaretLine;
	int m_CaretPos;
	int m_TotalLines;
	CFont m_NormalFont;
	CFont m_UnderlineFont;
	CFont m_StrikeoutFont;
	TEXTMETRIC m_FontMetric;

	int CharWidth() const { return m_FontMetric.tmAveCharWidth; }
	int LineHeight() const { return m_FontMetric.tmHeight + m_FontMetric.tmExternalLeading; }
	int LinesInView() const;
	int CharsInView() const;
	void MakePositionVisible(int line, int pos);
	void MoveCaretBy(int dx, int dy, bool bCancelSelection = true)
	{
		SetCaretPosition(m_CaretPos + dx, m_CaretLine + dy, bCancelSelection);
	}
	void SetCaretPosition(int pos, int line, bool bCancelSelection = true);
	void CreateAndShowCaret();
	void UpdateVScrollBar();
	void DoVScroll(int nLinesToScroll); // > 0 - scroll up (to see lines toward end),
	// < 0 - scroll down (to see lines to the begin of the file)
	void VScrollToTheLine(int nLine);

	void DoHScroll(int nCharsToScroll); // > 0 - scroll left (to see chars toward end),
	// < 0 - scroll right (to see chars to the begin of the line)
	void HScrollToThePos(int nPos);
	void UpdateHScrollBar();
	void CancelSelection();
protected:
	void DrawStringSections(CDC* pDC, CPoint point,
							const StringSection * pSection,
							int nSkipChars, int nVisibleChars, int nTabIndent);

	virtual ~CDiffFileView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CDiffFileView)
	afx_msg void OnWindowCloseDiff();
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#ifndef _DEBUG  // debug version in AlegrDiffView.cpp
inline CAlegrDiffDoc* CDiffFileView::GetDocument()
{ return (CAlegrDiffDoc*)m_pDocument; }
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIFFFILEVIEW_H__7F32E3F1_2705_407E_8A76_BC606A6B88D1__INCLUDED_)
