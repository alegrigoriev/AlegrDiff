// DiffFileView.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DiffFileView.h"
#include "GoToLineDialog.h"
#include "ChildFrm.h"
#include "FindDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDiffFileView

IMPLEMENT_DYNCREATE(CDiffFileView, CView)

CDiffFileView::CDiffFileView()
	: m_FirstLineSeen(0),
	m_FirstPosSeen(0),
	m_LButtonDown(false),
	m_TrackingSelection(false),
	m_DrawnSelBegin(0, 0),
	m_VisibleRect(0, 0, 0, 0),
	m_PreferredRect(0, 0, 0, 0),
	m_NumberMarginWidth(0),
	m_OnActivateViewEntered(false),
	m_LineNumberMarginWidth(0),
	m_CharOverhang(0),
	m_ShownFileVersion(ShownAllText),
	m_DrawnSelEnd(0, 0)
{
	// init font size, to avoid zero divide
	m_FontMetric.tmAveCharWidth = 1;
	m_FontMetric.tmExternalLeading = 1;
	m_FontMetric.tmHeight = 1;
	m_ShowLineNumbers = GetApp()->m_bShowLineNumbers;
}

CDiffFileView::~CDiffFileView()
{
}


BEGIN_MESSAGE_MAP(CDiffFileView, CView)
	//{{AFX_MSG_MAP(CDiffFileView)
	ON_COMMAND(ID_WINDOW_CLOSE_DIFF, OnWindowCloseDiff)
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_KILLFOCUS()
	ON_WM_CAPTURECHANGED()
	ON_COMMAND(ID_EDIT_GOTONEXTDIFF, OnEditGotonextdiff)
	ON_COMMAND(ID_EDIT_GOTOPREVDIFF, OnEditGotoprevdiff)
	ON_COMMAND(ID_VIEW_SHOW_LINE_NUMBERS, OnViewShowLineNumbers)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_LINE_NUMBERS, OnUpdateViewShowLineNumbers)
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_EDIT_FIND_NEXT, OnEditFindNext)
	ON_COMMAND(ID_EDIT_FIND_PREV, OnEditFindPrev)
	ON_COMMAND(ID_EDIT_FIND_WORD_NEXT, OnEditFindWordNext)
	ON_COMMAND(ID_EDIT_FIND_WORD_PREV, OnEditFindWordPrev)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_GOTOLINE, OnEditGotoline)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILE_2_VERSION, OnUpdateViewFile2Version)
	ON_COMMAND(ID_VIEW_FILE_2_VERSION, OnViewFile2Version)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILE1_VERSION, OnUpdateViewFile1Version)
	ON_COMMAND(ID_VIEW_FILE1_VERSION, OnViewFile1Version)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiffFileView drawing
void CDiffFileView::DrawStringSections(CDC* pDC, CPoint point,
										const StringSection * pSection,
										int nSkipChars, int nVisibleChars, int nTabIndent, int SelBegin, int SelEnd)
{
	TCHAR buf[2048];
	CFilePairDoc* pDoc = GetDocument();
	CThisApp * pApp = GetApp();

	SelBegin -= nSkipChars;
	SelEnd -= nSkipChars;
	if (SelBegin < 0)
	{
		SelBegin = 0;
	}
	if (SelBegin > nVisibleChars)
	{
		SelBegin = nVisibleChars;
	}
	if (SelEnd < 0)
	{
		SelEnd = 0;
	}
	if (SelEnd > nVisibleChars)
	{
		SelEnd = nVisibleChars;
	}

	pDC->MoveTo(point);
	int ExpandedLinePos = 0;    // position with expanded tabs
	for (int nDrawnChars = 0; pSection != NULL && nDrawnChars < nVisibleChars; pSection = pSection->pNext)
	{
		if ((pSection->Attr & pSection->Whitespace)
			&& (pSection->Attr & pSection->Erased)
			&& pDoc->m_bIgnoreWhitespaces)
		{
			continue;   // don't show the section
		}
		LPCTSTR pText = pSection->pBegin;
		int Length = pSection->Length;
		for (int j = 0, k = 0; j < sizeof buf / sizeof buf[0] && k < Length; j++, ExpandedLinePos++)
		{
			if (pText[k] == '\t')
			{
				buf[j] = ' ';
				if ((ExpandedLinePos + 1) % nTabIndent)
				{
					continue;
				}
			}
			else
			{
				buf[j] = pText[k];
			}
			k++;
		}
		Length = j;
		pText = buf;
		// if the string is drawn started from some position, test for clipped part
		if (Length <= nSkipChars)
		{
			nSkipChars -= Length;
			continue;
		}
		Length -= nSkipChars;
		pText += nSkipChars;
		nSkipChars = 0;
		// limit number of drawn chars
		if (Length > nVisibleChars - nDrawnChars)
		{
			Length = nVisibleChars - nDrawnChars;
		}
		// choose font
		// text not in the merged file is drawn with alterative background
		// for accepted changes it is erased text,
		// for discarded changes it is inserted text
		while (Length > 0)
		{
			CFont * pFont;
			DWORD Color;
			DWORD BackgroundColor = pApp->m_TextBackgroundColor;
			int nCharsToDraw = Length;

			if ((pSection->Attr & pSection->Inserted)
				&& ! ((pSection->Attr & pSection->Whitespace)
					&& pDoc->m_bIgnoreWhitespaces))
			{
				Color = pApp->m_AddedTextColor;
				pFont = & pApp->m_AddedFont;
				if (pSection->IsAccepted())
				{
					BackgroundColor = pApp->m_AcceptedTextBackgroundColor;
				}
				else if (pSection->IsDeclined())
				{
					BackgroundColor = pApp->m_DiscardedTextBackgroundColor;
				}
			}
			else if (pSection->Attr & pSection->Erased)
			{
				Color = pApp->m_ErasedTextColor;
				pFont = & pApp->m_ErasedFont;
				if (pSection->IsAccepted())
				{
					BackgroundColor = pApp->m_DiscardedTextBackgroundColor;
				}
				else if (pSection->IsDeclined())
				{
					BackgroundColor = pApp->m_AcceptedTextBackgroundColor;
				}
			}
			else
			{
				Color = pApp->m_NormalTextColor;
				pFont = & pApp->m_NormalFont;
			}

			if (nDrawnChars < SelEnd
				&& nDrawnChars >= SelBegin)
			{
				if (nCharsToDraw > SelEnd - nDrawnChars)
				{
					nCharsToDraw = SelEnd - nDrawnChars;
				}
				Color = pApp->m_SelectedTextColor;
				BackgroundColor = 0x000000;
			}
			else if (nDrawnChars < SelBegin
					&& nDrawnChars + nCharsToDraw > SelBegin)
			{
				nCharsToDraw = SelBegin - nDrawnChars;
			}

			pDC->SetBkColor(BackgroundColor);
			pDC->SetTextColor(Color);
			pDC->SelectObject(pFont);
			// text is drawn from the current position
			pDC->TextOut(0, 0, pText, nCharsToDraw);
			nDrawnChars += nCharsToDraw;
			pText += nCharsToDraw;
			Length -= nCharsToDraw;
		}
	}

	int nBeforeSelection = SelBegin - nDrawnChars;
	if (nBeforeSelection > 0)
	{
		for (int i = 0; i < nBeforeSelection; i++)
		{
			buf[i] = ' ';
		}
		buf[nBeforeSelection] = 0;
		pDC->SetBkColor(pApp->m_TextBackgroundColor);
		pDC->SetTextColor(pApp->m_NormalTextColor);
		pDC->SelectObject(& pApp->m_NormalFont);
		// text is drawn from the current position
		pDC->TextOut(0, 0, buf, nBeforeSelection);
		nDrawnChars += nBeforeSelection;
	}

	int nMoreSelection = SelEnd - nDrawnChars;
	if (nMoreSelection > 0)
	{
		for (int i = 0; i < nMoreSelection; i++)
		{
			buf[i] = ' ';
		}
		buf[nMoreSelection] = 0;
		pDC->SetBkColor(0x000000);
		pDC->SetTextColor(pApp->m_SelectedTextColor);
		pDC->SelectObject(& pApp->m_NormalFont);
		// text is drawn from the current position
		pDC->TextOut(0, 0, buf, nMoreSelection);
	}
}

