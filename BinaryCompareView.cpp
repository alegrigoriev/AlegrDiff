// BinaryCompareView.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "BinaryCompareView.h"
#include "BinaryCompareDoc.h"
#include "ChildFrm.h"
#include "DifferenceProgressDialog.h"
#include <afxpriv.h>
#include "GdiObjectSave.h"
#include "BinaryGoToDlg.h"

#include ".\binarycompareview.h"

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
	, m_bCaretOnChars(FALSE)
	, m_AddressMarginWidth(0)
	, m_MaxAddressChars(0)
	, m_PaneWithFocus(0)
{
	m_FontMetric.tmAveCharWidth = 1;
	m_FontMetric.tmExternalLeading = 1;
	m_FontMetric.tmHeight = 1;
	m_NumberOfPanes = GetApp()->m_NumberOfPanes;
}

CBinaryCompareView::~CBinaryCompareView()
{
}

BEGIN_MESSAGE_MAP(CBinaryCompareView, BaseClass)
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
	ON_COMMAND(ID_VIEW_LESSBYTESINLINE, OnViewLessbytesinline)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LESSBYTESINLINE, OnUpdateViewLessbytesinline)
	ON_COMMAND(ID_VIEW_MOREBYTESINLINE, OnViewMorebytesinline)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOREBYTESINLINE, OnUpdateViewMorebytesinline)

	ON_COMMAND(IDC_VIEW_SIDE_BY_SIDE, OnViewSideBySide)
	ON_UPDATE_COMMAND_UI(IDC_VIEW_SIDE_BY_SIDE, OnUpdateViewSideBySide)
	ON_COMMAND(ID_EDIT_GOTOLINE, OnEditGoto)
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
	FilePair * pFilePair = GetFilePair();
	if (NULL == pFilePair)
	{
		return;
	}

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

	CGdiObjectSaveT<CFont> OldFont(pDC, pDC->SelectObject( & pApp->m_NormalFont));

	pDC->SetTextAlign(pDC->GetTextAlign() | TA_UPDATECP);

	DWORD TextColor = pApp->m_NormalTextColor;
	DWORD OtherColor, AlternateColor;
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

	int const PaneWidth = GetPaneWidth();

	CGdiObjectSave OldPen(pDC, pDC->SelectStockObject(BLACK_PEN));

	for (int pane = 0, x = m_AddressMarginWidth - 1; pane < m_NumberOfPanes;
		pane++, x += PaneWidth)
	{
		pDC->MoveTo(x, ur.top);
		pDC->LineTo(x, ur.bottom);
	}

	FileItem::Lock();

	for (int CurrentY = ur.top - ur.top % LineHeight(); CurrentY < ur.bottom; CurrentY += LineHeight())
	{
		LONGLONG CurrentAddr = m_ScreenFilePos + CurrentY / LineHeight() * int(m_BytesPerLine);
		if (CurrentAddr > pDoc->GetFileSize())
		{
			break;
		}

		TCHAR buf[256];

		int len = _stprintf_s(buf, countof(buf), _T("%0*I64X "), m_MaxAddressChars, CurrentAddr);

		pDC->MoveTo(0, CurrentY);
		pDC->SetBkColor(pApp->m_TextBackgroundColor);
		pDC->SetTextColor(pApp->m_NormalTextColor);
		pDC->TextOut(0, CurrentY, buf, len);

		for (int pane = 0; pane < m_NumberOfPanes; pane++)
		{
			FileItem * pFile;
			FileItem * pOtherFile;

			pDC->MoveTo(m_AddressMarginWidth + pane * PaneWidth, CurrentY);

			if (((1 == m_NumberOfPanes && m_bShowSecondFile)
					|| (pane >= 1))
				|| !pFilePair->pFirstFile->HasContents())
			{
				pFile = pFilePair->pSecondFile;

				pOtherFile = pFilePair->pFirstFile;
				if (!pOtherFile->HasContents())
				{
					pOtherFile = NULL;
				}

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

			if ((NULL == pFile || CurrentAddr >= pFile->GetFileLength())
				&& (NULL == pOtherFile || CurrentAddr >= pOtherFile->GetFileLength()))
			{
				break;
			}

			int PosDrawn = - m_FirstPosSeen;
			int MaxPosToDraw = PaneWidth / CharWidth();
			if (pane == m_NumberOfPanes - 1)
			{
				MaxPosToDraw++;
			}

			pDC->MoveTo(m_AddressMarginWidth + PaneWidth * pane, CurrentY);

			for (unsigned offset = 0;
				offset < m_BytesPerLine;
				offset += m_WordSize)
			{
				for (int ByteNum = m_WordSize - 1; ByteNum >= 0; ByteNum--)
				{
					LONGLONG ByteOffset = CurrentAddr + offset + ByteNum;
					UCHAR byte1 = 0;
					UCHAR byte2 = 0;

					BOOL Byte1Valid = FALSE;

					if (NULL != pFile
						&& 1 == pFile->GetFileData(ByteOffset, & byte1, 1))
					{
						Byte1Valid = TRUE;
					}

					BOOL Byte2Valid = FALSE;

					if (NULL != pOtherFile
						&& 1 == pOtherFile->GetFileData(ByteOffset, & byte2, 1))
					{
						Byte2Valid = TRUE;
					}

					DWORD color = TextColor;

					if (Byte1Valid)
					{
						len = _stprintf_s(buf, countof(buf), _T("%02X"), byte1);

						if (NULL != pOtherFile && (! Byte2Valid
								|| byte1 != byte2))
						{
							color = OtherColor;
						}
					}
					else if (1 == m_NumberOfPanes
							&& NULL != pOtherFile && Byte2Valid)
					{
						len = _stprintf_s(buf, countof(buf), _T("%02X"), byte2);
						color = AlternateColor;
					}
					else
					{
						buf[0] = ' ';
						buf[1] = ' ';
						buf[2] = 0;
						len = 2;
					}

					DWORD BackgroundColor = pApp->m_TextBackgroundColor;

					if ((1 == m_NumberOfPanes
							|| pane == m_PaneWithFocus)
						&& ByteOffset < SelEnd
						&& ByteOffset >= SelBegin)
					{
						color = pApp->m_SelectedTextColor;
						BackgroundColor = 0x000000;
					}

					if (ByteNum == 0)
					{
						buf[2] = ' ';
						buf[3] = 0;
						len++;

						if (offset + m_WordSize == m_BytesPerLine)
						{
							_tcscat_s(buf, countof(buf), _T("  "));
							len += 2;
						}
					}

					TCHAR const * pDrawnBuf = buf;
					if (PosDrawn < 0)
					{
						if (PosDrawn + len > 0)
						{
							pDrawnBuf += -PosDrawn;
							len = PosDrawn + len;
							PosDrawn = len;
						}
						else
						{
							PosDrawn += len;
							len = 0;
						}
					}
					else
					{
						PosDrawn += len;
					}

					if (PosDrawn > MaxPosToDraw)
					{
						len -= PosDrawn - MaxPosToDraw;
					}

					if (len > 0)
					{
						pDC->SetBkColor(BackgroundColor);
						pDC->SetTextColor(color);
						pDC->TextOut(0, 0, pDrawnBuf, len);
					}
				}
			}

			for (unsigned offset = 0;
				offset < m_BytesPerLine;
				offset ++)
			{
				LONGLONG ByteOffset = CurrentAddr + offset;
				UCHAR byte1 = 0;
				UCHAR byte2 = 0;

				BOOL Byte1Valid = FALSE;

				if (NULL != pFile
					&& 1 == pFile->GetFileData(ByteOffset, & byte1, 1))
				{
					Byte1Valid = TRUE;
				}

				BOOL Byte2Valid = FALSE;

				if (NULL != pOtherFile
					&& 1 == pOtherFile->GetFileData(ByteOffset, & byte2, 1))
				{
					Byte2Valid = TRUE;
				}

				UCHAR CurrChar = 0;

				DWORD color = TextColor;
				if (Byte1Valid)
				{
					CurrChar = byte1;

					if (NULL != pOtherFile && (! Byte2Valid
							|| byte1 != byte2))
					{
						color = OtherColor;
					}
				}
				else if (1 == m_NumberOfPanes
						&& NULL != pOtherFile && Byte2Valid)
				{
					CurrChar = byte2;

					color = AlternateColor;
				}
				else
				{
					CurrChar = ' ';
				}

				DWORD BackgroundColor = pApp->m_TextBackgroundColor;
				if ((1 == m_NumberOfPanes
						|| pane == m_PaneWithFocus)
					&& ByteOffset < SelEnd
					&& ByteOffset >= SelBegin)
				{
					color = pApp->m_SelectedTextColor;
					BackgroundColor = 0x000000;
				}

				// convert the byte to unicode
				char tmp = CurrChar;
				if ( ! isprint(CurrChar)
					|| 1 != mbtowc(buf, & tmp, 1))
				{
					buf[0] = '.';
					buf[1] = 0;
				}

				int chars = 1;

				if (PosDrawn < 0)
				{
					if (PosDrawn + chars > 0)
					{
						PosDrawn += chars;
						chars = PosDrawn;
					}
					else
					{
						PosDrawn += chars;
						chars = 0;
					}
				}
				else
				{
					PosDrawn += chars;
				}

				if (PosDrawn > MaxPosToDraw)
				{
					chars -= PosDrawn - MaxPosToDraw;
				}

				if (chars > 0)
				{
					pDC->SetBkColor(BackgroundColor);
					pDC->SetTextColor(color);
					pDC->TextOut(0, 0, buf, chars);
				}
			}
		}
		CurrentAddr += m_BytesPerLine;
	}

	FileItem::Unlock();

}


