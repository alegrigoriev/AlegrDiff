// BinaryCompareView.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "BinaryCompareView.h"
#include "BinaryCompareDoc.h"
#include "ChildFrm.h"
// CBinaryCompareView

IMPLEMENT_DYNCREATE(CBinaryCompareView, CView)

CBinaryCompareView::CBinaryCompareView()
	: m_ScreenFilePos(0)
	, m_WheelAccumulator(0)
	, m_BytesPerLine(16)
	, m_WordSize(1)
	, m_FirstPosSeen(0)
	, m_DrawnSelBegin(0)
	, m_DrawnSelEnd(0)
	, m_ScrollDataScale(1)
	, m_LButtonDown(FALSE)
	, m_bShowSecondFile(FALSE)
{
	m_FontMetric.tmAveCharWidth = 1;
	m_FontMetric.tmExternalLeading = 1;
	m_FontMetric.tmHeight = 1;
}

CBinaryCompareView::~CBinaryCompareView()
{
}

BEGIN_MESSAGE_MAP(CBinaryCompareView, CView)
	ON_COMMAND(ID_FILE_CANCEL, OnWindowCloseDiff)
	ON_WM_KEYDOWN()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CBinaryCompareView drawing

void CBinaryCompareView::OnDraw(CDC* pDC)
{
	CBinaryCompareDoc* pDoc = GetDocument();
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

	pDC->SetTextAlign(pDC->GetTextAlign() | TA_UPDATECP);

	LONGLONG CurrentAddr = m_ScreenFilePos;
	DWORD TextColor = pApp->m_NormalTextColor;
	DWORD OtherColor, AlternateColor;
	FileItem * pFile;
	FileItem * pOtherFile;
	if (m_bShowSecondFile || NULL == pFilePair->pFirstFile)
	{
		pFile = pFilePair->pSecondFile;
		pOtherFile = pFilePair->pFirstFile;
		OtherColor = pApp->m_AddedTextColor;
		AlternateColor = pApp->m_ErasedTextColor;
	}
	else
	{
		pFile = pFilePair->pFirstFile;
		pOtherFile = pFilePair->pSecondFile;
		OtherColor = pApp->m_ErasedTextColor;
		AlternateColor = pApp->m_AddedTextColor;
	}

	// draw inside update rectangle
	for (int CurrentY = ur.top - ur.top % LineHeight(); CurrentY < ur.bottom; CurrentY += LineHeight())
	{
		if (CurrentAddr >= pFile->GetFileLength()
			&& (NULL == pOtherFile || CurrentAddr >= pOtherFile->GetFileLength()))
		{
			break;
		}
		// TODO: support hor scroll
		pDC->MoveTo(0, CurrentY);
		TCHAR buf[256];
		UCHAR FileBuf1[256];
		unsigned Buf1Filled;
		UCHAR FileBuf2[256];
		unsigned Buf2Filled;

		if (m_BytesPerLine <= sizeof FileBuf1)
		{
			Buf1Filled = pFile->GetFileData(CurrentAddr, & FileBuf1, sizeof FileBuf1);
			if (NULL != pOtherFile)
			{
				Buf2Filled = pOtherFile->GetFileData(CurrentAddr, & FileBuf2, sizeof FileBuf2);
			}
			else
			{
				Buf2Filled = 0;
			}
		}
		else
		{
			Buf1Filled = 0;
			Buf2Filled = 0;
		}

		LPCTSTR format;
		LONGLONG FileSize = pDoc->GetFileSize();

		if (FileSize >= 0x00FFFFFFFFFFFFFFi64)
		{
			format = _T("%016I64X  ");
		}
		else if (FileSize >= 0x0000FFFFFFFFFFFFi64)
		{
			format = _T("%014I64X  ");
		}
		else if (FileSize >= 0x000000FFFFFFFFFFi64)
		{
			format = _T("%012I64X  ");
		}
		else if (FileSize >= 0x00000000FFFFFFFFi64)
		{
			format = _T("%010I64X  ");
		}
		else
		{
			format = _T("%08I64X  ");
		}

		_stprintf(buf, format, CurrentAddr);

		pDC->SetBkColor(pApp->m_TextBackgroundColor);
		pDC->SelectObject( & pApp->m_NormalFont);
		pDC->SetTextColor(pApp->m_NormalTextColor);
		pDC->TextOut(0, CurrentY, buf, _tcslen(buf));

		for (unsigned offset = 0;
			offset < m_BytesPerLine && (offset < Buf1Filled || offset < Buf2Filled);
			offset += m_WordSize)
		{
			for (int ByteNum = 1; ByteNum <= m_WordSize; ByteNum++)
			{
				DWORD color = TextColor;
				if (Buf1Filled > offset + m_WordSize - ByteNum)
				{
					_stprintf(buf, _T("%02X"), FileBuf1[offset + m_WordSize - ByteNum]);
					if (Buf2Filled <= offset + m_WordSize - ByteNum
						|| FileBuf1[offset + m_WordSize - ByteNum]
						!= FileBuf2[offset + m_WordSize - ByteNum])
					{
						color = OtherColor;
					}
				}
				else if (Buf2Filled > offset + m_WordSize - ByteNum)
				{
					_stprintf(buf, _T("%02X"), FileBuf2[offset + m_WordSize - ByteNum]);
					color = AlternateColor;
				}
				else
				{
					buf[0] = '?';
					buf[1] = '?';
					buf[2] = 0;
				}
				int chars = 2;
				if (ByteNum == m_WordSize)
				{
					buf[2] = ' ';
					buf[3] = 0;
					chars = 3;
				}
				pDC->SetTextColor(color);
				pDC->TextOut(0, 0, buf, chars);
			}

		}
		CurrentAddr += m_BytesPerLine;
	}

}