void CDiffFileView::OnDraw(CDC* pDC)
{
	CFilePairDoc* pDoc = GetDocument();
	CThisApp * pApp = GetApp();
	if (NULL == pDoc)
	{
		return;
	}
	FilePair * pFilePair = pDoc->GetFilePair();
	if (NULL == pFilePair)
	{
		return;
	}

	CFont * pOldFont = pDC->SelectObject( & pApp->m_NormalFont);

	//TEXTMETRIC tm;
	//pDC->GetTextMetrics( & tm);

	RECT cr;
	GetClientRect( & cr);
	RECT ur;    // update rect
	if (pDC->IsKindOf(RUNTIME_CLASS(CPaintDC)))
	{
		CPaintDC* pPaintDC = (CPaintDC*)pDC;
		if (pPaintDC->m_ps.fErase)
		{
			//EraseBkgnd(pPaintDC);
		}
		ur = pPaintDC->m_ps.rcPaint;
	}
	else
	{
		pDC->GetClipBox( & ur);
	}

	//pDC->SetTextAlign
	int PosX = 0;
	if (m_ShowLineNumbers)
	{
		CGdiObject * pOldPen = pDC->SelectStockObject(BLACK_PEN);
		pDC->MoveTo(m_LineNumberMarginWidth - 1, ur.bottom);
		pDC->LineTo(m_LineNumberMarginWidth - 1, ur.top);
		pDC->SelectObject(pOldPen);
		PosX = m_LineNumberMarginWidth;
	}

	int nLineHeight = LineHeight();

	int nTabIndent = GetApp()->m_TabIndent;
	int nCharsInView = CharsInView() + 1;

	pDC->SetTextAlign(pDC->GetTextAlign() | TA_UPDATECP);

	for (int nLine = m_FirstLineSeen, PosY = cr.top; PosY < cr.bottom; nLine++, PosY += nLineHeight)
	{
		TextPos SelBegin, SelEnd;
		if (pDoc->m_CaretPos < pDoc->m_SelectionAnchor)
		{
			SelBegin = pDoc->m_CaretPos;
			SelEnd = pDoc->m_SelectionAnchor;
		}
		else
		{
			SelBegin = pDoc->m_SelectionAnchor;
			SelEnd = pDoc->m_CaretPos;
		}
		int nSelBegin = 0;
		int nSelEnd = 0;

		if (SelEnd.line == SelBegin.line)
		{
			if (SelBegin.line == nLine)
			{
				if (SelBegin.pos < SelEnd.pos)
				{
					nSelBegin = SelBegin.pos;
					nSelEnd = SelEnd.pos;
				}
				else if (SelBegin.pos > SelEnd.pos)
				{
					nSelBegin = SelEnd.pos;
					nSelEnd = SelBegin.pos;
				}
			}
		}
		else
		{
			if (SelBegin.line == nLine)
			{
				nSelBegin = SelBegin.pos;
				nSelEnd = 2048;
			}
			else if (SelBegin.line < nLine)
			{
				if (SelEnd.line == nLine)
				{
					nSelEnd = SelEnd.pos;
				}
				else if (SelEnd.line > nLine)
				{
					nSelEnd = 2048;
				}
			}
		}

		const LinePair * pPair = NULL;
		StringSection Section;
		Section.Attr = Section.Identical;
		Section.pNext = NULL;
		StringSection * pSection = NULL;
		if (nLine >= pFilePair->m_LinePairs.GetSize())
		{
			break;
		}
		pPair = pFilePair->m_LinePairs[nLine];
		ASSERT(NULL != pPair);
		if (NULL != pPair)
		{
			pSection = pPair->pFirstSection;
			// draw line number
			if (m_ShowLineNumbers)
			{
				CString s;
				DWORD TextColor = pApp->m_NormalTextColor;
				if (NULL == pPair->pFirstLine)
				{
					s.Format("%d", pPair->pSecondLine->GetLineNumber() + 1);
					TextColor = pApp->m_AddedTextColor;
				}
				else if (NULL == pPair->pSecondLine)
				{
					s.Format("(%d)", pPair->pFirstLine->GetLineNumber() + 1);
					TextColor = pApp->m_ErasedTextColor;
				}
				else
				{
					s.Format("%d", pPair->pSecondLine->GetLineNumber() + 1);
				}
				pDC->MoveTo(m_LineNumberMarginWidth - 1, PosY);
				pDC->SetTextAlign(TA_RIGHT | TA_TOP | TA_UPDATECP);
				pDC->SetTextColor(TextColor);
				pDC->SetBkColor(pApp->m_TextBackgroundColor);
				pDC->SelectObject( & pApp->m_NormalFont);
				pDC->TextOut(0, 0, s);
				pDC->SetTextAlign(TA_LEFT | TA_TOP |TA_UPDATECP);

			}
		}
		else
		{
			Section.pBegin = NULL;
			Section.Length = 0;
		}
		DrawStringSections(pDC, CPoint(PosX, PosY),
							pSection, m_FirstPosSeen, nCharsInView, nTabIndent,
							nSelBegin, nSelEnd);
	}

	pDC->SelectObject(pOldFont);
}

