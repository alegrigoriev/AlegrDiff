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

	LONGLONG m_ScreenFilePos;
	LONGLONG m_DrawnSelBegin;
	LONGLONG m_DrawnSelEnd;

	TEXTMETRIC m_FontMetric;
	int m_WheelAccumulator;
	int m_BytesPerLine;
	int m_WordSize;   // 1, 2, 4, 8
	int m_FirstPosSeen;
	long m_ScrollDataScale;
	BOOL m_LButtonDown;

	int GetMaxChars() const;
	BOOL m_bShowSecondFile;
	CRect m_VisibleRect;

	int CharWidth() const { return m_FontMetric.tmAveCharWidth; }
	int LineHeight() const { return m_FontMetric.tmHeight + m_FontMetric.tmExternalLeading; }
	int LinesInView() const { return m_VisibleRect.bottom - m_VisibleRect.top; }
	int CharsInView() const { return m_VisibleRect.right - m_VisibleRect.left; }
	void OnMetricsChange();

	void MoveCaretBy(int dx, int dy, int flags = SetPositionCancelSelection);
	void SetCaretPosition(LONGLONG Addr, int flags = SetPositionCancelSelection);
	void CreateAndShowCaret();
	void UpdateVScrollBar();

	void DoVScroll(int nLinesToScroll); // > 0 - scroll up (to see lines toward end),
	// < 0 - scroll down (to see lines to the begin of the file)
	void VScrollToTheAddr(LONGLONG Addr);

	void DoHScroll(int nCharsToScroll); // > 0 - scroll left (to see chars toward end),
	// < 0 - scroll right (to see chars to the begin of the line)
	void HScrollToThePos(int nPos);
	void UpdateHScrollBar();
	void CaretToHome(int flags);
	void CaretToEnd(int flags);
	void InvalidateRange(LONGLONG begin, LONGLONG end);

	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnWindowCloseDiff();
public:
	virtual void OnInitialUpdate();
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};

#ifndef _DEBUG  // debug version in AlegrDiffView.cpp
inline CBinaryCompareDoc * CBinaryCompareView::GetDocument()
{ return (CBinaryCompareDoc*)m_pDocument; }
#endif

