// BinaryCompareView.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "BinaryCompareView.h"
#include "BinaryCompareDoc.h"
#include "ChildFrm.h"
#include "DifferenceProgressDialog.h"
#include <afxpriv.h>
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
	, m_PreferredRect(0, 0, 0, 0)
	, m_VisibleRect(0, 0, 0, 0)
	, m_TrackingSelection(FALSE)
	, m_bShowSecondFile(FALSE)
	, m_AddressMarginWidth(8)
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
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_WM_KILLFOCUS()
	ON_WM_CAPTURECHANGED()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_WORDSIZE_1BYTE, OnWordsize1byte)
	ON_UPDATE_COMMAND_UI(ID_WORDSIZE_1BYTE, OnUpdateWordsize1byte)
	ON_COMMAND(ID_WORDSIZE_2BYTES, OnWordsize2bytes)
	ON_UPDATE_COMMAND_UI(ID_WORDSIZE_2BYTES, OnUpdateWordsize2bytes)
	ON_COMMAND(ID_WORDSIZE_4BYTES, OnWordsize4bytes)
	ON_UPDATE_COMMAND_UI(ID_WORDSIZE_4BYTES, OnUpdateWordsize4bytes)
	ON_COMMAND(ID_WORDSIZE_8BYTES, OnWordsize8bytes)
	ON_UPDATE_COMMAND_UI(ID_WORDSIZE_8BYTES, OnUpdateWordsize8bytes)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_BINDIFF_SHOWFIRSTFILE, OnBindiffShowfirstfile)
	ON_UPDATE_COMMAND_UI(ID_BINDIFF_SHOWFIRSTFILE, OnUpdateBindiffShowfirstfile)
	ON_COMMAND(ID_BINDIFF_SHOW2NDFILE, OnBindiffShow2ndfile)
	ON_UPDATE_COMMAND_UI(ID_BINDIFF_SHOW2NDFILE, OnUpdateBindiffShow2ndfile)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_EDIT_GOTONEXTDIFF, OnEditGotonextdiff)
	ON_COMMAND(ID_EDIT_GOTOPREVDIFF, OnEditGotoprevdiff)
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
	LONGLONG SelBegin, SelEnd;
	if (pDoc->m_CaretPos <= pDoc->m_SelectionAnchor)
	{
		SelBegin = pDoc->m_CaretPos & - m_WordSize;
		SelEnd = pDoc->m_SelectionAnchor & - m_WordSize;
	}
	else
	{
		SelBegin = pDoc->m_SelectionAnchor & - m_WordSize;
		SelEnd = pDoc->m_CaretPos & - m_WordSize;
	}

	for (int CurrentY = ur.top - ur.top % LineHeight(); CurrentY < ur.bottom; CurrentY += LineHeight())
	{
		LONGLONG CurrentAddr = m_ScreenFilePos + CurrentY / LineHeight() * m_BytesPerLine;
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

		_stprintf(buf, _T("%0*I64X  "), m_AddressMarginWidth, CurrentAddr);

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

				DWORD BackgroundColor = pApp->m_TextBackgroundColor;
				if (CurrentAddr + offset < SelEnd
					&& CurrentAddr + offset >= SelBegin)
				{
					color = pApp->m_SelectedTextColor;
					BackgroundColor = 0x000000;
				}

				pDC->SetBkColor(BackgroundColor);
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
	m_PreferredRect.bottom = m_VisibleRect.bottom / 4;
	m_PreferredRect.top = m_PreferredRect.bottom; //m_VisibleRect.bottom - m_VisibleRect.bottom / 4;

	m_VisibleRect.right =  cr.Width() / CharWidth();
	if (m_VisibleRect.right < 0)
	{
		m_VisibleRect.right = 0;
	}

	m_PreferredRect.left = m_VisibleRect.right / 3;
	m_PreferredRect.right = m_VisibleRect.right - m_VisibleRect.right / 3;

	LONGLONG FileSize = pDoc->GetFileSize();

	if (FileSize >= 0x00FFFFFFFFFFFFFFi64)
	{
		m_AddressMarginWidth = 16;
	}
	else if (FileSize >= 0x0000FFFFFFFFFFFFi64)
	{
		m_AddressMarginWidth = 14;
	}
	else if (FileSize >= 0x000000FFFFFFFFFFi64)
	{
		m_AddressMarginWidth = 12;
	}
	else if (FileSize >= 0x00000000FFFFFFFFi64)
	{
		m_AddressMarginWidth = 10;
	}
	else
	{
		m_AddressMarginWidth = 8;
	}

	Invalidate(TRUE);
	UpdateVScrollBar();
	UpdateHScrollBar();
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
	if (pDoc->m_CaretPos >= m_ScreenFilePos
		&& pDoc->m_CaretPos <= m_ScreenFilePos + (LinesInView() + 1) * m_BytesPerLine)
	{
		int ByteOnTheLine = (ULONG(pDoc->m_CaretPos - m_ScreenFilePos) % m_BytesPerLine)
							& -m_WordSize;

		CPoint p((ByteOnTheLine * 2 + ByteOnTheLine / m_WordSize
					+ m_AddressMarginWidth + 2) * CharWidth(),
				ULONG(pDoc->m_CaretPos - m_ScreenFilePos) / m_BytesPerLine * LineHeight());
		CreateSolidCaret(2, LineHeight());
		if (0) TRACE("CBinaryCompareView::CreateAndShowCaret %d %d\n", p.x, p.y);
		SetCaretPos(p);
		ShowCaret();
	}
	else
	{
		DestroyCaret();
	}
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
		MoveCaretBy(- m_WordSize, 0, SelectionFlags);
		break;

	case VK_RIGHT:
		TRACE("VK_RIGHT\n");
		MoveCaretBy(m_WordSize, 0, SelectionFlags);
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
	NewPos = (NewPos + dx) & -m_WordSize;
	SetCaretPosition(NewPos + (NewLine + dy) * m_BytesPerLine, flags);
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
		VScrollToTheAddr(nPos * m_ScrollDataScale * m_BytesPerLine);
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

	int CharsToScroll = GetMaxChars() - nCharsInView;
	if (CharsToScroll < 0)
	{
		CharsToScroll = 0;
	}

	if (nPos > CharsToScroll)
	{
		nPos = CharsToScroll;
	}

	if (nPos < 0)
	{
		nPos = 0;
	}

	if (nPos == m_FirstPosSeen)
	{
		return;
	}
	// need to scroll the view

	CRect cr;

	GetClientRect( & cr);

	cr.left = 0;
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
	LONGLONG nLines = GetDocument()->GetFileSize() / m_BytesPerLine;
	LONGLONG nCurLine = m_ScreenFilePos / m_BytesPerLine;
	sci.nPage = LinesInView();
	// limit the paramaters to 16 bit
	m_ScrollDataScale = 1;
	while (nLines > SHRT_MAX)
	{
		nLines >>= 1;
		nCurLine >>= 1;
		sci.nPage >>= 1;
		m_ScrollDataScale <<= 1;
	}
	if (0 == sci.nPage)
	{
		sci.nPage = 1;
	}
	sci.nMax = int(nLines);
	sci.nPos = int(nCurLine);
	TRACE("CBinaryCompareView::UpdateVScrollBar pos = %d\n", sci.nPos);
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

void CBinaryCompareView::CaretToEnd(int flags)
{
	CBinaryCompareDoc * pDoc = GetDocument();
	LONGLONG NewPos = pDoc->m_CaretPos | (m_BytesPerLine - 1);

	if ((flags & SetPositionCancelSelection)
		&& pDoc->m_CaretPos < pDoc->m_SelectionAnchor)
	{
		NewPos = pDoc->m_SelectionAnchor;
	}

	pDoc->SetCaretPosition(NewPos, flags);
	MakeCaretVisible();
}

void CBinaryCompareView::CaretToHome(int flags)
{
	CBinaryCompareDoc * pDoc = GetDocument();
	LONGLONG NewPos = pDoc->m_CaretPos - ULONG(pDoc->m_CaretPos) % m_BytesPerLine;

	if ((flags & SetPositionCancelSelection)
		&& pDoc->m_CaretPos > pDoc->m_SelectionAnchor)
	{
		NewPos = pDoc->m_SelectionAnchor;
	}

	pDoc->SetCaretPosition(NewPos, flags);
	MakeCaretVisible();
}

void CBinaryCompareView::BringPositionsToBounds(LONGLONG textpos, LONGLONG endpos, const CRect & AllowedBounds, const CRect & BringToBounds)
{
	// if caret position is inside bounds, it doesn't need to change
	CPoint CaretPos = PositionToPoint(textpos);
	CPoint EndPos =  PositionToPoint(endpos);

	if (CaretPos.y < AllowedBounds.top)
	{
		// bring to BringToBounds.top
		DoVScroll(CaretPos.y - BringToBounds.top);
	}
	else if (EndPos.y > AllowedBounds.bottom)
	{
		int vscroll = EndPos.y - AllowedBounds.bottom;
		if (CaretPos.y - vscroll < BringToBounds.bottom)
		{
			vscroll = CaretPos.y - BringToBounds.bottom;
		}
		DoVScroll(vscroll);
	}
	if (CaretPos.x < AllowedBounds.left)
	{
		DoHScroll(CaretPos.x - BringToBounds.left);
	}
	else if(EndPos.x > AllowedBounds.right)
	{
		int hscroll = EndPos.x - AllowedBounds.right;
		if (CaretPos.x - hscroll < AllowedBounds.left)
		{
			hscroll = CaretPos.x - BringToBounds.left;
		}
		DoHScroll(hscroll);
	}
	CreateAndShowCaret();
}

void CBinaryCompareView::MakePositionVisible(LONGLONG pos)
{
	BringPositionsToBounds(pos, pos, m_VisibleRect, m_VisibleRect);
}

void CBinaryCompareView::MakePositionCentered(LONGLONG pos)
{
	BringPositionsToBounds(pos, pos, m_VisibleRect, m_PreferredRect);
}

void CBinaryCompareView::InvalidateRange(LONGLONG begin, LONGLONG end)
{
	ASSERT(end >= begin);
	CRect r;
	if (0) TRACE("InvalidateRange((%d, %d), (%d, %d))\n", begin, end);
	int nLinesInView = LinesInView();
	int nCharsInView = CharsInView() + 1;
	int nCharsInScreen = nLinesInView * m_BytesPerLine;

	if (begin == end
		|| end < m_ScreenFilePos
		|| begin > m_ScreenFilePos + nCharsInScreen)
	{
		return;
	}

	begin -= m_ScreenFilePos;
	if (begin < 0)
	{
		begin = 0;
	}

	end -= m_ScreenFilePos;
	if (end > nCharsInScreen)
	{
		end = nCharsInScreen;
	}

	int BeginLine = int(begin) / m_BytesPerLine;
	int BeginByte = (int(begin) % m_BytesPerLine) & -m_WordSize;

	int EndLine = int(end) / m_BytesPerLine;
	int EndByte = ((int(end) % m_BytesPerLine) + m_WordSize - 1) & -m_WordSize;

	if (BeginLine == EndLine)
	{
		r.top = BeginLine * LineHeight();
		r.bottom = r.top + LineHeight();
		if (EndByte <= 0 || BeginByte > m_BytesPerLine)
		{
			return;
		}
		if (BeginByte < 0)
		{
			BeginByte = 0;
		}
		if (EndByte > m_BytesPerLine)
		{
			EndByte = m_BytesPerLine;
		}
		r.left = (m_AddressMarginWidth + 2 + BeginByte * 2 + BeginByte / m_WordSize) * CharWidth();
		r.right = (m_AddressMarginWidth + 2 + EndByte * 2 + EndByte / m_WordSize) * CharWidth() + m_FontMetric.tmOverhang;
		InvalidateRect( & r);
	}
	else
	{
		if (BeginLine >= 0 && BeginByte <= m_BytesPerLine)
		{
			if (BeginByte < 0)
			{
				BeginByte = 0;
			}
			r.top = BeginLine * LineHeight();
			r.bottom = r.top + LineHeight();
			r.left = (m_AddressMarginWidth + 2 + BeginByte * 2 + BeginByte / m_WordSize) * CharWidth();
			r.right = (m_AddressMarginWidth + 2 + nCharsInView + 1) * CharWidth() + m_FontMetric.tmOverhang;
			InvalidateRect( & r);
		}
		if (EndLine <= nLinesInView + 2 && EndByte > 0)
		{
			if (EndByte > m_BytesPerLine + 1)
			{
				EndByte = m_BytesPerLine + 1;
			}
			r.top = EndLine * LineHeight();
			r.bottom = r.top + LineHeight();
			r.right = (m_AddressMarginWidth + 2 + EndByte * 2 + EndByte / m_WordSize) * CharWidth() + m_FontMetric.tmOverhang;
			r.left = 0;
			InvalidateRect( & r);
		}
		if (EndLine > nLinesInView + 2)
		{
			EndLine = nLinesInView + 2;
		}
		if (EndLine > BeginLine + 1)
		{
			r.left = 0;
			r.right = (nCharsInView + 1) * CharWidth() + m_FontMetric.tmOverhang;
			r.top = (BeginLine + 1) * LineHeight();
			r.bottom = EndLine * LineHeight();
			InvalidateRect( & r);
		}
	}
}

CPoint CBinaryCompareView::PositionToPoint(ULONGLONG pos)
{
	pos -= m_ScreenFilePos;
	CPoint p;
	LONGLONG line = pos / unsigned(m_BytesPerLine);
	if (line > SHRT_MAX)
	{
		p.y = SHRT_MAX;
	}
	if (line < SHRT_MIN)
	{
		p.y = SHRT_MIN;
	}
	else
	{
		p.y = int(line);
	}
	p.x = unsigned(pos) % m_BytesPerLine;
	return p;
}

void CBinaryCompareView::OnSize(UINT nType, int cx, int cy)
{
	TRACE("CBinaryCompareView::OnSize\n");
	CView::OnSize(nType, cx, cy);

	m_VisibleRect.top = 0;
	m_VisibleRect.bottom = cy / LineHeight() - 1;
	m_PreferredRect.bottom = m_VisibleRect.bottom / 4;
	m_PreferredRect.top = m_VisibleRect.bottom - m_VisibleRect.bottom / 4;

	m_VisibleRect.right =  cx / CharWidth();
	m_VisibleRect.left = 0;
	m_PreferredRect.left = m_VisibleRect.right / 3;
	m_PreferredRect.right = m_VisibleRect.right - m_VisibleRect.right / 3;

	UpdateVScrollBar();
	UpdateHScrollBar();
}

BOOL CBinaryCompareView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
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

void CBinaryCompareView::OnKillFocus(CWnd* pNewWnd)
{
	DestroyCaret();
	m_LButtonDown = false;
	CView::OnKillFocus(pNewWnd);
}

void CBinaryCompareView::OnCaptureChanged(CWnd *pWnd)
{
	if (pWnd != this)
	{
		m_TrackingSelection = false;
		m_LButtonDown = false;
	}

	CView::OnCaptureChanged(pWnd);
}

void CBinaryCompareView::OnLButtonDown(UINT nFlags, CPoint point)
{
	TRACE("CDiffFileView::OnLButtonDown\n");
	CView::OnLButtonDown(nFlags, point);
	m_LButtonDown = true;

	// if the address margin is clicked, the whole line is selected
	point.x -= (m_AddressMarginWidth + 2) * CharWidth();
	LONGLONG Addr = point.y / LineHeight() * m_BytesPerLine + m_ScreenFilePos;

	int flags = SetPositionMakeVisible;
	if (point.x < 0)
	{
		if (0 == (nFlags & MK_SHIFT))
		{
			SetCaretPosition(Addr, SetPositionCancelSelection);
		}
		if (GetDocument()->m_SelectionAnchor <= Addr)
		{
			Addr += m_BytesPerLine;
		}
	}
	else
	{
		point.x = (point.x + CharWidth() / 2) / CharWidth();
		point.x -= point.x / (m_WordSize * 2 + 1);
		Addr += (point.x / 2) & -m_WordSize;
		if (0 == (nFlags & MK_SHIFT))
		{
			flags |= SetPositionCancelSelection;
		}
		if (nFlags & MK_CONTROL)
		{
			flags |= SetWordSelectionMode;
		}
	}
	SetCaretPosition(Addr, flags);
}

void CBinaryCompareView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_LButtonDown = false;

	if (m_TrackingSelection)
	{
		ReleaseCapture();
	}
	CView::OnLButtonUp(nFlags, point);
}