/////////////////////////////////////////////////////////////////////////////
// CDiffFileView diagnostics

#ifdef _DEBUG
void CDiffFileView::AssertValid() const
{
	CView::AssertValid();
}

void CDiffFileView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
CFilePairDoc* CDiffFileView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFilePairDoc)));
	return (CFilePairDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDiffFileView message handlers

void CDiffFileView::OnWindowCloseDiff()
{
	CChildFrame * pFrame = (CChildFrame *)GetParentFrame();
	ASSERT_VALID(pFrame);
	pFrame->OnClose();
	return;

	// and close it
	GetDocument()->PreCloseFrame(pFrame);
	pFrame->DestroyWindow();
}

void CDiffFileView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	OnMetricsChange();
}

void CDiffFileView::OnDestroy()
{
	CView::OnDestroy();
}

BOOL CDiffFileView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_VSCROLL | WS_HSCROLL;
	return CView::PreCreateWindow(cs);
}

void CDiffFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	bool ShiftPressed = (0 != (0x8000 & GetKeyState(VK_SHIFT)));
	bool CtrlPressed = (0 != (0x8000 & GetKeyState(VK_CONTROL)));
	int SelectionFlags = SetPositionMakeVisible;
	if ( ! ShiftPressed)
	{
		SelectionFlags |= SetPositionCancelSelection;
	}
	int nLinesInView = LinesInView();

	switch (nChar)
	{
	case VK_DOWN:
		TRACE("VK_DOWN\n");
		if (CtrlPressed)
		{
		}
		else
		{
			MoveCaretBy(0, 1, SelectionFlags);
		}
		break;

	case VK_UP:
		TRACE("VK_UP\n");
		if (CtrlPressed)
		{
		}
		else
		{
			MoveCaretBy(0, -1, SelectionFlags);
		}
		break;

	case VK_LEFT:
		TRACE("VK_LEFT\n");
		if (CtrlPressed)
		{
			// move to one word left
			GetDocument()->CaretLeftToWord(SelectionFlags);
			MakeCaretVisible();
		}
		else
		{
			MoveCaretBy(-1, 0, SelectionFlags);
		}
		break;

	case VK_RIGHT:
		TRACE("VK_RIGHT\n");
		if (CtrlPressed)
		{
			// move to one word right
			GetDocument()->CaretRightToWord(SelectionFlags);
			MakeCaretVisible();
		}
		else
		{
			MoveCaretBy(1, 0, SelectionFlags);
		}
		break;

	case VK_HOME:
		TRACE("VK_HOME\n");
		if (CtrlPressed)
		{
			SetCaretPosition(0, 0, SelectionFlags);
		}
		else
		{
			CaretToHome(SelectionFlags);
		}
		break;

	case VK_END:
		TRACE("VK_END\n");
		if (CtrlPressed)
		{
			SetCaretPosition(0, GetDocument()->GetTotalLines(), SelectionFlags);
		}
		else
		{
			CaretToEnd(SelectionFlags);
		}
		break;

	case VK_PRIOR:
		TRACE("VK_PRIOR\n");
		// do VScroll and move cursor, to keep the cursor at the same line
		DoVScroll(-(nLinesInView - 1));
		MoveCaretBy(0, -(nLinesInView - 1), SelectionFlags);
		break;

	case VK_NEXT:
		TRACE("VK_NEXT\n");
		// do VScroll and move cursor, to keep the cursor at the same line
		DoVScroll(nLinesInView - 1);
		MoveCaretBy(0, nLinesInView - 1, SelectionFlags);
		break;
	}
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDiffFileView::DoHScroll(int nCharsToScroll)
{
	// > 0 - scroll left (to see chars toward end),
	// < 0 - scroll right (to see chars to the begin of the line)
	HScrollToThePos(nCharsToScroll + m_FirstPosSeen);
}

void CDiffFileView::HScrollToThePos(int nPos)
{
	int nCharsInView = CharsInView();

	if (nPos < 0)
	{
		nPos = 0;
	}

	if (nPos > 2048 - nCharsInView)
	{
		nPos = 2048 - nCharsInView;
	}

	if (nPos == m_FirstPosSeen)
	{
		return;
	}
	// need to scroll the view

	CRect cr;

	GetClientRect( & cr);

	cr.left = m_LineNumberMarginWidth;
	if (cr.left < cr.right)
	{
		int ndx = (nPos - m_FirstPosSeen) * CharWidth();
		ScrollWindowEx( -ndx, 0, & cr, & cr, NULL, NULL,
						SW_INVALIDATE | SW_ERASE);
	}
	m_FirstPosSeen = nPos;

	UpdateHScrollBar();
	CreateAndShowCaret();
}