// CBinaryCompareView diagnostics

#ifdef _DEBUG
void CBinaryCompareView::AssertValid() const
{
	BaseClass::AssertValid();
}

void CBinaryCompareView::Dump(CDumpContext& dc) const
{
	BaseClass::Dump(dc);
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

	BinaryFilePair* pFilePair = GetFilePair();

	if (pFilePair != nullptr && !pFilePair->CanCompare())
	{
		m_NumberOfPanes = 1;
	}

	CRect cr;
	GetClientRect( & cr);

	CGdiObjectSaveT<CFont> OldFont(wdc, wdc.SelectObject(pFont));
	wdc.GetTextMetrics( & m_FontMetric);

	CBinaryCompareDoc * pDoc = GetDocument();

	LONGLONG FileSize = pDoc->GetFileSize();

	if (FileSize >= 0x00FFFFFFFFFFFFFFi64)
	{
		m_MaxAddressChars = 16;
	}
	else if (FileSize >= 0x0000FFFFFFFFFFFFi64)
	{
		m_MaxAddressChars = 14;
	}
	else if (FileSize >= 0x000000FFFFFFFFFFi64)
	{
		m_MaxAddressChars = 12;
	}
	else if (FileSize >= 0x00000000FFFFFFFFi64)
	{
		m_MaxAddressChars = 10;
	}
	else
	{
		m_MaxAddressChars = 8;
	}

	m_AddressMarginWidth = (m_MaxAddressChars + 1) * CharWidth() + 1;

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
		//TRACE("CBinaryCompareView::CreateAndShowCaret - No Focus\n");
		return;
	}
	if (0 == GetDocument()->GetFileSize())
	{
		//TRACE("CBinaryCompareView::CreateAndShowCaret - 0 == GetDocument()->GetFileSize()\n");
		return;
	}

	CPoint p;
	int ByteOnTheLine = (ULONG(pDoc->m_CaretPos - m_ScreenFilePos) % m_BytesPerLine)
						& -m_WordSize;

	int const PaneWidth = GetPaneWidth();

	if (m_bCaretOnChars)
	{
		p.x = (ByteOnTheLine + m_BytesPerLine * 2 + m_BytesPerLine / m_WordSize
				+ 2 - m_FirstPosSeen) * CharWidth();
	}
	else
	{
		p.x = (ByteOnTheLine * 2 + ByteOnTheLine / m_WordSize
				- m_FirstPosSeen) * CharWidth();
	}


	if (p.x >= 0
		&& p.x < PaneWidth - 1
		&& pDoc->m_CaretPos >= m_ScreenFilePos
		&& pDoc->m_CaretPos <= m_ScreenFilePos + (LinesInView() + 1) * int(m_BytesPerLine))
	{

		p.x += m_AddressMarginWidth + m_PaneWithFocus * PaneWidth;

		p.y = ULONG(pDoc->m_CaretPos - m_ScreenFilePos) / m_BytesPerLine * LineHeight();

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
	BaseClass::OnInitialUpdate();

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

	return BaseClass::PreCreateWindow(cs);
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

	case VK_TAB:
		TRACE("VK_TAB\n");

		if ( ! m_TrackingSelection)
		{
			if ( ! m_bCaretOnChars)
			{
				m_bCaretOnChars = TRUE;
			}
			else
			{
				m_bCaretOnChars = FALSE;
				// go to next pane
				int NewPaneWithFocus = m_PaneWithFocus + 1;

				if (NewPaneWithFocus >= m_NumberOfPanes)
				{
					NewPaneWithFocus = 0;
				}
				SetFocusPane(NewPaneWithFocus);
			}
			CreateAndShowCaret();
		}
		break;
	}
	BaseClass::OnKeyDown(nChar, nRepCnt, nFlags);
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
		// not actually generated with standard scrollbar
	case SB_TOP:
		TRACE("OnVScroll SB_TOP, nPos=%d\n", nPos);
		VScrollToTheAddr(0);
		break;

		// not actually generated with standard scrollbar
	case SB_BOTTOM:
		TRACE("OnVScroll SB_BOTTOM, nPos=%d\n", nPos);
		VScrollToTheAddr(GetDocument()->GetFileSize() - nLinesInView * int(m_BytesPerLine));
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
		if (nPos == m_VScrollInfo.nMax - (m_VScrollInfo.nPage - 1))
		{
			// scroll to the end of file
			VScrollToTheAddr(GetDocument()->GetFileSize() - nLinesInView * int(m_BytesPerLine));
		}
		else
		{
			VScrollToTheAddr(nPos * m_ScrollDataScale * LONGLONG(m_BytesPerLine));
		}
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
	int nCharsInPane = (GetPaneWidth() - 1) / CharWidth();

	int CharsToScroll = GetMaxChars() - nCharsInPane;
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

	GetClientRect(cr);

	CRect ScrollRect(cr);

	int PaneWidth = GetPaneWidth();

	int ndx = (nPos - m_FirstPosSeen) * CharWidth();
	for (int pane = 0; pane < m_NumberOfPanes; pane++)
	{
		ScrollRect.left = m_AddressMarginWidth + pane * PaneWidth;
		if (pane != m_NumberOfPanes - 1)
		{
			ScrollRect.right = ScrollRect.left + PaneWidth - 1;
		}
		else
		{
			ScrollRect.right = cr.right;
		}

		if (ScrollRect.left < ScrollRect.right)
		{
			ScrollWindowEx( -ndx, 0, & ScrollRect, & ScrollRect, NULL, NULL,
							SW_INVALIDATE | SW_ERASE);
		}
	}

	m_FirstPosSeen = nPos;

	UpdateHScrollBar();
	CreateAndShowCaret();
}

	// > 0 - scroll up (to see lines toward end),
	// < 0 - scroll down (to see lines to the begin of the file)
