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
	typedef CTextFilePairDoc ThisDoc;
	CDiffFileView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDiffFileView)

	TextFilePair* GetFilePair()
	{
		return GetDocument()->GetFilePair();
	}

	// Attributes
public:
	ThisDoc * GetDocument();
	bool m_OnActivateViewEntered;

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
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:

	int m_FirstLineSeen;
	int m_FirstPosSeen;
	int m_NumberMarginWidth;
	int m_NumberOfPanes;
	int m_PaneWithFocus;
	int m_MaxLineWidth[3] = { 0,0,0 };  // combined file, left file, right file
	// visible rectangle (0-relative)
	CRect m_VisibleRect;
	// rectangle for BringCaretToBounds.
	CRect m_PreferredRect;

	TextPosDisplay m_DrawnSelBegin;
	TextPosDisplay m_DrawnSelEnd;

	bool m_LButtonDown;
	bool m_TrackingSelection;
	bool m_ShowLineNumbers;
	int m_LineNumberMarginWidth;
	int m_WheelAccumulator;

	enum ShownVersion { ShownAllText, ShownFile1Version, ShownFile2Version, ShownMerged, };
	ShownVersion m_ShownFileVersion;

	TEXTMETRIC m_FontMetric;
	int m_CharOverhang;

	void UpdateVisibleRectangleBounds();

	int CharWidth() const noexcept { return m_FontMetric.tmAveCharWidth; }
	int LineHeight() const noexcept { return m_FontMetric.tmHeight + m_FontMetric.tmExternalLeading; }
	int LinesInView() const noexcept { return m_VisibleRect.bottom - m_VisibleRect.top; }
	int CharsInView() const noexcept { return m_VisibleRect.right - m_VisibleRect.left; }
	void MakePositionVisible(size_t line, int pos);
	void MakeCaretCenteredRangeVisible(TextPosDisplay NewPos, TextPosDisplay EndPos);
	void MakeCaretVisible()
	{
		const ThisDoc* const pDoc = GetDocument();
		MakePositionVisible(pDoc->m_CaretPos.line, pDoc->m_CaretPos.pos);
	}
	void MakePositionCentered(size_t line, int pos);
	void MakeCaretCentered()
	{
		const ThisDoc* const pDoc = GetDocument();
		MakePositionCentered(pDoc->m_CaretPos.line, pDoc->m_CaretPos.pos);
	}
	void BringPositionsToBounds(TextPosDisplay textpos, TextPosDisplay endpos,
								const CRect & AllowedBounds, const CRect & BringToBounds);

	void InvalidateRange(TextPosDisplay begin, TextPosDisplay end);
	void InvalidateRangeLine(TextPosLine begin, TextPosLine end);

	void MoveCaretBy(int dx, int dy, int flags = SetPositionCancelSelection);
	void SetCaretPosition(int pos, int line, int flags = SetPositionCancelSelection);
	void SetCaretPosition(TextPosDisplay pos, int flags = SetPositionCancelSelection);
	void SetCaretPosition(TextPosLine pos, eFileScope FileScope, int flags = SetPositionCancelSelection);
	TextPosDisplay PointToTextPos(POINT point, int pane = -1);

	void CreateAndShowCaret();
	void UpdateVScrollBar();
	void DoVScroll(int nLinesToScroll); // > 0 - scroll up (to see lines toward end),
	// < 0 - scroll down (to see lines to the begin of the file)
	void VScrollToTheLine(int nLine);

	void DoHScroll(int nCharsToScroll); // > 0 - scroll left (to see chars toward end),
	// < 0 - scroll right (to see chars to the begin of the line)
	void HScrollToThePos(int nPos);
	void UpdateHScrollBar();
	void OnMetricsChange();
	void UpdateFileLineWidth();
	void CaretToHome(int flags);
	void CaretToEnd(int flags);

	int PointToPaneNumber(int x);
	int PointToPaneOffset(int x, int nPane = -1);
	int GetPaneWidth(RECT const *client_rect = nullptr);
	ULONG HitTest(POINT p, int pane = -1, bool NeedCharIndex = false); // if NeedCharIndex, the precise index of the hit character is calculated
	enum
	{
		HitTestLineNumberMask = 0xFFF,  // lines on screen
		HitTestLineNumberShift = 0,
		HitTestPaneMask = 0x3000,
		HitTestPaneShift = 12,
		HitTestPositionMask = 0x7FFF0000, // if NeedCharIndex, this is character index, rounded to closest inter-char gap
		HitTestPositionShift = 16,
		// if !NeedCharIndex, this is pixel relative to the beginning of the text area of the pane
		HitTestNumberMargin = 0x80000000,
	};
	bool OnFind(bool PickWordOrSelection, bool bBackwards, bool bInvokeDialog);

protected:
	void DrawStringSections(CDC* pDC, CPoint point,
							ListHead<StringSection> const * pSection,
							int nSkipChars, int nVisibleChars, int nTabIndent,
							int SelBegin, int SelEnd, eFileScope nFileSelect,
							CThisApp::NORMAL_OR_SELECTED_COLOR const* DefaultColor);

	virtual ~CDiffFileView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView*);
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
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnEditGotonextdiff();
	afx_msg void OnEditGotoprevdiff();
	afx_msg void OnViewShowLineNumbers();
	afx_msg void OnUpdateViewShowLineNumbers(CCmdUI* pCmdUI);
	afx_msg void OnEditFind();
	afx_msg void OnEditFindNext();
	afx_msg void OnEditFindPrev();
	afx_msg void OnEditFindWordNext();
	afx_msg void OnEditFindWordPrev();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditGotoline();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEditSelectAll();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateViewIgnoreWhitespaces(CCmdUI* pCmdUI);
	afx_msg void OnViewSideBySide();
	afx_msg void OnUpdateViewSideBySide(CCmdUI *pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	virtual void OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame);
public:
};

/////////////////////////////////////////////////////////////////////////////
#ifndef _DEBUG  // debug version in AlegrDiffView.cpp
inline CTextFilePairDoc* CDiffFileView::GetDocument()
{
	return (ThisDoc*)m_pDocument;
}
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIFFFILEVIEW_H__7F32E3F1_2705_407E_8A76_BC606A6B88D1__INCLUDED_)