void CDiffFileView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it

	// ignore scroll bar msgs from other controls
	if (pScrollBar != GetScrollBarCtrl(SB_HORZ))
		return;

	int nCharsInView = CharsInView();
	switch (nSBCode)
	{
	case SB_TOP:
		TRACE("OnVScroll SB_TOP, nPos=%d\n", nPos);
		HScrollToThePos(0);
		break;

	case SB_BOTTOM:
		TRACE("OnVScroll SB_BOTTOM, nPos=%d\n", nPos);
		HScrollToThePos(2048);
		break;

	case SB_LINEUP:
		TRACE("OnVScroll SB_LINEUP, nPos=%d\n", nPos);
		DoHScroll( -1);
		break;

	case SB_LINEDOWN:
		TRACE("OnVScroll SB_LINEDOWN, nPos=%d\n", nPos);
		DoHScroll( +1);
		break;

	case SB_PAGEUP:
		TRACE("OnVScroll SB_PAGEUP, nPos=%d\n", nPos);
		DoHScroll( - (nCharsInView - 1));
		break;

	case SB_PAGEDOWN:
		TRACE("OnVScroll SB_PAGEDOWN, nPos=%d\n", nPos);
		DoHScroll( + nCharsInView - 1);
		break;

	case SB_THUMBTRACK:
		TRACE("OnVScroll SB_THUMBTRACK, nPos=%d\n", nPos);
		HScrollToThePos(nPos);
		break;
	default:
		break;
	}
}

	// > 0 - scroll up (to see lines toward end),
	// < 0 - scroll down (to see lines to the begin of the file)
void CDiffFileView::DoVScroll(int nLinesToScroll)
{
	VScrollToTheLine(m_FirstLineSeen + nLinesToScroll);
}

void CDiffFileView::VScrollToTheLine(int nLine)
{
	int nLinesInView = LinesInView();

	if (nLine < 0)
	{
		nLine = 0;
	}

	if (nLine > GetDocument()->GetTotalLines() - (nLinesInView - 1))
	{
		nLine = GetDocument()->GetTotalLines() - (nLinesInView - 1);
	}

	if (nLine == m_FirstLineSeen)
	{
		return;
	}
	// need to scroll the view
	int ndy = (nLine - m_FirstLineSeen) * LineHeight();
	m_FirstLineSeen = nLine;
	ScrollWindowEx(0, -ndy, NULL, NULL, NULL, NULL,
					SW_INVALIDATE | SW_ERASE);

	if (m_LButtonDown)
	{
		SetTimer(1, 20, NULL);
	}
	UpdateVScrollBar();
	CreateAndShowCaret();
}

void CDiffFileView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it

	// ignore scroll bar msgs from other controls
	if (pScrollBar != GetScrollBarCtrl(SB_VERT))
		return;

	int nLinesInView = LinesInView();
	switch (nSBCode)
	{
	case SB_TOP:
		TRACE("OnVScroll SB_TOP, nPos=%d\n", nPos);
		VScrollToTheLine(0);
		break;

	case SB_BOTTOM:
		TRACE("OnVScroll SB_BOTTOM, nPos=%d\n", nPos);
		VScrollToTheLine(GetDocument()->GetTotalLines() - nLinesInView);
		break;

	case SB_LINEUP:
		TRACE("OnVScroll SB_LINEUP, nPos=%d\n", nPos);
		DoVScroll( -1);
		break;

	case SB_LINEDOWN:
		TRACE("OnVScroll SB_LINEDOWN, nPos=%d\n", nPos);
		DoVScroll( +1);
		break;

	case SB_PAGEUP:
		TRACE("OnVScroll SB_PAGEUP, nPos=%d\n", nPos);
		DoVScroll( - (nLinesInView - 1));
		break;

	case SB_PAGEDOWN:
		TRACE("OnVScroll SB_PAGEDOWN, nPos=%d\n", nPos);
		DoVScroll( + nLinesInView - 1);
		break;

	case SB_THUMBTRACK:
		TRACE("OnVScroll SB_THUMBTRACK, nPos=%d\n", nPos);
		VScrollToTheLine(nPos);
		break;
	default:
		break;
	}
}

void CDiffFileView::UpdateVScrollBar()
{
	if (0 == LineHeight())
	{
		return;
	}
	int nLinesInView = LinesInView();

	SCROLLINFO sci;
	sci.cbSize = sizeof sci;
	sci.nMin = 0;
	sci.nMax = GetDocument()->GetTotalLines();
	sci.nPage = nLinesInView;
	sci.nPos = m_FirstLineSeen;
	sci.nTrackPos = 0;
	sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

	SetScrollInfo(SB_VERT, & sci);
}

void CDiffFileView::UpdateHScrollBar()
{
	if (0 == CharWidth())
	{
		return;
	}
	int nCharsInView = CharsInView();

	SCROLLINFO sci;
	sci.cbSize = sizeof sci;
	sci.nMin = 0;
	sci.nMax = 2048;
	sci.nPage = nCharsInView;
	sci.nPos = m_FirstPosSeen;
	sci.nTrackPos = 0;
	sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

	SetScrollInfo(SB_HORZ, & sci);
}

void CDiffFileView::MakePositionVisible(int line, int pos)
{
	BringPositionToBounds(TextPos(line, pos), m_VisibleRect, m_VisibleRect);
}

void CDiffFileView::MakePositionCentered(int line, int pos)
{
	BringPositionToBounds(TextPos(line, pos), m_VisibleRect, m_PreferredRect);
}