void CBinaryCompareView::DoVScroll(LONGLONG nLinesToScroll)
{
	VScrollToTheAddr(m_ScreenFilePos + nLinesToScroll * int(m_BytesPerLine));
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

	if (Addr > FileSize - (BytesPerScreen - int(m_BytesPerLine)))
	{
		Addr = FileSize - (BytesPerScreen - int(m_BytesPerLine));
	}

	LONGLONG nOffset = Addr - m_ScreenFilePos;
	if (nOffset <= BytesPerScreen && nOffset >= -BytesPerScreen)
	{
		// need to scroll the view
		int ndy = LONG(nOffset) / int(m_BytesPerLine);
		if (0 == ndy)
		{
			return;
		}
		m_ScreenFilePos += ndy * int(m_BytesPerLine);

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
		m_ScreenFilePos = Addr - Addr % int(m_BytesPerLine);
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

	m_VScrollInfo.cbSize = sizeof m_VScrollInfo;
	m_VScrollInfo.nMin = 0;
	// TODO: scale nMax and nPos
	LONGLONG nLines = GetDocument()->GetFileSize() / m_BytesPerLine;
	LONGLONG nCurLine = m_ScreenFilePos / m_BytesPerLine;
	m_VScrollInfo.nPage = LinesInView();
	// limit the parameters to 16 bit
	m_ScrollDataScale = 1;
	while (nLines > SHRT_MAX)
	{
		nLines >>= 1;
		nCurLine >>= 1;
		m_VScrollInfo.nPage >>= 1;
		m_ScrollDataScale <<= 1;
	}
	if (0 == m_VScrollInfo.nPage)
	{
		m_VScrollInfo.nPage = 1;
	}
	m_VScrollInfo.nMax = int(nLines);
	m_VScrollInfo.nPos = int(nCurLine);

	TRACE("CBinaryCompareView::UpdateVScrollBar pos = %d\n", m_VScrollInfo.nPos);
	m_VScrollInfo.nTrackPos = 0;
	m_VScrollInfo.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

	SetScrollInfo(SB_VERT, & m_VScrollInfo);
	UpdateVisibleRect();
}

int CBinaryCompareView::GetMaxChars() const
{
	return 4 + m_BytesPerLine * 3
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
	sci.nPage = (GetPaneWidth() - 1) / CharWidth();

	if (m_FirstPosSeen > int(sci.nMax - sci.nPage))
	{
		m_FirstPosSeen = 0;
		Invalidate();
	}

	sci.nPos = m_FirstPosSeen;
	sci.nTrackPos = 0;
	sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

	SetScrollInfo(SB_HORZ, & sci);
	UpdateVisibleRect();
}

void CBinaryCompareView::SetCaretPosition(LONGLONG Addr, int flags)
{
	CBinaryCompareDoc * pDoc = GetDocument();
	pDoc->SetCaretPosition(Addr, flags & ~(SetPositionMakeVisible | SetPositionMakeCentered));

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
	else if (lHint == CBinaryCompareDoc::UpdateMakeCaretCentered)
	{
		MakeCaretCentered();
	}
	else if (lHint == CBinaryCompareDoc::UpdateMakeCaretVisible)
	{
		MakeCaretVisible();
	}
	else if (lHint == UpdateViewsFilePairChanged)
	{
		FilePairChangedArg * pArg = dynamic_cast<FilePairChangedArg *>(pHint);
		if (NULL != pArg
			&& pArg->m_pPair == pDoc->GetFilePair())
		{
			OnMetricsChange();
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
	LONGLONGPoint CaretPos = PositionToPoint(textpos);
	LONGLONGPoint EndPos =  PositionToPoint(endpos);

	if (CaretPos.y < AllowedBounds.top)
	{
		// bring to BringToBounds.top
		DoVScroll(CaretPos.y - BringToBounds.top);
	}
	else if (EndPos.y > AllowedBounds.bottom)
	{
		LONGLONG vscroll = EndPos.y - AllowedBounds.bottom;
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

int CBinaryCompareView::GetXCoordOfHexData(int Pane, int LineOffset)
{
	if (LineOffset > int(m_BytesPerLine))
	{
		LineOffset = m_BytesPerLine;
	}
	int x = (LineOffset * 2 + LineOffset / m_WordSize - m_FirstPosSeen) * CharWidth();
	if (x < 0)
	{
		x = 0;
	}
	int PaneWidth = GetPaneWidth();
	if (x >= PaneWidth)
	{
		x = PaneWidth - 1;
	}
	return m_AddressMarginWidth + x + PaneWidth * Pane;
}

int CBinaryCompareView::GetXCoordOfTextData(int Pane, int LineOffset)
{
	if (LineOffset > int(m_BytesPerLine))
	{
		LineOffset = m_BytesPerLine;
	}

	int x = (LineOffset + m_BytesPerLine * 2 + m_BytesPerLine / m_WordSize
				+ 2 - m_FirstPosSeen) * CharWidth();

	if (x < 0)
	{
		x = 0;
	}

	int PaneWidth = GetPaneWidth();
	if (Pane != m_NumberOfPanes - 1
		&& x >= PaneWidth)
	{
		x = PaneWidth - 1;
	}
	return m_AddressMarginWidth + x + PaneWidth * Pane;
}

void CBinaryCompareView::InvalidatePaneLine(int Pane, int line, int BeginOffset, int EndOffset)
{
	if (EndOffset <= 0 || BeginOffset > int(m_BytesPerLine))
	{
		return;
	}

	CRect r;

	r.top = line * LineHeight();
	r.bottom = r.top + LineHeight();
	r.left = GetXCoordOfHexData(Pane, BeginOffset);
	r.right = GetXCoordOfHexData(Pane, EndOffset) + m_FontMetric.tmOverhang;

	InvalidateRect( & r);

	// invalidate text chars
	r.left = GetXCoordOfTextData(Pane, BeginOffset);
	r.right = GetXCoordOfTextData(Pane, EndOffset) + m_FontMetric.tmOverhang;

	InvalidateRect( & r);
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
	unsigned BeginByte = (int(begin) % m_BytesPerLine) & -m_WordSize;

	int EndLine = int(end) / m_BytesPerLine;
	unsigned EndByte = ((int(end) % m_BytesPerLine) + m_WordSize - 1) & -m_WordSize;

	if (EndLine > nLinesInView + 2)
	{
		EndLine = nLinesInView + 2;
	}
	if (BeginLine < -1)
	{
		BeginLine = -1;
	}

	if (BeginLine == EndLine)
	{
		InvalidatePaneLine(m_PaneWithFocus, BeginLine, BeginByte, EndByte);
	}
	else
	{
		if (BeginLine >= 0)
		{
			InvalidatePaneLine(m_PaneWithFocus, BeginLine, BeginByte, nCharsInView);
		}
		if (EndLine <= nLinesInView + 2)
		{
			InvalidatePaneLine(m_PaneWithFocus, EndLine, 0, EndByte);
		}

		for (int line = BeginLine + 1; line < EndLine; line++)
		{
			InvalidatePaneLine(m_PaneWithFocus, line, 0, nCharsInView);
		}
	}
}

CBinaryCompareView::LONGLONGPoint CBinaryCompareView::PositionToPoint(ULONGLONG pos)
{
	pos -= m_ScreenFilePos;
	LONGLONGPoint p;
	p.y = pos / unsigned(m_BytesPerLine);
	p.x = unsigned(pos) & int(m_BytesPerLine - 1);
	return p;
}

void CBinaryCompareView::OnSize(UINT nType, int cx, int cy)
{
	TRACE("CBinaryCompareView::OnSize\n");
	BaseClass::OnSize(nType, cx, cy);

	UpdateVScrollBar();
	UpdateHScrollBar();
}

BOOL CBinaryCompareView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
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
	BaseClass::OnKillFocus(pNewWnd);
}

void CBinaryCompareView::OnCaptureChanged(CWnd *pWnd)
{
	if (pWnd != this)
	{
		m_TrackingSelection = false;
		m_LButtonDown = false;
	}

	BaseClass::OnCaptureChanged(pWnd);
}

void CBinaryCompareView::OnLButtonDown(UINT nFlags, CPoint point)
{
	TRACE("CDiffFileView::OnLButtonDown\n");
	BaseClass::OnLButtonDown(nFlags, point);
	m_LButtonDown = true;

	int pane = PointToPaneNumber(point.x);

	point.x = PointToPaneOffset(point.x);
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
		int x = point.x / CharWidth();
		x = (x / (m_WordSize * 2 + 1)) * m_WordSize;

		// TODO:if the address margin is clicked, the whole line is selected
		if (x >= int(m_BytesPerLine))
		{
			x = (point.x + CharWidth() / 2) / CharWidth() -
				(m_BytesPerLine * 2 + m_BytesPerLine / m_WordSize + 2);

			if (x >= int(m_BytesPerLine))
			{
				x = m_BytesPerLine - 1;
			}

			m_bCaretOnChars = TRUE;
		}
		else
		{
			m_bCaretOnChars = FALSE;
			x &= -m_WordSize;
		}
		if (x < 0)
		{
			x = 0;
		}

		Addr += x;

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
	SetFocusPane(pane);
}

void CBinaryCompareView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_LButtonDown = false;

	if (m_TrackingSelection)
	{
		ReleaseCapture();
	}
	BaseClass::OnLButtonUp(nFlags, point);
}

void CBinaryCompareView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_LButtonDown && ! m_TrackingSelection)
	{
		m_TrackingSelection = true;
		SetCapture();
	}
	BaseClass::OnMouseMove(nFlags, point);
	if (m_TrackingSelection)
	{
		point.x = PointToPaneOffset(point.x);

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
			int x = point.x / CharWidth();

			if (m_bCaretOnChars)
			{
				x = x - (m_BytesPerLine * 2 + m_BytesPerLine / m_WordSize + 2);
			}
			else
			{
				x = (x / (m_WordSize * 2 + 1)) * m_WordSize;
			}

			if (x < 0)
			{
				x = 0;
			}

			if (unsigned(x) > m_BytesPerLine)
			{
				x = m_BytesPerLine;
			}

			Addr += x;
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
	if (GetFilePair()->CanCompare())
	{
		m_bShowSecondFile = FALSE;
		Invalidate();
	}
}

void CBinaryCompareView::OnUpdateBindiffShowfirstfile(CCmdUI *pCmdUI)
{
	FilePair * pPair = GetFilePair();
	if (1 == m_NumberOfPanes)
	{
		if (pPair->CanCompare())
		{
			pCmdUI->Enable(TRUE);
			pCmdUI->SetRadio( ! m_bShowSecondFile);
		}
		else
		{
			pCmdUI->SetRadio(NULL != pPair->pFirstFile);
			pCmdUI->Enable(FALSE);
		}
	}
	else
	{
		pCmdUI->SetRadio(FALSE);
		pCmdUI->Enable(FALSE);
	}
}

void CBinaryCompareView::OnBindiffShow2ndfile()
{
	if (GetFilePair()->CanCompare())
	{
		m_bShowSecondFile = TRUE;
		Invalidate();
	}
}

void CBinaryCompareView::OnUpdateBindiffShow2ndfile(CCmdUI *pCmdUI)
{
	FilePair * pPair = GetFilePair();
	if (1 == m_NumberOfPanes)
	{
		if (pPair->CanCompare())
		{
			pCmdUI->Enable(TRUE);
			pCmdUI->SetRadio(m_bShowSecondFile);
		}
		else
		{
			pCmdUI->SetRadio(NULL != pPair->pSecondFile);
			pCmdUI->Enable(FALSE);
		}
	}
	else
	{
		pCmdUI->SetRadio(FALSE);
		pCmdUI->Enable(FALSE);
	}
}

void CBinaryCompareView::OnSetFocus(CWnd* pOldWnd)
{
	BaseClass::OnSetFocus(pOldWnd);

	CreateAndShowCaret();
}

void CBinaryCompareView::OnEditGotonextdiff()
{
	CBinaryCompareDoc * pDoc = GetDocument();
	CDifferenceProgressDialog dlg;
	dlg.m_pDoc = pDoc;
	dlg.BeginAddr = pDoc->m_CaretPos;
	dlg.EndAddr = pDoc->GetFileSize();

	INT_PTR result = dlg.DoModalDelay(200);

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
	dlg.m_pDoc = pDoc;
	dlg.BeginAddr = pDoc->m_CaretPos;
	dlg.EndAddr = 0;

	INT_PTR result = dlg.DoModalDelay(200);

	if (IDOK == result)
	{
		SetCaretPosition(dlg.BeginAddr,
						SetPositionCancelSelection | SetPositionMakeVisible);
	}
}

int CBinaryCompareView::LinesInView() const
{
	CRect cr;
	GetClientRect( & cr);
	return cr.Height() / LineHeight();
}

int CBinaryCompareView::CharsInView() const
{
	CRect cr;
	GetClientRect( & cr);
	return cr.Width() / CharWidth();
}

void CBinaryCompareView::UpdateVisibleRect()
{
	CRect cr;
	GetClientRect( & cr);

	m_VisibleRect.top = 0;
	m_VisibleRect.bottom = cr.Height() / LineHeight() - 1;

	m_PreferredRect.bottom = m_VisibleRect.bottom / 4;
	m_PreferredRect.top = m_VisibleRect.bottom;//m_VisibleRect.bottom - m_VisibleRect.bottom / 4;

	int nPaneWidth = cr.Width() - m_AddressMarginWidth - (m_NumberOfPanes - 1);

	m_VisibleRect.right = nPaneWidth / CharWidth() / m_NumberOfPanes;
	if (m_VisibleRect.right < 0)
	{
		m_VisibleRect.right = 0;
	}

	m_VisibleRect.left = 0;
	m_PreferredRect.left = m_VisibleRect.right / 3;
	m_PreferredRect.right = m_VisibleRect.right - m_VisibleRect.right / 3;
}

void CBinaryCompareView::OnViewLessbytesinline()
{
	if (m_BytesPerLine > 8)
	{
		m_BytesPerLine /= 2;
		OnMetricsChange();
	}
}

void CBinaryCompareView::OnUpdateViewLessbytesinline(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_BytesPerLine > 8);
}

void CBinaryCompareView::OnViewMorebytesinline()
{
	if (m_BytesPerLine < 64)
	{
		m_BytesPerLine *= 2;
		OnMetricsChange();
	}
}

void CBinaryCompareView::OnUpdateViewMorebytesinline(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_BytesPerLine < 64);
}

