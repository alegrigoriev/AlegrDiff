#pragma once
#include "BinaryCompareDoc.h"

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
	int m_AddressMarginWidth;
	BOOL m_LButtonDown;
	BOOL m_TrackingSelection;

	int GetMaxChars() const;
	BOOL m_bShowSecondFile;
	CRect m_VisibleRect;
	CRect m_PreferredRect;

	int CharWidth() const { return m_FontMetric.tmAveCharWidth; }
	int LineHeight() const { return m_FontMetric.tmHeight + m_FontMetric.tmExternalLeading; }
	int LinesInView() const { return m_VisibleRect.bottom - m_VisibleRect.top; }
	int CharsInView() const { return m_VisibleRect.right - m_VisibleRect.left; }

	CPoint PositionToPoint(LONGLONG pos);
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

	void MakePositionVisible(LONGLONG pos);
	void MakeCaretCenteredRangeVisible(LONGLONG NewPos, LONGLONG EndPos);
	void MakeCaretVisible()
	{
		MakePositionVisible(GetDocument()->m_CaretPos);
	}
	void MakePositionCentered(LONGLONG pos);
	void MakeCaretCentered()
	{
		MakePositionCentered(GetDocument()->m_CaretPos);
	}
	void BringPositionsToBounds(LONGLONG textpos, LONGLONG endpos,
								const CRect & AllowedBounds, const CRect & BringToBounds);

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
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnWordsize1byte();
	afx_msg void OnUpdateWordsize1byte(CCmdUI *pCmdUI);
	afx_msg void OnWordsize2bytes();
	afx_msg void OnUpdateWordsize2bytes(CCmdUI *pCmdUI);
	afx_msg void OnWordsize4bytes();
	afx_msg void OnUpdateWordsize4bytes(CCmdUI *pCmdUI);
	afx_msg void OnWordsize8bytes();
	afx_msg void OnUpdateWordsize8bytes(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in AlegrDiffView.cpp
inline CBinaryCompareDoc * CBinaryCompareView::GetDocument()
{ return (CBinaryCompareDoc*)m_pDocument; }
#endif