void CDiffFileView::InvalidateRange(TextPos begin, TextPos end)
{
	ASSERT(end >= begin);
	CRect r;
	if (0) TRACE("InvalidateRange((%d, %d), (%d, %d))\n", begin, end);
	int nLinesInView = LinesInView();
	int nCharsInView = CharsInView();
	if (begin == end
		|| end.line < m_FirstLineSeen
		|| begin.line > m_FirstLineSeen + nLinesInView + 1)
	{
		return;
	}
	begin.pos -= m_FirstPosSeen;
	begin.line -= m_FirstLineSeen;

	end.pos -= m_FirstPosSeen;
	end.line -= m_FirstLineSeen;

	if (begin.line == end.line)
	{
		r.top = begin.line * LineHeight();
		r.bottom = r.top + LineHeight();
		if (end.pos <= 0 || begin.pos > nCharsInView)
		{
			return;
		}
		if (begin.pos < 0)
		{
			begin.pos = 0;
		}
		if (end.pos > nCharsInView)
		{
			end.pos = nCharsInView;
		}
		r.left = begin.pos * CharWidth() + m_LineNumberMarginWidth;
		r.right = end.pos * CharWidth() + m_LineNumberMarginWidth + m_FontMetric.tmOverhang;
		InvalidateRect( & r);
	}
	else
	{
		if (begin.line >= 0 && begin.pos <= nCharsInView)
		{
			if (begin.pos < 0)
			{
				begin.pos = 0;
			}
			r.top = begin.line * LineHeight();
			r.bottom = r.top + LineHeight();
			r.left = begin.pos * CharWidth() + m_LineNumberMarginWidth;
			r.right = (nCharsInView + 1) * CharWidth() + m_LineNumberMarginWidth + m_FontMetric.tmOverhang;
			InvalidateRect( & r);
		}
		if (end.line <= nLinesInView + 2 && end.pos > 0)
		{
			if (end.pos > nCharsInView + 1)
			{
				end.pos = nCharsInView + 1;
			}
			r.top = end.line * LineHeight();
			r.bottom = r.top + LineHeight();
			r.right = end.pos * CharWidth() + m_LineNumberMarginWidth + m_FontMetric.tmOverhang;
			r.left = m_LineNumberMarginWidth;
			InvalidateRect( & r);
		}
		if (end.line > nLinesInView + 2)
		{
			end.line = nLinesInView + 2;
		}
		if (end.line > begin.line + 1)
		{
			r.left = m_LineNumberMarginWidth;
			r.right = (nCharsInView + 1) * CharWidth() + m_LineNumberMarginWidth + m_FontMetric.tmOverhang;
			r.top = (begin.line + 1) * LineHeight();
			r.bottom = end.line * LineHeight();
			InvalidateRect( & r);
		}
	}
}

void CDiffFileView::SetCaretPosition(int pos, int line, int flags)
{
	CFilePairDoc * pDoc = GetDocument();
	pDoc->SetCaretPosition(pos, line, flags);
	if (flags & SetPositionMakeVisible)
	{
		MakeCaretVisible();
	}
	else if (flags & SetPositionMakeCentered)
	{
		MakeCaretCentered();
	}
}

void CDiffFileView::CreateAndShowCaret()
{
	CFilePairDoc * pDoc = GetDocument();
	if (this != GetFocus())
	{
		TRACE("CDiffFileView::CreateAndShowCaret - No Focus\n");
		return;
	}
	if (0 == GetDocument()->GetTotalLines())
	{
		TRACE("CDiffFileView::CreateAndShowCaret - 0 == GetDocument()->GetTotalLines()\n");
		return;
	}
	CPoint p((pDoc->m_CaretPos.pos - m_FirstPosSeen) * CharWidth() + m_LineNumberMarginWidth,
			(pDoc->m_CaretPos.line - m_FirstLineSeen) * LineHeight());
	if (pDoc->m_CaretPos.pos >= m_FirstPosSeen
		&& pDoc->m_CaretPos.line >= m_FirstLineSeen
		&& pDoc->m_CaretPos.line <= m_FirstLineSeen + LinesInView() + 1)
	{
		CreateSolidCaret(2, LineHeight());
		if (0) TRACE("CDiffFileView::CreateAndShowCaret %d %d\n", p.x, p.y);
		SetCaretPos(p);
		ShowCaret();
	}
	else
	{
		DestroyCaret();
	}
}

void CDiffFileView::OnSetFocus(CWnd* pOldWnd)
{
	CView::OnSetFocus(pOldWnd);

	CreateAndShowCaret();
}

void CDiffFileView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CView::OnLButtonDown(nFlags, point);
	m_LButtonDown = true;
	int flags = SetPositionMakeVisible;
	// if the left margin is clicked, the whole line is selected
	point.x -= m_LineNumberMarginWidth;
	int nLine = point.y / LineHeight() + m_FirstLineSeen;

	if (point.x < 0)
	{
		if (0 == (nFlags & MK_SHIFT))
		{
			SetCaretPosition(0, nLine);
		}
		if (GetDocument()->m_SelectionAnchor.line <= nLine)
		{
			SetCaretPosition(0, nLine + 1, SetPositionMakeVisible);
		}
		else
		{
			SetCaretPosition(0, nLine, SetPositionMakeVisible);
		}
	}
	else
	{
		TextPos ClickPos(nLine, m_FirstPosSeen + (point.x + CharWidth() / 2) / CharWidth());
		if (0 == (nFlags & MK_SHIFT))
		{
			flags |= SetPositionCancelSelection;
		}
		if (nFlags & MK_CONTROL)
		{
			TextPos Begin, End;
			CFilePairDoc * pDoc = GetDocument();
			if (nFlags & MK_SHIFT)
			{
				TextPos AnchorBegin = pDoc->m_SelectionAnchor;
				TextPos AnchorEnd = pDoc->m_SelectionAnchor;
				pDoc->GetWordOnPos(pDoc->m_SelectionAnchor, AnchorBegin, AnchorEnd);
				Begin = ClickPos;
				End = ClickPos;
				pDoc->GetWordOnPos(ClickPos, Begin, End);
				if (End < AnchorEnd)
				{
					End = AnchorEnd;
				}
				if (Begin > AnchorBegin)
				{
					Begin = AnchorBegin;
				}
				pDoc->SetSelection(End, Begin, 0);
				MakeCaretVisible();
				return;
			}
			else
			{
				if (pDoc->GetWordOnPos(ClickPos, Begin, End))
				{
					// select a word
					pDoc->SetSelection(End, Begin);
					MakeCaretVisible();
					return;
				}
			}
		}
		SetCaretPosition(ClickPos.pos, ClickPos.line, flags);
	}
}

