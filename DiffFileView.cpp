// DiffFileView.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DiffFileView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDiffFileView

IMPLEMENT_DYNCREATE(CDiffFileView, CView)

CDiffFileView::CDiffFileView()
	: m_pFilePair(NULL),
	m_FirstLineSeen(0),
	m_FirstPosSeen(0),
	m_CaretLine(0),
	m_CaretPos(0),
	m_TotalLines(0),
	m_BaseOnFirstFile(true)
{
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiffFileView drawing
void CDiffFileView::DrawStringSections(CDC* pDC, CPoint point,
										StringSection const Sections[], int NumOfSections,
										int nSkipChars, int nVisibleChars, int nTabIndent)
{
	TCHAR buf[2048];
	pDC->MoveTo(point);
	int ExpandedLinePos = 0;    // poisition with expanded tabs
	for (int i = 0, nDrawnChars = 0; i < NumOfSections && nVisibleChars > nDrawnChars; i++)
	{
		LPCTSTR pText = Sections[i].pBegin;
		int Length = Sections[i].Length;
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
			Length--;
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
		CFont * pFont;
		if (Sections[i].Attr == Sections[i].Inserted)
		{
			pFont = & m_UnderlineFont;
		}
		else if (Sections[i].Attr == Sections[i].Erased)
		{
			pFont = & m_StrikeoutFont;
		}
		else
		{
			pFont = & m_NormalFont;
		}
		pDC->SelectObject(pFont);
		// text is drawn from the current position
		pDC->TextOut(0, 0, pText, Length);
		nDrawnChars += Length;
	}
}

void CDiffFileView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	if (NULL == m_pFilePair)
	{
		return;
	}
	FileItem * pFile = m_pFilePair->pFirstFile;
	if (NULL == pFile
		|| (! m_BaseOnFirstFile && NULL != m_pFilePair->pSecondFile))
	{
		pFile = m_pFilePair->pSecondFile;
	}
	if (NULL == pFile)
	{
		return;
	}

	CFont * pOldFont = pDC->SelectObject( & m_NormalFont);

	TEXTMETRIC tm;
	pDC->GetTextMetrics( & tm);

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
	int nLineHeight = LineHeight();
	int nTabWidth = GetApp()->m_TabIndent * CharWidth();
	for (int nLine = m_FirstLineSeen,
		PosY = cr.top;
		nLine < pFile->GetNumLines() && PosY < cr.bottom;
		nLine++, PosY += nLineHeight)
	{
		const FileLine * pLine = pFile->GetLine(nLine);
		if (pLine->GetLength() > m_FirstPosSeen)
		{
			pDC->TabbedTextOut(0, PosY,
								LPCTSTR(pLine->GetText()) + m_FirstPosSeen,
								pLine->GetLength() - m_FirstPosSeen, 1, & nTabWidth, 0);
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
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDiffFileView message handlers

void CDiffFileView::OnWindowCloseDiff()
{
	CFrameWnd* pFrame = GetParentFrame();
	ASSERT_VALID(pFrame);

	// and close it
	GetDocument()->PreCloseFrame(pFrame);
	pFrame->DestroyWindow();
}

void CDiffFileView::OnInitialUpdate()
{
	// create font
	BOOL success = m_NormalFont.CreateFont(20, 0, 0, 0, FW_NORMAL,
											false, false, false,
											ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
											DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, NULL /*"Courier New" */);
	success = m_UnderlineFont.CreateFont(20, 0, 0, 0, FW_NORMAL,
										false, true, false,
										ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
										DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, NULL /*"Courier New" */);
	success = m_StrikeoutFont.CreateFont(20, 0, 0, 0, FW_NORMAL,
										false, false, true,
										ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
										DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, NULL /*"Courier New" */);
	{
		CWindowDC wdc(this);
		CFont * pOldFont = wdc.SelectObject( & m_NormalFont);
		wdc.GetTextMetrics( & m_FontMetric);
		wdc.SelectObject(pOldFont);
	}
	// build the line pair array
	FileItem * pFile = m_pFilePair->pFirstFile;
	if (NULL == pFile
		|| (! m_BaseOnFirstFile && NULL != m_pFilePair->pSecondFile))
	{
		pFile = m_pFilePair->pSecondFile;
	}

	m_TotalLines = pFile->GetNumLines();
	CView::OnInitialUpdate();
	UpdateVScrollBar();
	UpdateHScrollBar();
	CreateAndShowCaret();
}

void CDiffFileView::OnDestroy()
{
	if (m_pFilePair)
	{
		m_pFilePair->UnloadFiles();
		m_pFilePair = NULL;
	}
	CView::OnDestroy();
}

BOOL CDiffFileView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_VSCROLL | WS_HSCROLL;
	return CView::PreCreateWindow(cs);
}

int CDiffFileView::LinesInView() const
{
	CRect cr;
	GetClientRect( & cr);
	return cr.Height() / LineHeight();
}

int CDiffFileView::CharsInView() const
{
	CRect cr;
	GetClientRect( & cr);
	return cr.Width() / CharWidth();
}

void CDiffFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	bool ShiftPressed = (0 != (0x8000 & GetKeyState(VK_SHIFT)));
	bool CtrlPressed = (0 != (0x8000 & GetKeyState(VK_CONTROL)));
	bool bCancelSelection = ! ShiftPressed;

	int nLinesInView = LinesInView();

	switch (nChar)
	{
	case VK_DOWN:
		TRACE("VK_DOWN\n");
		MoveCaretBy(0, 1, bCancelSelection);
		break;

	case VK_UP:
		TRACE("VK_UP\n");
		MoveCaretBy(0, -1, bCancelSelection);
		break;

	case VK_LEFT:
		TRACE("VK_LEFT\n");
		MoveCaretBy(-1, 0, bCancelSelection);
		break;

	case VK_RIGHT:
		TRACE("VK_RIGHT\n");
		MoveCaretBy(1, 0, bCancelSelection);
		break;

	case VK_HOME:
		TRACE("VK_HOME\n");
		if (CtrlPressed)
		{
			SetCaretPosition(0, 0, bCancelSelection);
		}
		else
		{
		}
		break;

	case VK_END:
		TRACE("VK_END\n");
		if (CtrlPressed)
		{
			SetCaretPosition(0, m_TotalLines, bCancelSelection);
		}
		else
		{
		}
		break;

	case VK_PRIOR:
		TRACE("VK_PRIOR\n");
		// do VScroll and move cursor, to keep the cursor at the same line
		DoVScroll(-(nLinesInView - 1));
		MoveCaretBy(0, -(nLinesInView - 1), bCancelSelection);
		break;

	case VK_NEXT:
		TRACE("VK_NEXT\n");
		// do VScroll and move cursor, to keep the cursor at the same line
		DoVScroll(nLinesInView - 1);
		MoveCaretBy(0, nLinesInView - 1, bCancelSelection);
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
	int ndx = (nPos - m_FirstPosSeen) * CharWidth();
	m_FirstPosSeen = nPos;
	ScrollWindowEx( -ndx, 0, NULL, NULL, NULL, NULL,
					SW_INVALIDATE | SW_ERASE);

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

	if (nLine > m_TotalLines - nLinesInView)
	{
		nLine = m_TotalLines - nLinesInView;
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
		VScrollToTheLine(m_TotalLines - nLinesInView);
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
	int nLinesInView = LinesInView();

	SCROLLINFO sci;
	sci.cbSize = sizeof sci;
	sci.nMin = 0;
	sci.nMax = m_TotalLines;
	sci.nPage = nLinesInView;
	sci.nPos = m_FirstLineSeen;
	sci.nTrackPos = 0;
	sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

	SetScrollInfo(SB_VERT, & sci);
}

void CDiffFileView::UpdateHScrollBar()
{
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
	int MoveX = 0;
	int MoveY = 0;
	int nLinesInView = LinesInView();
	int nCharsInView = CharsInView();
	if (m_CaretLine < m_FirstLineSeen)
	{
		MoveY = m_FirstLineSeen - m_CaretLine;
	}
	else if (m_CaretLine >= m_FirstLineSeen + nLinesInView)
	{
		MoveY = m_FirstLineSeen + nLinesInView - 1 - m_CaretLine;
	}

	if (m_CaretPos < m_FirstPosSeen)
	{
		MoveX = m_FirstPosSeen - m_CaretPos;
	}
	else if (m_CaretPos >= m_FirstPosSeen + nCharsInView)
	{
		MoveX = m_FirstPosSeen + nCharsInView - m_CaretPos;
	}

	m_FirstLineSeen -= MoveY;
	m_FirstPosSeen -= MoveX;
	if (0 != MoveX)
	{
		ScrollWindowEx(MoveX * CharWidth(), 0, NULL, NULL, NULL, NULL,
						SW_INVALIDATE | SW_ERASE);
		UpdateHScrollBar();
	}
	if (0 != MoveY)
	{
		ScrollWindowEx(0, MoveY * LineHeight(), NULL, NULL, NULL, NULL,
						SW_INVALIDATE | SW_ERASE);
		UpdateVScrollBar();
	}

	if (0 != MoveX || 0 != MoveY)
	{
		CreateAndShowCaret();
	}
}

void CDiffFileView::SetCaretPosition(int pos, int line, bool bCancelSelection)
{
	if (line > m_TotalLines)
	{
		line = m_TotalLines;
	}
	if (line < 0)
	{
		line = 0;
	}
	m_CaretLine = line;

	if (pos < 0)
	{
		pos = 0;
	}
	if (pos > 2048)
	{
		pos = 2048;
	}
	m_CaretPos = pos;

	MakePositionVisible(m_CaretPos, m_CaretLine);
	CreateAndShowCaret();
}

void CDiffFileView::CreateAndShowCaret()
{
	if (this != GetFocus())
	{
		TRACE("CDiffFileView::CreateAndShowCaret - No Focus\n");
		return;
	}
	if (0 == m_TotalLines)
	{
		return;
	}
	CPoint p((m_CaretPos - m_FirstPosSeen) * CharWidth(),
			(m_CaretLine - m_FirstLineSeen) * LineHeight());

	CreateSolidCaret(2, LineHeight());

	SetCaretPos(p);
	ShowCaret();
}

void CDiffFileView::OnSetFocus(CWnd* pOldWnd)
{
	CView::OnSetFocus(pOldWnd);

	CreateAndShowCaret();
}

void CDiffFileView::CancelSelection()
{
	// TODO
}

void CDiffFileView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CView::OnLButtonDown(nFlags, point);
	SetCaretPosition(m_FirstPosSeen + (point.x + CharWidth() / 2) / CharWidth(),
					point.y / LineHeight() + m_FirstLineSeen,
					0 == (nFlags & MK_SHIFT));
}

void CDiffFileView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnLButtonUp(nFlags, point);
}

void CDiffFileView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: make mouse selection

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
	CView::OnSize(nType, cx, cy);
	UpdateVScrollBar();
	UpdateHScrollBar();

}

BOOL CDiffFileView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	SetCursor(GetApp()->LoadCursor(IDC_CURSOR_BEAM));
	return TRUE;

	return CView::OnSetCursor(pWnd, nHitTest, message);
}
