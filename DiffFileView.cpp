// DiffFileView.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DiffFileView.h"
#include "GoToLineDialog.h"
#include "ChildFrm.h"

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
	m_WheelAccumulator(0),
	m_NumberOfPanes(1),
	m_PaneWithFocus(0),
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
	ON_COMMAND(ID_FILE_CANCEL, OnWindowCloseDiff)
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
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
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
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_COMMAND(IDC_VIEW_SIDE_BY_SIDE, OnViewSideBySide)
	ON_UPDATE_COMMAND_UI(IDC_VIEW_SIDE_BY_SIDE, OnUpdateViewSideBySide)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiffFileView drawing
void CDiffFileView::DrawStringSections(CDC* pDC, CPoint point,
										KListEntry<StringSection> const * SectionEntry,
										int nSkipChars, int nVisibleChars, int nTabIndent,
										int SelBegin, int SelEnd, int nFileSelect)
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
	StringSection * pSection = SectionEntry->First();

	for (int nDrawnChars = 0; SectionEntry->NotEnd(pSection) && nDrawnChars < nVisibleChars; pSection = pSection->Next())
	{
		if ((pSection->Attr & pSection->Erased)
			&& 2 == nFileSelect)
		{
			continue;
		}

		if ((pSection->Attr & pSection->Inserted)
			&& 1 == nFileSelect)
		{
			continue;
		}

		if ((pSection->Attr & pSection->Whitespace)
			&& (pSection->Attr & pSection->Erased)
			&& pDoc->m_bIgnoreWhitespaces)
		{
			continue;   // don't show the section
		}
		LPCTSTR pText = pSection->pBegin;
		int Length = pSection->Length;
		for (int j = 0, k = 0; j < countof(buf) && k < Length; j++, ExpandedLinePos++)
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
		if (0 == Length)
		{
			Length = 1;
			pText = _T(" ");
		}
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
				if (pSection->IsAccepted() || pSection->IsIncluded())
				{
					BackgroundColor = pApp->m_AcceptedTextBackgroundColor;
				}
				else if (pSection->IsDeclined() || pSection->IsDiscarded())
				{
					BackgroundColor = pApp->m_DiscardedTextBackgroundColor;
				}
			}
			else if (pSection->Attr & pSection->Erased)
			{
				Color = pApp->m_ErasedTextColor;
				pFont = & pApp->m_ErasedFont;
				if (pSection->IsAccepted() || pSection->IsDiscarded())
				{
					BackgroundColor = pApp->m_DiscardedTextBackgroundColor;
				}
				else if (pSection->IsDeclined() || pSection->IsIncluded())
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

	CRect cr;
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

	int nPaneWidth = cr.Width()
					- m_LineNumberMarginWidth * m_NumberOfPanes
					- (m_NumberOfPanes - 1);

	nPaneWidth = nPaneWidth / CharWidth() / m_NumberOfPanes * CharWidth()
				+ m_LineNumberMarginWidth + 1;

	CString s;

	for (int nPane = 0, nPaneBegin = 0;
		nPane < m_NumberOfPanes; nPane++, nPaneBegin += nPaneWidth)
	{
		int PosX = nPaneBegin;
		int nCharsInView = CharsInView();
		if (nPane == m_NumberOfPanes - 1)
		{
			nCharsInView += 2;
		}
		else
		{
			// draw separator line
			pDC->MoveTo(PosX + nPaneWidth - 1, ur.top);
			pDC->LineTo(PosX + nPaneWidth - 1, ur.bottom);
		}

		if (m_ShowLineNumbers)
		{
			CGdiObject * pOldPen = pDC->SelectStockObject(BLACK_PEN);

			PosX += m_LineNumberMarginWidth;
			pDC->MoveTo(PosX - 1, ur.top);
			pDC->LineTo(PosX - 1, ur.bottom);
			if (NULL != pFilePair->pFirstFile
				&& NULL != pFilePair->pSecondFile
				&& 1 == m_NumberOfPanes)
			{
				pDC->MoveTo(m_LineNumberMarginWidth / 2 - 1, ur.top);
				pDC->LineTo(m_LineNumberMarginWidth / 2 - 1, ur.bottom);
			}
			pDC->SelectObject(pOldPen);
		}

		int nLineHeight = LineHeight();

		int nTabIndent = GetApp()->m_TabIndent;

		pDC->SetTextAlign(pDC->GetTextAlign() | TA_UPDATECP);

		int nPaneToDraw = nPane + 1;

		if (1 == m_NumberOfPanes)
		{
			nPaneToDraw = 0;
		}

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

			if (m_PaneWithFocus == nPane)
			{
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
			}

			KListEntry<StringSection> EmptyList;
			KListEntry<StringSection> const * pSectionEntry = & EmptyList;

			if (nLine >= (int)pFilePair->m_LinePairs.size())
			{
				break;
			}

			const LinePair * pPair = pFilePair->m_LinePairs[nLine];
			ASSERT(NULL != pPair);

			if (NULL != pPair)
			{
				pSectionEntry = & pPair->StrSections;
				// draw line number
				if (m_ShowLineNumbers)
				{
					DWORD TextColor = pApp->m_NormalTextColor;

					pDC->SetTextAlign(TA_RIGHT | TA_TOP | TA_NOUPDATECP);
					pDC->SetBkColor(pApp->m_TextBackgroundColor);
					pDC->SelectObject( & pApp->m_NormalFont);

					if (NULL == pFilePair->pFirstFile
						|| NULL == pFilePair->pSecondFile)
					{
						s.Format(_T("%d"), pPair->pFirstLine->GetLineNumber() + 1);
						pDC->SetTextColor(TextColor);
						pDC->TextOut(m_LineNumberMarginWidth - 1, PosY, s);
					}
					else
					{
						if (NULL != pPair->pFirstLine
							&& (m_NumberOfPanes == 1 || nPane == 0))
						{
							s.Format(_T("%d"), pPair->pFirstLine->GetLineNumber() + 1);
							if (NULL == pPair->pSecondLine)
							{
								TextColor = pApp->m_ErasedTextColor;
							}
							pDC->SetTextColor(TextColor);

							int pos = PosX - 1;
							if (m_NumberOfPanes == 1)
							{
								pos = m_LineNumberMarginWidth / 2 - 1;
							}
							pDC->TextOut(pos, PosY, s);
						}
						if (NULL != pPair->pSecondLine
							&& (m_NumberOfPanes == 1 || nPane == 1))
						{
							s.Format(_T("%d"), pPair->pSecondLine->GetLineNumber() + 1);
							if (NULL == pPair->pFirstLine)
							{
								TextColor = pApp->m_AddedTextColor;
							}
							pDC->SetTextColor(TextColor);
							int pos = PosX - 1;
							if (m_NumberOfPanes == 1)
							{
								pos = m_LineNumberMarginWidth - 1;
							}
							pDC->TextOut(pos, PosY, s);
						}
					}

					pDC->SetTextAlign(TA_LEFT | TA_TOP |TA_UPDATECP);

				}
			}

			DrawStringSections(pDC, CPoint(PosX, PosY),
								pSectionEntry, m_FirstPosSeen, nCharsInView, nTabIndent,
								nSelBegin, nSelEnd, nPaneToDraw);
		}
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
	CFilePairDoc * pDoc = GetDocument();
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
			// do scroll, but leave the cursor inside screen boundary
			// if the cursor was outside boundaries, just bring it in
			int dy = 0;
			if (pDoc->m_CaretPos.line >= m_FirstLineSeen
				&& pDoc->m_CaretPos.line <= m_FirstLineSeen + nLinesInView)
			{
				DoVScroll(1);
				if (pDoc->m_CaretPos.line < m_FirstLineSeen)
				{
					dy = 1;
				}
			}
			MoveCaretBy(0, dy, SelectionFlags);
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
			// do scroll, but leave the cursor inside screen boundary
			// if the cursor was outside boundaries, just bring it in
			int dy = 0;
			if (pDoc->m_CaretPos.line >= m_FirstLineSeen
				&& pDoc->m_CaretPos.line <= m_FirstLineSeen + nLinesInView)
			{
				DoVScroll(-1);
				if (pDoc->m_CaretPos.line > m_FirstLineSeen + nLinesInView)
				{
					dy = -1;
				}
			}
			MoveCaretBy(0, dy, SelectionFlags);
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
		MoveCaretBy(0, -(nLinesInView - 1), SelectionFlags | MoveCaretPositionAlways);
		break;

	case VK_NEXT:
		TRACE("VK_NEXT\n");
		// do VScroll and move cursor, to keep the cursor at the same line
		DoVScroll(nLinesInView - 1);
		MoveCaretBy(0, nLinesInView - 1, SelectionFlags | MoveCaretPositionAlways);
		break;

	case VK_TAB:
		// switch selection between panes
		if (2 == m_NumberOfPanes)
		{
			TextPos begin, end;
			if (pDoc->m_CaretPos < pDoc->m_SelectionAnchor)
			{
				begin = pDoc->m_CaretPos;
				end = pDoc->m_SelectionAnchor;
			}
			else
			{
				begin = pDoc->m_SelectionAnchor;
				end = pDoc->m_CaretPos;
			}

			InvalidateRange(begin, end);

			if (0 == m_PaneWithFocus)
			{
				m_PaneWithFocus = 1;
			}
			else
			{
				m_PaneWithFocus = 0;
			}

			InvalidateRange(begin, end);
			CreateAndShowCaret();
		}
		break;
	}
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDiffFileView::MoveCaretBy(int dx, int dy, int flags)
{
	CFilePairDoc * pDoc = GetDocument();
	int NewLine = pDoc->m_CaretPos.line;
	int NewPos = pDoc->m_CaretPos.pos;
	if ((flags & SetPositionCancelSelection)
		&& 0 == (flags & MoveCaretPositionAlways))
	{
		if ((dx < 0 || dy < 0) == (pDoc->m_CaretPos > pDoc->m_SelectionAnchor))
		{
			NewLine = pDoc->m_SelectionAnchor.line;
			NewPos = pDoc->m_SelectionAnchor.pos;
		}
		if (pDoc->m_CaretPos != pDoc->m_SelectionAnchor)
		{
			dx = 0;
		}
	}
	SetCaretPosition(NewPos + dx, NewLine + dy, flags);
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
	CRect cr1 = cr;

	int nPaneWidth = GetPaneWidth();

	cr.left = m_LineNumberMarginWidth;
	cr.right = nPaneWidth - 1;

	for (int nPane = 0; nPane < m_NumberOfPanes; nPane++)
	{
		if (m_NumberOfPanes - 1 == nPane)
		{
			cr.right = cr1.right;
		}

		if (cr.left < cr.right)
		{
			int ndx = (nPos - m_FirstPosSeen) * CharWidth();
			ScrollWindowEx( -ndx, 0, & cr, & cr, NULL, NULL,
							SW_INVALIDATE | SW_ERASE);
		}
		cr.left += nPaneWidth;
		cr.right += nPaneWidth;
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

	if (nLine > GetDocument()->GetTotalLines() - (nLinesInView - 1))
	{
		nLine = GetDocument()->GetTotalLines() - (nLinesInView - 1);
	}

	if (nLine < 0)
	{
		nLine = 0;
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

	SCROLLINFO sci;
	sci.cbSize = sizeof sci;
	sci.nMin = 0;
	sci.nMax = GetDocument()->GetTotalLines();
	sci.nPage = LinesInView();
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

	SCROLLINFO sci;
	sci.cbSize = sizeof sci;
	sci.nMin = 0;
	sci.nMax = 2048;
	sci.nPage = CharsInView();
	sci.nPos = m_FirstPosSeen;
	sci.nTrackPos = 0;
	sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

	SetScrollInfo(SB_HORZ, & sci);
}

void CDiffFileView::MakePositionVisible(int line, int pos)
{
	BringPositionsToBounds(TextPos(line, pos), TextPos(line, pos), m_VisibleRect, m_VisibleRect);
}

void CDiffFileView::MakePositionCentered(int line, int pos)
{
	BringPositionsToBounds(TextPos(line, pos), TextPos(line, pos), m_VisibleRect, m_PreferredRect);
}

void CDiffFileView::InvalidateRange(TextPos begin, TextPos end)
{
	ASSERT(end >= begin);
	CRect r;
	if (0) TRACE("InvalidateRange((%d, %d), (%d, %d))\n", begin, end);

	int nLinesInView = LinesInView();
	int nCharsInView = CharsInView() + 1;

	int nViewOffset = m_LineNumberMarginWidth + m_PaneWithFocus * GetPaneWidth();

	if (begin == end
		|| end.line < m_FirstLineSeen
		|| begin.line > m_FirstLineSeen + nLinesInView + 1)
	{
		return;
	}
	begin.pos -= m_FirstPosSeen;
	begin.line -= m_FirstLineSeen;

	if (begin.line < -1)
	{
		begin.line = -1;
	}

	end.pos -= m_FirstPosSeen;
	end.line -= m_FirstLineSeen;

	if (end.line > nLinesInView + 3)
	{
		end.line = nLinesInView + 3;
	}

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

		r.left = begin.pos * CharWidth() + nViewOffset;
		r.right = end.pos * CharWidth() + nViewOffset + m_FontMetric.tmOverhang;
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
			r.left = begin.pos * CharWidth() + nViewOffset;
			r.right = (nCharsInView + 1) * CharWidth() + nViewOffset + m_FontMetric.tmOverhang;
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
			r.right = end.pos * CharWidth() + nViewOffset + m_FontMetric.tmOverhang;
			r.left = nViewOffset;
			InvalidateRect( & r);
		}

		if (end.line > nLinesInView + 2)
		{
			end.line = nLinesInView + 2;
		}

		if (end.line > begin.line + 1)
		{
			r.left = nViewOffset;
			r.right = (nCharsInView + 1) * CharWidth() + nViewOffset + m_FontMetric.tmOverhang;
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

	CPoint p((pDoc->m_CaretPos.pos - m_FirstPosSeen) * CharWidth()
			+ m_LineNumberMarginWidth + m_PaneWithFocus * GetPaneWidth(),
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
	TRACE("CDiffFileView::OnLButtonDown\n");
	CView::OnLButtonDown(nFlags, point);
	m_LButtonDown = true;
	int flags = SetPositionMakeVisible;

	int nPane = PointToPaneNumber(point.x);
	point.x = PointToPaneOffset(point.x);

	// if the left margin is clicked, the whole line is selected
	point.x -= m_LineNumberMarginWidth;
	int nLine = point.y / LineHeight() + m_FirstLineSeen;

	if (nPane != m_PaneWithFocus)
	{
		// Pane changes, ignore Shift
		nFlags &= ~MK_SHIFT;
		// reset old selection
		SetCaretPosition(0, 0);
	}

	m_PaneWithFocus = nPane;

	if (point.x < 0)
	{
		if (0 == (nFlags & MK_SHIFT))
		{
			SetCaretPosition(0, nLine, SetPositionCancelSelection);
		}
		if (GetDocument()->m_SelectionAnchor.line <= nLine)
		{
			nLine++;
		}
		SetCaretPosition(0, nLine, SetPositionMakeVisible);
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
			flags |= SetWordSelectionMode;
		}
		SetCaretPosition(ClickPos.pos, ClickPos.line, flags);
	}
}

void CDiffFileView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_LButtonDown = false;
	GetDocument()->m_WordSelectionMode = false;

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
		point.x = PointToPaneOffset(point.x, m_PaneWithFocus) - m_LineNumberMarginWidth;

		int nLine = point.y / LineHeight() + m_FirstLineSeen;

		if (point.x < 0)
		{
			if (GetDocument()->m_SelectionAnchor.line <= nLine)
			{
				nLine++;
			}
			SetCaretPosition(0, nLine, SetPositionMakeVisible);
		}
		else
		{
			SetCaretPosition(m_FirstPosSeen + (point.x + CharWidth() / 2) / CharWidth(), nLine, SetPositionMakeVisible);
		}
	}
	CView::OnMouseMove(nFlags, point);
}