void CDiffFileView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_LButtonDown = false;
	if (m_TrackingSelection)
	{
		ReleaseCapture();
	}
	CView::OnLButtonUp(nFlags, point);
}

void CDiffFileView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_LButtonDown && ! m_TrackingSelection)
	{
		m_TrackingSelection = true;
		SetCapture();
	}
	if (m_TrackingSelection)
	{
		point.x -= m_LineNumberMarginWidth;
		int nLine = point.y / LineHeight() + m_FirstLineSeen;

		if (point.x < 0)
		{
			if (GetDocument()->m_SelectionAnchor.line <= nLine)
			{
				SetCaretPosition(0, nLine + 1, SetPositionMakeVisible);
			}
			else
			{
				SetCaretPosition(0, nLine, SetPositionMakeVisible);
			}
		}
		else
		{
			TextPos MousePos(nLine, m_FirstPosSeen + (point.x + CharWidth() / 2) / CharWidth());
			if (nFlags & MK_CONTROL)
			{
				CFilePairDoc * pDoc = GetDocument();
				TextPos AnchorBegin = pDoc->m_SelectionAnchor;
				TextPos AnchorEnd = pDoc->m_SelectionAnchor;
				pDoc->GetWordOnPos(pDoc->m_SelectionAnchor, AnchorBegin, AnchorEnd);
				TextPos Begin = MousePos;
				TextPos End = MousePos;
				pDoc->GetWordOnPos(MousePos, Begin, End);
				if (End < AnchorEnd)
				{
					End = AnchorEnd;
				}
				if (Begin > AnchorBegin)
				{
					Begin = AnchorBegin;
				}
				pDoc->SetSelection(End, Begin, 0);
				MakeCaretVisible();
			}
			else
			{
				SetCaretPosition(MousePos.pos, nLine, SetPositionMakeVisible);
			}
		}
	}
	CView::OnMouseMove(nFlags, point);
}

BOOL CDiffFileView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// todo: accumulate the delta until WHEEL_DELTA is reached
	if (WHEEL_DELTA == zDelta)
	{
		DoVScroll( -3);
	}
	else if ( - WHEEL_DELTA == zDelta)
	{
		DoVScroll( +3);
	}

	return TRUE;
}

void CDiffFileView::OnSize(UINT nType, int cx, int cy)
{
	TRACE("CDiffFileView::OnSize\n");
	CView::OnSize(nType, cx, cy);

	m_VisibleRect.bottom = cy / LineHeight() - 1;
	m_PreferredRect.bottom = m_VisibleRect.bottom / 4;
	m_PreferredRect.top = m_VisibleRect.bottom - m_VisibleRect.bottom / 4;

	m_VisibleRect.right =  cx / CharWidth();
	m_PreferredRect.left = m_VisibleRect.right / 3;
	m_PreferredRect.right = m_VisibleRect.right - m_VisibleRect.right / 3;

	UpdateVScrollBar();
	UpdateHScrollBar();
}

void CDiffFileView::OnMetricsChange()
{
	TRACE("CDiffFileView::OnMetricsChange\n");

	// create font
	CThisApp * pApp = GetApp();

	CWindowDC wdc(this);
	CFont * pFont =  & pApp->m_NormalFont;

	CFont font;
	if (pApp->m_NormalLogFont.lfItalic
		|| pApp->m_AddedLogFont.lfItalic
		|| pApp->m_ErasedLogFont.lfItalic
		|| pApp->m_NormalLogFont.lfWeight > FW_NORMAL
		|| pApp->m_AddedLogFont.lfWeight > FW_NORMAL
		|| pApp->m_ErasedLogFont.lfWeight > FW_NORMAL)
	{
		LOGFONT lf = pApp->m_NormalLogFont;
		lf.lfItalic = TRUE;
		lf.lfWeight = FW_BLACK;
		font.CreateFontIndirect( & lf);

		pFont = & font;
	}

	CFont * pOldFont = wdc.SelectObject(pFont);
	wdc.GetTextMetrics( & m_FontMetric);
	wdc.SelectObject(pOldFont);

	CRect cr;
	GetClientRect( & cr);

	if (m_ShowLineNumbers)
	{
		int nNumChars = 6;
		if (GetDocument()->GetTotalLines() > 9999)
		{
			nNumChars = 8;
		}
		m_LineNumberMarginWidth = CharWidth() * nNumChars + 1;
	}
	else
	{
		m_LineNumberMarginWidth = 0;
	}

	m_VisibleRect.bottom = cr.Height() / LineHeight() - 1;
	m_PreferredRect.bottom = m_VisibleRect.bottom / 4;
	m_PreferredRect.top = m_PreferredRect.bottom; //m_VisibleRect.bottom - m_VisibleRect.bottom / 4;

	m_VisibleRect.right =  (cr.Width() - m_LineNumberMarginWidth) / CharWidth();
	if (m_VisibleRect.right < 0)
	{
		m_VisibleRect.right = 0;
	}

	m_PreferredRect.left = m_VisibleRect.right / 3;
	m_PreferredRect.right = m_VisibleRect.right - m_VisibleRect.right / 3;

	UpdateVScrollBar();
	UpdateHScrollBar();
	CreateAndShowCaret();
}