// CBinaryCompareView diagnostics

#ifdef _DEBUG
void CBinaryCompareView::AssertValid() const
{
	CView::AssertValid();
}

void CBinaryCompareView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
CBinaryCompareDoc * CBinaryCompareView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBinaryCompareDoc)));
	return (CBinaryCompareDoc*)m_pDocument;
}
#endif //_DEBUG


// CBinaryCompareView message handlers
void CBinaryCompareView::OnMetricsChange()
{
	TRACE("CBinaryCompareView::OnMetricsChange\n");

	// create font
	CThisApp * pApp = GetApp();

	CWindowDC wdc(this);
	CFont * pFont =  & pApp->m_NormalFont;

	CFont font;
	if (pApp->m_NormalLogFont.lfItalic
		//|| pApp->m_AddedLogFont.lfItalic
		//|| pApp->m_ErasedLogFont.lfItalic
		|| pApp->m_NormalLogFont.lfWeight > FW_NORMAL
		//|| pApp->m_AddedLogFont.lfWeight > FW_NORMAL
		//|| pApp->m_ErasedLogFont.lfWeight > FW_NORMAL
		)
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

	CBinaryCompareDoc * pDoc = GetDocument();
	FilePair * pFilePair = pDoc->GetFilePair();

	m_VisibleRect.bottom = cr.Height() / LineHeight() - 1;
	//m_PreferredRect.bottom = m_VisibleRect.bottom / 4;
	//m_PreferredRect.top = m_PreferredRect.bottom; //m_VisibleRect.bottom - m_VisibleRect.bottom / 4;

	m_VisibleRect.right =  cr.Width() / CharWidth();
	if (m_VisibleRect.right < 0)
	{
		m_VisibleRect.right = 0;
	}

	//m_PreferredRect.left = m_VisibleRect.right / 3;
	//m_PreferredRect.right = m_VisibleRect.right - m_VisibleRect.right / 3;

	Invalidate(TRUE);
	//UpdateVScrollBar();
	//UpdateHScrollBar();
	CreateAndShowCaret();
}

void CBinaryCompareView::CreateAndShowCaret()
{
	CBinaryCompareDoc * pDoc = GetDocument();
	if (this != GetFocus())
	{
		TRACE("CBinaryCompareView::CreateAndShowCaret - No Focus\n");
		return;
	}
	if (0 == GetDocument()->GetFileSize())
	{
		TRACE("CBinaryCompareView::CreateAndShowCaret - 0 == GetDocument()->GetFileSize()\n");
		return;
	}
#if 0
	CPoint p((pDoc->m_CaretPos.pos - m_FirstPosSeen) * CharWidth() + m_LineNumberMarginWidth,
			(pDoc->m_CaretPos.line - m_FirstLineSeen) * LineHeight());
	if (pDoc->m_CaretPos.pos >= m_FirstPosSeen
		&& pDoc->m_CaretPos.line >= m_FirstLineSeen
		&& pDoc->m_CaretPos.line <= m_FirstLineSeen + LinesInView() + 1)
	{
		CreateSolidCaret(2, LineHeight());
		if (0) TRACE("CBinaryCompareView::CreateAndShowCaret %d %d\n", p.x, p.y);
		SetCaretPos(p);
		ShowCaret();
	}
	else
	{
		DestroyCaret();
	}
#endif
}