BOOL CDiffFileView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_WheelAccumulator += zDelta;
	if (m_WheelAccumulator >= 0)
	{
		while (m_WheelAccumulator >= WHEEL_DELTA)
		{
			m_WheelAccumulator -= WHEEL_DELTA;
			DoVScroll( -3);
		}
	}
	else
	{
		while (m_WheelAccumulator <= - WHEEL_DELTA)
		{
			m_WheelAccumulator += WHEEL_DELTA;
			DoVScroll( +3);
		}
	}
	return TRUE;
}

void CDiffFileView::OnSize(UINT nType, int cx, int cy)
{
	TRACE("CDiffFileView::OnSize\n");
	CView::OnSize(nType, cx, cy);

	m_VisibleRect.top = 0;
	m_VisibleRect.bottom = cy / LineHeight() - 1;
	m_PreferredRect.bottom = m_VisibleRect.bottom / 4;
	m_PreferredRect.top = m_VisibleRect.bottom - m_VisibleRect.bottom / 4;

	m_VisibleRect.right =  (cx - m_LineNumberMarginWidth) / CharWidth();
	m_VisibleRect.left = 0;
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

	m_LineNumberMarginWidth = 0;
	CFilePairDoc * pDoc = GetDocument();
	FilePair * pFilePair = pDoc->GetFilePair();
	if (m_ShowLineNumbers && pFilePair != NULL)
	{
		int nNumChars = 5;
		if ((pFilePair->pFirstFile != NULL
				&& pFilePair->pFirstFile->GetNumLines() > 9999)
			|| (pFilePair->pSecondFile != NULL
				&& pFilePair->pSecondFile->GetNumLines() > 9999))
		{
			nNumChars = 7;
		}
		m_LineNumberMarginWidth = CharWidth() * nNumChars + 1;
		if (pFilePair->pFirstFile != NULL
			&& pFilePair->pSecondFile != NULL
			// two files, one pane
			&& 1 == m_NumberOfPanes)
		{
			m_LineNumberMarginWidth *= 2;
		}
	}

	m_VisibleRect.bottom = cr.Height() / LineHeight() - 1;
	m_PreferredRect.bottom = m_VisibleRect.bottom / 4;
	m_PreferredRect.top = m_PreferredRect.bottom; //m_VisibleRect.bottom - m_VisibleRect.bottom / 4;

	int nPaneWidth = cr.Width() - m_LineNumberMarginWidth * m_NumberOfPanes - (m_NumberOfPanes - 1);

	m_VisibleRect.right = nPaneWidth / CharWidth() / m_NumberOfPanes;

	if (m_VisibleRect.right < 0)
	{
		m_VisibleRect.right = 0;
	}

	m_PreferredRect.left = m_VisibleRect.right / 3;
	m_PreferredRect.right = m_VisibleRect.right - m_VisibleRect.right / 3;

	Invalidate(TRUE);
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
		int nPane = PointToPaneNumber(p.x);
		p.x = PointToPaneOffset(p.x);

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
	TextPos NewPos;
	TextPos EndPos;
	if ( ! pDoc->GetFilePair()->NextDifference(pDoc->DisplayPosToLinePos(pDoc->m_CaretPos),
												pDoc->m_bIgnoreWhitespaces, & NewPos, & EndPos))
	{
		return;
	}
	NewPos = pDoc->LinePosToDisplayPos(NewPos);
	EndPos = pDoc->LinePosToDisplayPos(EndPos);
	SetCaretPosition(NewPos.pos, NewPos.line, SetPositionCancelSelection);
	// make caret position centered and the whole diff visible, if possible
	MakeCaretCenteredRangeVisible(NewPos, EndPos);
}