BOOL CDiffFileView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (HTCLIENT == nHitTest)
	{
		CPoint p;

		GetCursorPos( & p);
		ScreenToClient( & p);

		if (p.x >= m_LineNumberMarginWidth)
		{
			SetCursor(GetApp()->LoadCursor(IDC_CURSOR_BEAM));
		}
		else
		{
			SetCursor(GetApp()->LoadCursor(IDC_CURSOR_LEFT_ARROW));
		}
		return TRUE;
	}

	return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CDiffFileView::OnKillFocus(CWnd* pNewWnd)
{
	DestroyCaret();
	m_LButtonDown = false;
	CView::OnKillFocus(pNewWnd);

}

void CDiffFileView::OnCaptureChanged(CWnd *pWnd)
{
	if (pWnd != this)
	{
		m_TrackingSelection = false;
		m_LButtonDown = false;
	}

	CView::OnCaptureChanged(pWnd);
}

void CDiffFileView::OnEditGotonextdiff()
{
	CFilePairDoc * pDoc = GetDocument();
	pDoc->OnEditGotonextdiff();
	MakeCaretCentered();
}

void CDiffFileView::OnEditGotoprevdiff()
{
	CFilePairDoc * pDoc = GetDocument();
	pDoc->OnEditGotoprevdiff();
	MakeCaretCentered();
}

void CDiffFileView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CFilePairDoc * pDoc = GetDocument();
	if (lHint == CFilePairDoc::CaretPositionChanged)
	{
		// invalidate selection
		// invalidate where the selection changed
		// first we make sorted array of old and new selection boundaries
		// then we invalidate between 0, 1 and 2, 3
		if (0) TRACE("Caret: (%d, %d), sel anchor: (%d, %d), prev: (%d, %d), (%d, %d)\n",
					pDoc->m_CaretPos, pDoc->m_SelectionAnchor,
					m_DrawnSelBegin, m_DrawnSelEnd);
		TextPos Sel[4];
		if (pDoc->m_CaretPos < pDoc->m_SelectionAnchor)
		{
			Sel[0] = pDoc->m_CaretPos;
			Sel[3] = pDoc->m_SelectionAnchor;
		}
		else
		{
			Sel[0]= pDoc->m_SelectionAnchor;
			Sel[3] = pDoc->m_CaretPos;
		}
		if (m_DrawnSelBegin >= Sel[0])
		{
			Sel[1] = m_DrawnSelBegin;
			m_DrawnSelBegin = Sel[0];
		}
		else
		{
			Sel[1] = Sel[0];
			Sel[0] = m_DrawnSelBegin;
			m_DrawnSelBegin = Sel[1];
		}

		if (m_DrawnSelEnd <= Sel[3])
		{
			Sel[2] = m_DrawnSelEnd;
			m_DrawnSelEnd = Sel[3];
		}
		else
		{
			Sel[2] = Sel[3];
			Sel[3] = m_DrawnSelEnd;
			m_DrawnSelEnd = Sel[2];
		}
		if (Sel[1] > Sel[2])
		{
			TextPos tmp = Sel[1];
			Sel[1] = Sel[2];
			Sel[2] = tmp;
		}
		// array is sorted
		InvalidateRange(Sel[0], Sel[1]);
		InvalidateRange(Sel[2], Sel[3]);
	}
	else if (lHint == CFilePairDoc::InvalidateRange)
	{
		InvalidatedRange * pRange = dynamic_cast<InvalidatedRange *>(pHint);
		if (NULL != pRange)
		{
			TextPos begin, end;
			if (pRange->begin <= pRange->end)
			{
				begin = pRange->begin;
				end = pRange->end;
			}
			else
			{
				end = pRange->begin;
				begin = pRange->end;
			}

			InvalidateRange(begin, end);
		}
	}
	else if (lHint == CFilePairDoc::FileLoaded)
	{
		UpdateHScrollBar();
		UpdateVScrollBar();
		CreateAndShowCaret();
	}
	else if (lHint == CFilePairDoc::MetricsChanged)
	{
		OnMetricsChange();
		Invalidate(TRUE);
		UpdateHScrollBar();
		UpdateVScrollBar();
		CreateAndShowCaret();
	}
	else
	{
		Invalidate(TRUE);
		UpdateHScrollBar();
		UpdateVScrollBar();
		CreateAndShowCaret();
	}
}

void CDiffFileView::BringPositionToBounds(TextPos textpos, const CRect & AllowedBounds, const CRect & BringToBounds)
{
	// if caret position is inside bounds, it doesn't need to change
	int CaretLine = textpos.line - m_FirstLineSeen;
	int CaretPos =  textpos.pos - m_FirstPosSeen;
	if (CaretLine < AllowedBounds.top)
	{
		// bring to BringToBounds.top
		DoVScroll(CaretLine - BringToBounds.top);
	}
	else if (CaretLine > AllowedBounds.bottom)
	{
		DoVScroll(CaretLine - BringToBounds.bottom);
	}
	if (CaretPos < AllowedBounds.left)
	{
		DoHScroll(CaretPos - BringToBounds.left);
	}
	else if(CaretPos > AllowedBounds.right)
	{
		DoHScroll(CaretPos - BringToBounds.right);
	}
	CreateAndShowCaret();
}

void CDiffFileView::CaretToHome(int flags)
{
	GetDocument()->CaretToHome(flags);
	MakeCaretVisible();
}

void CDiffFileView::CaretToEnd(int flags)
{
	GetDocument()->CaretToEnd(flags);
	MakeCaretVisible();
}

void CDiffFileView::OnViewShowLineNumbers()
{
	m_ShowLineNumbers = ! m_ShowLineNumbers;
	GetApp()->m_bShowLineNumbers = m_ShowLineNumbers;

	OnUpdate(NULL, CFilePairDoc::MetricsChanged, NULL);
}