void CBinaryCompareView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	OnMetricsChange();
}

void CBinaryCompareView::OnWindowCloseDiff()
{
	CChildFrame * pFrame = (CChildFrame *)GetParentFrame();
	ASSERT_VALID(pFrame);
	pFrame->OnClose();
	return;

}


BOOL CBinaryCompareView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_VSCROLL | WS_HSCROLL;

	return CView::PreCreateWindow(cs);
}

void CBinaryCompareView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CBinaryCompareDoc * pDoc = GetDocument();
	bool ShiftPressed = (0 != (0x8000 & GetKeyState(VK_SHIFT)));
	bool CtrlPressed = (0 != (0x8000 & GetKeyState(VK_CONTROL)));
	int SelectionFlags = SetPositionMakeVisible;
	if ( ! ShiftPressed)
	{
		SelectionFlags |= SetPositionCancelSelection;
	}
	int nLinesInView = LinesInView();
	int nBytesInView = nLinesInView * m_BytesPerLine;

	switch (nChar)
	{
	case VK_DOWN:
		TRACE("VK_DOWN\n");
		if (CtrlPressed)
		{
			// do scroll, but leave the cursor inside screen boundary
			// if the cursor was outside boundaries, just bring it in
			int dy = 0;
			if (pDoc->m_CaretPos >= m_ScreenFilePos
				&& pDoc->m_CaretPos <= m_ScreenFilePos + nBytesInView)
			{
				DoVScroll(1);
				if (pDoc->m_CaretPos < m_ScreenFilePos)
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
			if (pDoc->m_CaretPos >= m_ScreenFilePos
				&& pDoc->m_CaretPos <= m_ScreenFilePos + nBytesInView)
			{
				DoVScroll(-1);
				if (pDoc->m_CaretPos > m_ScreenFilePos + nBytesInView)
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
		MoveCaretBy(-1, 0, SelectionFlags);
		break;

	case VK_RIGHT:
		TRACE("VK_RIGHT\n");
		MoveCaretBy(1, 0, SelectionFlags);
		break;

	case VK_HOME:
		TRACE("VK_HOME\n");
		if (CtrlPressed)
		{
			SetCaretPosition(0, SelectionFlags);
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
			SetCaretPosition(pDoc->GetFileSize(), SelectionFlags);
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
	}
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CBinaryCompareView::MoveCaretBy(int dx, int dy, int flags)
{
	CBinaryCompareDoc * pDoc = GetDocument();
	LONGLONG NewLine = pDoc->m_CaretPos / m_BytesPerLine;
	int NewPos = unsigned(pDoc->m_CaretPos) % m_BytesPerLine;

	if ((flags & SetPositionCancelSelection)
		&& 0 == (flags & MoveCaretPositionAlways))
	{
		if ((dx < 0 || dy < 0) == (pDoc->m_CaretPos > pDoc->m_SelectionAnchor))
		{
			NewLine = pDoc->m_SelectionAnchor / m_BytesPerLine;
			NewPos = unsigned(pDoc->m_SelectionAnchor) % m_BytesPerLine;
		}
		if (pDoc->m_CaretPos != pDoc->m_SelectionAnchor)
		{
			dx = 0;
		}
	}
	SetCaretPosition(NewPos + dx + (NewLine + dy) * m_BytesPerLine, flags);
}


void CBinaryCompareView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
		VScrollToTheAddr(0);
		break;

	case SB_BOTTOM:
		TRACE("OnVScroll SB_BOTTOM, nPos=%d\n", nPos);
		VScrollToTheAddr(GetDocument()->GetFileSize() - nLinesInView * m_BytesPerLine);
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
		VScrollToTheAddr(nPos);
		break;
	default:
		break;
	}
}

void CBinaryCompareView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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

void CBinaryCompareView::DoHScroll(int nCharsToScroll)
{
	// > 0 - scroll left (to see chars toward end),
	// < 0 - scroll right (to see chars to the begin of the line)
	HScrollToThePos(nCharsToScroll + m_FirstPosSeen);
}

void CBinaryCompareView::HScrollToThePos(int nPos)
{
	int nCharsInView = CharsInView();

	if (nPos < 0)
	{
		nPos = 0;
	}

	if (nPos > 2048 - GetMaxChars())
	{
		nPos = 2048 - GetMaxChars();
	}

	if (nPos == m_FirstPosSeen)
	{
		return;
	}
	// need to scroll the view

	CRect cr;

	GetClientRect( & cr);

	cr.left = 16 * CharWidth();
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

	// > 0 - scroll up (to see lines toward end),
	// < 0 - scroll down (to see lines to the begin of the file)
void CBinaryCompareView::DoVScroll(int nLinesToScroll)
{
	VScrollToTheAddr(m_ScreenFilePos + nLinesToScroll * m_BytesPerLine);
}

void CBinaryCompareView::VScrollToTheAddr(LONGLONG Addr)
{
	int nLinesInView = LinesInView();
	if (Addr < 0)
	{
		Addr = 0;
	}
	int BytesPerScreen = nLinesInView * m_BytesPerLine;
	LONGLONG FileSize = GetDocument()->GetFileSize();

	if (Addr > FileSize - (BytesPerScreen - m_BytesPerLine))
	{
		Addr = FileSize - (BytesPerScreen - m_BytesPerLine);
	}

	LONGLONG nOffset = Addr - m_ScreenFilePos;
	if (nOffset <= BytesPerScreen && nOffset >= -BytesPerScreen)
	{
		// need to scroll the view
		int ndy = LONG(nOffset) / m_BytesPerLine;
		if (0 == ndy)
		{
			return;
		}
		m_ScreenFilePos += ndy * m_BytesPerLine;

		ndy *= LineHeight();

		ScrollWindowEx(0, -ndy, NULL, NULL, NULL, NULL,
						SW_INVALIDATE | SW_ERASE);

		if (m_LButtonDown)
		{
			SetTimer(1, 20, NULL);
		}
	}
	else
	{
		m_ScreenFilePos = Addr - Addr % m_BytesPerLine;
		Invalidate();
	}
	UpdateVScrollBar();
	CreateAndShowCaret();
}

void CBinaryCompareView::UpdateVScrollBar()
{
	if (0 == LineHeight())
	{
		return;
	}

	SCROLLINFO sci;
	sci.cbSize = sizeof sci;
	sci.nMin = 0;
	// TODO: scale nMax and nPos
	sci.nMax = int(GetDocument()->GetFileSize() / m_BytesPerLine);
	sci.nPage = LinesInView();
	sci.nPos = int(m_ScreenFilePos / m_BytesPerLine);
	// TODO: for Win9x limit the paramaters to 16 bit
	sci.nTrackPos = 0;
	sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

	SetScrollInfo(SB_VERT, & sci);
}

int CBinaryCompareView::GetMaxChars() const
{
	return 18 + m_BytesPerLine * 2
			+ m_BytesPerLine / m_WordSize;
}

void CBinaryCompareView::UpdateHScrollBar()
{
	if (0 == CharWidth())
	{
		return;
	}

	SCROLLINFO sci;
	sci.cbSize = sizeof sci;
	sci.nMin = 0;
	sci.nMax = GetMaxChars();
	sci.nPage = CharsInView();
	sci.nPos = m_FirstPosSeen;
	sci.nTrackPos = 0;
	sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

	SetScrollInfo(SB_HORZ, & sci);
}

void CBinaryCompareView::SetCaretPosition(LONGLONG Addr, int flags)
{
	CBinaryCompareDoc * pDoc = GetDocument();
	pDoc->SetCaretPosition(Addr, flags);

	if (flags & SetPositionMakeVisible)
	{
		MakeCaretVisible();
	}
	else if (flags & SetPositionMakeCentered)
	{
		MakeCaretCentered();
	}
}



void CBinaryCompareView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
	CBinaryCompareDoc * pDoc = GetDocument();
	if (lHint == CBinaryCompareDoc::CaretPositionChanged)
	{
		// invalidate selection
		// invalidate where the selection changed
		// first we make sorted array of old and new selection boundaries
		// then we invalidate between 0, 1 and 2, 3
		LONGLONG Sel[4];
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
			LONGLONG tmp = Sel[1];
			Sel[1] = Sel[2];
			Sel[2] = tmp;
		}
		// array is sorted
		InvalidateRange(Sel[0], Sel[1]);
		InvalidateRange(Sel[2], Sel[3]);
	}
	else if (lHint == CBinaryCompareDoc::InvalidateRange)
	{
		InvalidatedRangeBin * pRange = dynamic_cast<InvalidatedRangeBin *>(pHint);
		if (NULL != pRange)
		{
			LONGLONG begin, end;
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
	else if (lHint == CBinaryCompareDoc::FileLoaded
			|| lHint == CBinaryCompareDoc::MetricsChanged)
	{
		OnMetricsChange();
	}
	else
	{
		Invalidate(TRUE);
		UpdateHScrollBar();
		UpdateVScrollBar();
		CreateAndShowCaret();
	}
}