void CDiffFileView::OnEditGotoprevdiff()
{
	CFilePairDoc * pDoc = GetDocument();
	TextPos NewPos;
	TextPos EndPos;
	if ( ! pDoc->GetFilePair()->PrevDifference(pDoc->DisplayPosToLinePos(pDoc->m_CaretPos),
												pDoc->m_bIgnoreWhitespaces, & NewPos, & EndPos))
	{
		return;
	}
	NewPos = pDoc->LinePosToDisplayPos(NewPos);
	EndPos = pDoc->LinePosToDisplayPos(EndPos);
	SetCaretPosition(NewPos.pos, NewPos.line, SetPositionCancelSelection);
	MakeCaretCenteredRangeVisible(NewPos, EndPos);
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
	else if (lHint == UpdateViewsFilePairChanged)
	{
		FilePairChangedArg * pArg = dynamic_cast<FilePairChangedArg *>(pHint);
		if (NULL != pArg
			&& pArg->pPair == pDoc->GetFilePair())
		{
			OnMetricsChange();
			Invalidate(TRUE);
			UpdateHScrollBar();
			UpdateVScrollBar();
			CreateAndShowCaret();
		}
	}
	else if (lHint == UpdateViewsMetricsChanged)
	{
		OnMetricsChange();
	}
	else if (0 == lHint
			|| UpdateViewsColorsChanged == lHint)
	{
		Invalidate(TRUE);
		UpdateHScrollBar();
		UpdateVScrollBar();
		CreateAndShowCaret();
	}
}