void CBinaryCompareView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	BaseClass::OnActivateView(bActivate, pActivateView, pDeactiveView);
	CFrameWnd * pMainFrm = dynamic_cast<CFrameWnd *>(AfxGetMainWnd());
	if (NULL != pMainFrm)
	{
		if (bActivate && this == pActivateView)
		{
			FilePair * pPair = GetFilePair();
			if (NULL != pPair)
			{
				pMainFrm->SetMessageText(pPair->GetComparisonResultStr());
			}
		}
		else
		{
			pMainFrm->SetMessageText(AFX_IDS_IDLEMESSAGE);
		}
	}
}

int CBinaryCompareView::PointToPaneNumber(int x)
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

	x -= m_AddressMarginWidth;
	if (x < 0)
	{
		x = 0;
	}

	int nPane = x / nPaneWidth;
	if (nPane >= m_NumberOfPanes)
	{
		nPane = m_NumberOfPanes - 1;
	}
	return nPane;
}

int CBinaryCompareView::PointToPaneOffset(int x, int nPane)
{
	x -= m_AddressMarginWidth;

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
		if (nPane < 0)
		{
			nPane = 0;
		}
	}

	if (nPane >= m_NumberOfPanes)
	{
		nPane = m_NumberOfPanes - 1;
	}

	x -= nPane * nPaneWidth + CharWidth() * m_FirstPosSeen;

	if (x < 0)
	{
		x = 0;
	}

	return x;
}