void CBinaryCompareView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_LButtonDown && ! m_TrackingSelection)
	{
		m_TrackingSelection = true;
		SetCapture();
	}
	CView::OnMouseMove(nFlags, point);
	if (m_TrackingSelection)
	{
		point.x -= (m_AddressMarginWidth + 2) * CharWidth();
		LONGLONG Addr = point.y / LineHeight() * m_BytesPerLine + m_ScreenFilePos;

		if (point.x < 0)
		{
			if (GetDocument()->m_SelectionAnchor <= Addr)
			{
				Addr += m_BytesPerLine;
			}
		}
		else
		{
			point.x = (point.x + CharWidth() / 2) / CharWidth();
			point.x -= point.x / (m_WordSize * 2 + 1);
			Addr += (point.x / 2) & -m_WordSize;
		}
		SetCaretPosition(Addr, SetPositionMakeVisible);
	}
}

void CBinaryCompareView::OnWordsize1byte()
{
	if (1 == m_WordSize)
	{
		return;
	}
	m_WordSize = 1;
	OnMetricsChange();
}

void CBinaryCompareView::OnUpdateWordsize1byte(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_WordSize == 1);
}

void CBinaryCompareView::OnWordsize2bytes()
{
	if (2 == m_WordSize)
	{
		return;
	}
	m_WordSize = 2;
	OnMetricsChange();
}