void CDiffFileView::OnUpdateViewShowLineNumbers(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_ShowLineNumbers);
}

void CDiffFileView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactivateView)
{
	CView::OnActivateView(bActivate, pActivateView, pDeactivateView);
	if (0) TRACE("bActivate=%d, this=%08X, pActivateView=%08X\n", bActivate, this, pActivateView);
	if ( ! AfxGetMainWnd()->IsWindowEnabled())
	{
		// modal dialog is running
		return;
	}
	if (m_OnActivateViewEntered)
	{
		return;
	}
	m_OnActivateViewEntered = true;
	if (GetApp()->m_AutoReloadChangedFiles)
	{
		if (bActivate && this == pActivateView)
		{
			GetDocument()->OnViewRefresh();
		}
	}
	m_OnActivateViewEntered = false;
}

void CDiffFileView::OnEditFind()
{
	CThisApp * pApp = GetApp();
	CMyFindDialog dlg;
	dlg.m_sFindCombo = pApp->m_FindString;
	dlg.m_bCaseSensitive = pApp->m_bCaseSensitive;

	if (IDOK == dlg.DoModal())
	{
		pApp->m_FindString = dlg.m_sFindCombo;
		pApp->m_bCaseSensitive = dlg.m_bCaseSensitive;
		if (GetDocument()->FindTextString(pApp->m_FindString, false, pApp->m_bCaseSensitive))
		{
			MakeCaretCentered();
		}
	}
}

void CDiffFileView::OnEditFindNext()
{
	CThisApp * pApp = GetApp();
	if (GetDocument()->FindTextString(pApp->m_FindString, false, pApp->m_bCaseSensitive))
	{
		MakeCaretCentered();
	}
}

void CDiffFileView::OnEditFindPrev()
{
	CThisApp * pApp = GetApp();
	if (GetDocument()->FindTextString(pApp->m_FindString, true, pApp->m_bCaseSensitive))
	{
		MakeCaretCentered();
	}
}

void CDiffFileView::OnEditFindWordNext()
{
	if (GetDocument()->FindWordOrSelection(false))
	{
		MakeCaretCentered();
	}
}

void CDiffFileView::OnEditFindWordPrev()
{
	if (GetDocument()->FindWordOrSelection(true))
	{
		MakeCaretCentered();
	}
}


void CDiffFileView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CView::OnLButtonDblClk(nFlags, point);
	// select a word
	TextPos Begin, End;
	CFilePairDoc * pDoc = GetDocument();
	point.x -= m_LineNumberMarginWidth;
	if (point.x < 0)
	{
		return;
	}
	TextPos ClickPos(point.y / LineHeight() + m_FirstLineSeen,
					m_FirstPosSeen + (point.x + CharWidth() / 2) / CharWidth());
	if (pDoc->GetWordOnPos(ClickPos, Begin, End))
	{
		pDoc->SetSelection(End, Begin);
		MakeCaretVisible();
	}
}

void CDiffFileView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// make sure window is active
	GetParentFrame()->ActivateFrame();

	CMenu menu;
	if (menu.LoadMenu(IDR_MENU_FILEDIFF_CONTEXT))
	{
		CMenu* pPopup = menu.GetSubMenu(0);
		if(pPopup != NULL)
		{
			pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
									point.x, point.y,
									AfxGetMainWnd()); // use main window for cmds
		}
	}

}

void CDiffFileView::OnEditGotoline()
{
	CGotoLineDialog dlg;
	dlg.m_LineNumber = GetDocument()->m_CaretPos.line;
	if (IDOK == dlg.DoModal())
	{
		SetCaretPosition(0, dlg.m_LineNumber - 1, SetPositionMakeCentered | SetPositionCancelSelection);
	}
}

void CDiffFileView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CPoint point1 = point;
	point1.x -= m_LineNumberMarginWidth;
	int flags = SetPositionMakeVisible | SetPositionCancelSelection;
	// if the left margin is clicked, the whole line is selected
	int nLine = point1.y / LineHeight() + m_FirstLineSeen;

	if (point1.x < 0)
	{
		SetCaretPosition(0, nLine, flags);
	}
	else
	{
		TextPos NewPos(nLine, m_FirstPosSeen + (point1.x + CharWidth() / 2) / CharWidth());
		if (GetDocument()->m_CaretPos <= GetDocument()->m_SelectionAnchor
			&& (NewPos < GetDocument()->m_CaretPos || NewPos > GetDocument()->m_SelectionAnchor)
			|| GetDocument()->m_CaretPos > GetDocument()->m_SelectionAnchor
			&& (NewPos > GetDocument()->m_CaretPos || NewPos < GetDocument()->m_SelectionAnchor))
		{
			SetCaretPosition(NewPos.pos, nLine, flags);
		}
	}

	CView::OnRButtonDown(nFlags, point);
}

void CDiffFileView::OnEditSelectAll()
{
	GetDocument()->SetSelection(TextPos(0, 0), TextPos(GetDocument()->GetTotalLines(), 0));
	CreateAndShowCaret();
}


void CDiffFileView::OnUpdateViewFile2Version(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here

}

void CDiffFileView::OnViewFile2Version()
{
	// enable if there are any differences in the line, and the line is not exclusively File1
}

void CDiffFileView::OnUpdateViewFile1Version(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here

}

void CDiffFileView::OnViewFile1Version()
{
	// TODO: Add your command handler code here

}


void CDiffFileView::OnTimer(UINT nIDEvent)
{
	if (1 == nIDEvent)
	{
		KillTimer(1);
		CPoint p;
		GetCursorPos( & p);
		ScreenToClient( & p);
		int flags = 0;
		if (0x8000 & GetKeyState(VK_SHIFT))
		{
			flags |= MK_SHIFT;
		}
		if (0x8000 & GetKeyState(VK_CONTROL))
		{
			flags |= MK_CONTROL;
		}
		OnMouseMove(flags, p);
	}
	CView::OnTimer(nIDEvent);
}