int CBinaryCompareView::GetPaneWidth()
{
	CRect cr;
	GetClientRect( & cr);

	int nPaneWidth = cr.Width() - m_AddressMarginWidth
					- (m_NumberOfPanes - 1);

	if (m_NumberOfPanes > 1)
	{
		nPaneWidth = 1 + nPaneWidth / CharWidth() / m_NumberOfPanes * CharWidth();
	}

	if (nPaneWidth <= 0)
	{
		return 1;
	}
	return nPaneWidth;
}

void CBinaryCompareView::OnViewSideBySide()
{
	if (GetFilePair()->CanCompare())
	{
		if (2 == m_NumberOfPanes)
		{
			// TODO: adjust FirstPosSeen
			m_NumberOfPanes = 1;
			m_PaneWithFocus = 0;
//            pDoc->m_CaretPos.scope = 0;
		}
		else
		{
			m_NumberOfPanes = 2;
//            pDoc->m_CaretPos.scope = m_PaneWithFocus + 1;
			// TODO: convert the address
		}
		GetApp()->m_NumberOfPanes = m_NumberOfPanes;
		OnMetricsChange();
	}
}

void CBinaryCompareView::OnUpdateViewSideBySide(CCmdUI *pCmdUI)
{
	if (GetFilePair()->CanCompare())
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(m_NumberOfPanes > 1);
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetCheck(0);
	}
}


void CBinaryCompareView::OnEditGoto()
{
	ThisDoc * pDoc = GetDocument();

	CBinaryGoToDlg dlg(pDoc->m_CaretPos, pDoc->GetFileSize());

	if (IDOK == dlg.DoModal())
	{
		SetCaretPosition(dlg.GetOffset(), SetPositionMakeVisible | SetPositionCancelSelection);
	}
}

void CBinaryCompareView::SetFocusPane(int pane)
{
	if (pane != m_PaneWithFocus)
	{
		ThisDoc * pDoc = GetDocument();
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
		InvalidateRange(SelBegin, SelEnd);
		m_PaneWithFocus = pane;
		InvalidateRange(SelBegin, SelEnd);
	}
}