void CBinaryCompareView::OnUpdateWordsize2bytes(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_WordSize == 2);
}

void CBinaryCompareView::OnWordsize4bytes()
{
	if (4 == m_WordSize)
	{
		return;
	}
	m_WordSize = 4;
	OnMetricsChange();
}

void CBinaryCompareView::OnUpdateWordsize4bytes(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_WordSize == 4);
}

void CBinaryCompareView::OnWordsize8bytes()
{
	if (8 == m_WordSize)
	{
		return;
	}
	m_WordSize = 8;
	OnMetricsChange();
}

void CBinaryCompareView::OnUpdateWordsize8bytes(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_WordSize == 8);
}

void CBinaryCompareView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// make sure window is active
	GetParentFrame()->ActivateFrame();

	CMenu menu;
	if (menu.LoadMenu(IDR_BINARYDIFF_CONTEXT))
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

void CBinaryCompareView::OnBindiffShowfirstfile()
{
	FilePair * pPair = GetDocument()->GetFilePair();
	if (NULL != pPair->pFirstFile
		&& NULL != pPair->pSecondFile)
	{
		m_bShowSecondFile = FALSE;
		Invalidate();
	}
}

void CBinaryCompareView::OnUpdateBindiffShowfirstfile(CCmdUI *pCmdUI)
{
	FilePair * pPair = GetDocument()->GetFilePair();
	if (NULL != pPair->pFirstFile
		&& NULL != pPair->pSecondFile)
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetRadio( ! m_bShowSecondFile);
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetRadio(NULL != pPair->pFirstFile);
	}
}