void CDiffFileView::MakeCaretCenteredRangeVisible(TextPos NewPos, TextPos EndPos)
{
	BringPositionsToBounds(NewPos, EndPos, m_VisibleRect, m_PreferredRect);
}

void CDiffFileView::BringPositionsToBounds(TextPos textpos, TextPos endpos, const CRect & AllowedBounds, const CRect & BringToBounds)
{
	// if caret position is inside bounds, it doesn't need to change
	int CaretLine = textpos.line - m_FirstLineSeen;
	int EndLine = endpos.line - m_FirstLineSeen;
	int CaretPos =  textpos.pos - m_FirstPosSeen;
	int EndPos =  endpos.pos - m_FirstPosSeen;

	if (CaretLine < AllowedBounds.top)
	{
		// bring to BringToBounds.top
		DoVScroll(CaretLine - BringToBounds.top);
	}
	else if (EndLine > AllowedBounds.bottom)
	{
		int vscroll = EndLine - AllowedBounds.bottom;
		if (CaretLine - vscroll < BringToBounds.bottom)
		{
			vscroll = CaretLine - BringToBounds.bottom;
		}
		DoVScroll(vscroll);
	}
	if (CaretPos < AllowedBounds.left)
	{
		DoHScroll(CaretPos - BringToBounds.left);
	}
	else if(EndPos > AllowedBounds.right)
	{
		int hscroll = EndPos - AllowedBounds.right;
		if (CaretPos - hscroll < AllowedBounds.left)
		{
			hscroll = CaretPos - BringToBounds.left;
		}
		DoHScroll(hscroll);
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

	OnUpdate(NULL, UpdateViewsMetricsChanged, NULL);
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
	CFrameWnd * pMainFrm = dynamic_cast<CFrameWnd *>(AfxGetMainWnd());
	if (NULL != pMainFrm)
	{
		if (bActivate && this == pActivateView)
		{
			FilePair * pPair = GetDocument()->GetFilePair();
			if (NULL != pPair)
			{
				pMainFrm->SetMessageText(pPair->GetComparisonResult());
			}
		}
		else
		{
			pMainFrm->SetMessageText(AFX_IDS_IDLEMESSAGE);
		}
	}

}

void CDiffFileView::OnEditFind()
{
	if (GetDocument()->OnEditFind())
	{
		MakeCaretCentered();
	}
}

void CDiffFileView::OnEditFindNext()
{
	if (GetDocument()->OnEditFindNext())
	{
		MakeCaretCentered();
	}
}

void CDiffFileView::OnEditFindPrev()
{
	if (GetDocument()->OnEditFindPrev())
	{
		MakeCaretCentered();
	}
}

void CDiffFileView::OnEditFindWordNext()
{
	if (GetDocument()->OnEditFindWordNext())
	{
		MakeCaretCentered();
	}
}

void CDiffFileView::OnEditFindWordPrev()
{
	if (GetDocument()->OnEditFindWordPrev())
	{
		MakeCaretCentered();
	}
}


void CDiffFileView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CView::OnLButtonDblClk(nFlags, point);
	TRACE("CDiffFileView::OnLButtonDblClk\n");
	// select a word
	m_LButtonDown = true;
	point.x -= m_LineNumberMarginWidth;
	if (point.x < 0)
	{
		return;
	}
	SetCaretPosition(m_FirstPosSeen + (point.x + CharWidth() / 2) / CharWidth(),
					point.y / LineHeight() + m_FirstLineSeen,
					SetWordSelectionMode | SetPositionMakeVisible);
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
	CThisApp * pApp = GetApp();
	CFilePairDoc * pDoc = GetDocument();
	FilePair * pFilePair = pDoc->GetFilePair();
	if (NULL == pFilePair)
	{
		return;
	}

	dlg.m_CombinedFileLine = pDoc->m_CaretPos.line;
	dlg.m_CombinedFileNumLines = pFilePair->m_LinePairs.size();
	dlg.m_GoToLineFileSelection = pApp->m_GoToLineFileSelection;

	if (NULL == pFilePair->pFirstFile
		|| NULL == pFilePair->pSecondFile
		|| pFilePair->pFirstFile == pFilePair->pSecondFile)
	{
		dlg.m_bSingleFile = TRUE;
		dlg.m_FirstFileNumLines = dlg.m_CombinedFileNumLines;
		dlg.m_SecondFileNumLines = dlg.m_CombinedFileNumLines;
		dlg.m_FirstFileLine = dlg.m_CombinedFileLine;
		dlg.m_SecondFileLine = dlg.m_CombinedFileLine;
	}
	else
	{
		dlg.m_bSingleFile = FALSE;

		if (NULL != pFilePair->pFirstFile)
		{
			dlg.m_FirstFileNumLines = pFilePair->pFirstFile->GetNumLines();
		}
		if (NULL != pFilePair->pSecondFile)
		{
			dlg.m_SecondFileNumLines = pFilePair->pSecondFile->GetNumLines();
		}


		dlg.m_FirstFileLine = 0;
		dlg.m_SecondFileLine = 0;
		if (dlg.m_CombinedFileLine < dlg.m_CombinedFileNumLines)
		{
			for (int i = pDoc->m_CaretPos.line
				; i >= 0
				&& (0 == dlg.m_FirstFileLine
					|| 0 == dlg.m_SecondFileLine); i--)
			{
				LinePair * pPair = pFilePair->m_LinePairs[i];
				if (0 == dlg.m_FirstFileLine
					&& pPair->pFirstLine != NULL)
				{
					dlg.m_FirstFileLine = pPair->pFirstLine->GetLineNumber();
				}
				if (0 == dlg.m_SecondFileLine
					&& pPair->pSecondLine != NULL)
				{
					dlg.m_SecondFileLine = pPair->pSecondLine->GetLineNumber();
				}
			}
		}
		else
		{
			dlg.m_FirstFileLine = dlg.m_FirstFileNumLines;
			dlg.m_SecondFileLine = dlg.m_SecondFileNumLines;
		}
	}

	dlg.m_CombinedFileLine++;
	dlg.m_FirstFileLine++;
	dlg.m_SecondFileLine++;

	if (IDOK != dlg.DoModal())
	{
		return;
	}

	dlg.m_CombinedFileLine--;
	dlg.m_FirstFileLine--;
	dlg.m_SecondFileLine--;

	int LineNumber;
	if ( dlg.m_bSingleFile)
	{
		LineNumber = dlg.m_LineNumber;
	}
	else
	{
		pApp->m_GoToLineFileSelection = dlg.m_GoToLineFileSelection;

		switch (dlg.m_GoToLineFileSelection)
		{
		case 0:
			// find corresponding line pair
		{
			for (unsigned i = dlg.m_FirstFileLine; i < pFilePair->m_LinePairs.size(); i++)
			{
				LinePair * pPair = pFilePair->m_LinePairs[i];
				if (pPair->pFirstLine != NULL
					&& pPair->pFirstLine->GetLineNumber() >= dlg.m_FirstFileLine)
				{
					LineNumber = i;
					break;
				}
			}
		}
			break;
		case 1:
			// find corresponding line pair
		{
			for (unsigned i = dlg.m_SecondFileLine; i < pFilePair->m_LinePairs.size(); i++)
			{
				LinePair * pPair = pFilePair->m_LinePairs[i];
				if (pPair->pSecondLine != NULL
					&& pPair->pSecondLine->GetLineNumber() >= dlg.m_SecondFileLine)
				{
					LineNumber = i;
					break;
				}
			}
		}
			break;
		default:
			//case 2:
			LineNumber = dlg.m_CombinedFileLine;
			break;
		}
	}

	SetCaretPosition(0, LineNumber, SetPositionMakeCentered | SetPositionCancelSelection);

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


void CDiffFileView::OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame)
{
	CView::OnActivateFrame(nState, pDeactivateFrame);

	CFrameWnd * pMainFrm = dynamic_cast<CFrameWnd *>(AfxGetMainWnd());
	if (NULL != pMainFrm)
	{
		if (WA_INACTIVE == nState)
		{
			pMainFrm->SetMessageText(AFX_IDS_IDLEMESSAGE);
		}
		else if (WA_ACTIVE == nState
				|| WA_CLICKACTIVE == nState)
		{
			FilePair * pPair = GetDocument()->GetFilePair();
			if (NULL != pPair)
			{
				pMainFrm->SetMessageText(pPair->GetComparisonResult());
			}
		}
	}
}

void CDiffFileView::OnViewSideBySide()
{
	FilePair * pFilePair = GetDocument()->GetFilePair();

	if (pFilePair->pFirstFile != NULL
		&& pFilePair->pSecondFile != NULL)
	{
		if (2 == m_NumberOfPanes)
		{
			m_NumberOfPanes = 1;
			m_PaneWithFocus = 0;
		}
		else
		{
			m_NumberOfPanes = 2;
		}
		OnMetricsChange();
	}
}

void CDiffFileView::OnUpdateViewSideBySide(CCmdUI *pCmdUI)
{
	FilePair * pFilePair = GetDocument()->GetFilePair();

	if (pFilePair->pFirstFile != NULL
		&& pFilePair->pSecondFile != NULL)
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(2 == m_NumberOfPanes);
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetCheck(0);
	}
}

int CDiffFileView::PointToPaneNumber(int x)
{
	if (1 == m_NumberOfPanes)
	{
		return 0;
	}

	int nPaneWidth = GetPaneWidth();

	if (nPaneWidth <= 0)
	{
		return 0;
	}

	int nPane = x / nPaneWidth;
	if (nPane >= m_NumberOfPanes)
	{
		nPane = m_NumberOfPanes - 1;
	}
	return nPane;
}

int CDiffFileView::PointToPaneOffset(int x, int nPane)
{
	if (1 == m_NumberOfPanes)
	{
		return x;
	}

	int nPaneWidth = GetPaneWidth();

	if (nPaneWidth <= 0)
	{
		return 0;
	}

	if (-1 == nPane)
	{
		nPane = x / nPaneWidth;
	}

	if (nPane >= m_NumberOfPanes)
	{
		nPane = m_NumberOfPanes - 1;
	}

	return x - nPane * nPaneWidth;
}

int CDiffFileView::GetPaneWidth()
{

	CRect cr;
	GetClientRect( & cr);

	if (1 == m_NumberOfPanes)
	{
		return cr.Width();
	}

	int nPaneWidth = cr.Width() - m_LineNumberMarginWidth * m_NumberOfPanes
					- (m_NumberOfPanes - 1);

	nPaneWidth = nPaneWidth / CharWidth() / m_NumberOfPanes * CharWidth()
				+ m_LineNumberMarginWidth + 1;

	if (nPaneWidth <= 0)
	{
		return 0;
	}
	return nPaneWidth;
}

void CDiffFileView::OnEditCopy()
{
	int FileSelect = 0;
	if (m_NumberOfPanes > 1)
	{
		FileSelect = m_PaneWithFocus + 1;
	}

	GetDocument()->OnEditCopy(FileSelect);
}