void CBinaryCompareView::OnBindiffShow2ndfile()
{
	FilePair * pPair = GetDocument()->GetFilePair();
	if (NULL != pPair->pFirstFile
		&& NULL != pPair->pSecondFile)
	{
		m_bShowSecondFile = TRUE;
		Invalidate();
	}
}

void CBinaryCompareView::OnUpdateBindiffShow2ndfile(CCmdUI *pCmdUI)
{
	FilePair * pPair = GetDocument()->GetFilePair();
	if (NULL != pPair->pFirstFile
		&& NULL != pPair->pSecondFile)
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetRadio(m_bShowSecondFile);
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetRadio(NULL != pPair->pSecondFile);
	}
}

void CBinaryCompareView::OnSetFocus(CWnd* pOldWnd)
{
	CView::OnSetFocus(pOldWnd);

	CreateAndShowCaret();
}

UINT AFX_CDECL CBinaryCompareView::_FindDataProc(PVOID param)
{
	CDifferenceProgressDialog * pContext = static_cast<CDifferenceProgressDialog *>(param);
	return pContext->m_pView->FindDataProc(pContext);
}

UINT CBinaryCompareView::FindDataProc(class CDifferenceProgressDialog * pContext)
{
	CBinaryCompareDoc * pDoc = GetDocument();

	FilePair * pFilePair = pDoc->GetFilePair();
	if (NULL == pFilePair
		|| NULL == pFilePair->pFirstFile
		|| NULL == pFilePair->pSecondFile)
	{
		return 0;
	}
	int const BufferSize = 0x10000;
	UCHAR * File1Buffer = new UCHAR[BufferSize];
	UCHAR * File2Buffer = new UCHAR[BufferSize];
	long File1BufFilled = 0;
	long File2BufFilled = 0;
	long File1BufIndex = 0;
	long File2BufIndex = 0;
	BOOL FindingFirstDifference = FALSE;

	LONGLONG Addr = pContext->BeginAddr;

	if (Addr > pContext->EndAddr)
	{
		FindingFirstDifference = TRUE;
		// search backward
		while (! pContext->m_StopSearch
				&& Addr > pContext->EndAddr)
		{
			if (File1BufIndex <= 0)
			{
				ULONG ToRead = BufferSize;
				if (Addr - pContext->EndAddr < BufferSize)
				{
					ToRead = ULONG(Addr - pContext->EndAddr);
				}

				File1BufFilled = pFilePair->pFirstFile->GetFileData(Addr - ToRead,
									File1Buffer, ToRead);
				File1BufIndex = File1BufFilled;
			}

			if (File2BufIndex <= 0)
			{
				ULONG ToRead = BufferSize;
				if (Addr - pContext->EndAddr < BufferSize)
				{
					ToRead = ULONG(Addr - pContext->EndAddr);
				}

				File2BufFilled = pFilePair->pSecondFile->GetFileData(Addr - ToRead,
									File2Buffer, ToRead);
				File2BufIndex = File2BufFilled;

				int PercentCompleted = int((pContext->BeginAddr - Addr) * 100
											/ (pContext->BeginAddr - pContext->EndAddr));

				if (PercentCompleted != pContext->m_PercentCompleted
					|| pContext->m_hWnd != NULL)
				{
					pContext->m_PercentCompleted = PercentCompleted;
					::PostMessage(pContext->m_hWnd, WM_KICKIDLE, 0, 0);
				}
			}

			if (0 == File1BufIndex
				|| 0 == File2BufIndex)
			{
				break;
			}

			File1BufIndex--;
			File2BufIndex--;

			if (FindingFirstDifference)
			{
				if (File1Buffer[File1BufIndex] != File2Buffer[File2BufIndex])
				{
					FindingFirstDifference = FALSE;
				}
			}
			else
			{
				if (File1Buffer[File1BufIndex] == File2Buffer[File2BufIndex])
				{
					break;
				}
			}
			Addr--;
		}
		pContext->BeginAddr = Addr;
	}
	else
	{
		// search forward
		while (! pContext->m_StopSearch
				&& Addr < pContext->EndAddr)
		{
			if (File1BufIndex >= File1BufFilled)
			{
				ULONG ToRead = BufferSize;
				if (pContext->EndAddr - Addr < BufferSize)
				{
					ToRead = ULONG(pContext->EndAddr - Addr);
				}

				File1BufFilled = pFilePair->pFirstFile->GetFileData(Addr,
									File1Buffer, ToRead);
				File1BufIndex = 0;
			}

			if (File2BufIndex >= File2BufFilled)
			{
				ULONG ToRead = BufferSize;
				if (pContext->EndAddr - Addr < BufferSize)
				{
					ToRead = ULONG(pContext->EndAddr - Addr);
				}

				File2BufFilled = pFilePair->pSecondFile->GetFileData(Addr,
									File2Buffer, ToRead);
				File2BufIndex = 0;

				int PercentCompleted = int((Addr - pContext->BeginAddr) * 100
											/ (pContext->EndAddr - pContext->BeginAddr));
				if (PercentCompleted != pContext->m_PercentCompleted
					|| pContext->m_hWnd != NULL)
				{
					pContext->m_PercentCompleted = PercentCompleted;
					::PostMessage(pContext->m_hWnd, WM_KICKIDLE, 0, 0);
				}
			}

			if (File1BufIndex >= File1BufFilled
				|| File2BufIndex >= File2BufFilled)
			{
				break;
			}

			if (FindingFirstDifference)
			{
				if (File1Buffer[File1BufIndex] != File2Buffer[File2BufIndex])
				{
					break;
				}
			}
			else
			{
				if (File1Buffer[File1BufIndex] == File2Buffer[File2BufIndex])
				{
					FindingFirstDifference = TRUE;
				}
			}
			File1BufIndex++;
			File2BufIndex++;
			Addr++;
		}
		pContext->BeginAddr = Addr;
	}
	delete[] File1Buffer;
	delete[] File2Buffer;
	pContext->m_SearchCompleted = TRUE;

	if (NULL != pContext->m_hWnd)
	{
		::PostMessage(pContext->m_hWnd, WM_COMMAND, IDOK, 0);
	}
	return 0;
}

void CBinaryCompareView::OnEditGotonextdiff()
{
	CBinaryCompareDoc * pDoc = GetDocument();
	CDifferenceProgressDialog dlg;
	dlg.m_pView = this;
	dlg.BeginAddr = pDoc->m_CaretPos;
	dlg.EndAddr = pDoc->GetFileSize();

	CWinThread Thread(_FindDataProc, & dlg);
	Thread.m_bAutoDelete = FALSE;

	Thread.CreateThread();

	int result = IDOK;
	if (WAIT_TIMEOUT == WaitForSingleObject(Thread.m_hThread, 200))
	{
		result = dlg.DoModal();
		WaitForSingleObject(Thread.m_hThread, INFINITE);
	}
	if (IDOK == result)
	{
		SetCaretPosition(dlg.BeginAddr,
						SetPositionCancelSelection | SetPositionMakeVisible);
	}
}

void CBinaryCompareView::OnEditGotoprevdiff()
{
	CBinaryCompareDoc * pDoc = GetDocument();
	CDifferenceProgressDialog dlg;
	dlg.m_pView = this;
	dlg.BeginAddr = pDoc->m_CaretPos;
	dlg.EndAddr = 0;

	CWinThread Thread(_FindDataProc, & dlg);
	Thread.m_bAutoDelete = FALSE;

	Thread.CreateThread();

	int result = IDOK;
	if (WAIT_TIMEOUT == WaitForSingleObject(Thread.m_hThread, 200))
	{
		result = dlg.DoModal();
		WaitForSingleObject(Thread.m_hThread, INFINITE);
	}
	if (IDOK == result)
	{
		SetCaretPosition(dlg.BeginAddr,
						SetPositionCancelSelection | SetPositionMakeVisible);
	}
}
