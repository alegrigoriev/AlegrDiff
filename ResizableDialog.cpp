// ResizableDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ResizableDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizableDialog dialog


CResizableDialog::CResizableDialog(UINT id, CWnd* pParent)
	: CDialog(id, pParent)
{
	//{{AFX_DATA_INIT(CResizableDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pResizeItems = NULL;
	m_pResizeItemsCount = 0;
	m_PrevSize.cx = -1;
	m_PrevSize.cy = -1;
	m_DlgWidth = 0;
	m_DlgHeight = 0;

	memzero(m_mmxi);
}


BEGIN_MESSAGE_MAP(CResizableDialog, CDialog)
	//{{AFX_MSG_MAP(CResizableDialog)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_GETMINMAXINFO()
	ON_WM_ERASEBKGND()
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResizableDialog message handlers

void CResizableDialog::OnMetricsChange()
{
	// Initialize MINMAXINFO
	CRect r;
	SystemParametersInfo(SPI_GETWORKAREA, 0, & r, 0);
	m_mmxi.ptMaxSize.x = r.Width();
	m_mmxi.ptMaxTrackSize.x = m_mmxi.ptMaxSize.x;
	m_mmxi.ptMaxSize.y = r.Height();
	m_mmxi.ptMaxTrackSize.y = m_mmxi.ptMaxSize.y;
	m_mmxi.ptMaxPosition.x = r.left;
	m_mmxi.ptMaxPosition.y = r.top;
	GetWindowRect(& r);
	m_mmxi.ptMinTrackSize.x = r.Width();
	m_mmxi.ptMinTrackSize.y = r.Height();
}

void CResizableDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (m_PrevSize.cx < 0)
	{
		m_PrevSize.cx = cx;
		m_PrevSize.cy = cy;
		return;
	}

	int dx = cx - m_PrevSize.cx;
	int dy = cy - m_PrevSize.cy;
	m_PrevSize.cx = cx;
	m_PrevSize.cy = cy;

	if (0 == dx && 0 == dy)
	{
		TRACE("Nothing to do in OnSize\n");
		return;
	}

	if (0 != m_pResizeItemsCount)
	{
		HDWP hdwp = ::BeginDeferWindowPos(m_pResizeItemsCount);
		for (int i = 0; i < m_pResizeItemsCount && NULL != hdwp; i++)
		{
			HWND hWnd = ::GetDlgItem(GetSafeHwnd(), m_pResizeItems[i].Id);
			if (NULL == hWnd) continue;

			CRect cr;
			::GetWindowRect(hWnd, cr);
			ScreenToClient(cr);

			if (m_pResizeItems[i].flags & CenterHorizontally)
			{
				cr.right += (dx + (cx & 1)) >> 1;
				cr.left += (dx + (cx & 1)) >> 1;
			}
			else
			{
				if (m_pResizeItems[i].flags & (ExpandRight | MoveRight))
				{
					cr.right += dx;
				}
				if (m_pResizeItems[i].flags & MoveRight)
				{
					cr.left += dx;
				}
			}

			if (m_pResizeItems[i].flags & (ExpandDown | MoveDown))
			{
				cr.bottom += dy;
			}

			if (m_pResizeItems[i].flags & MoveDown)
			{
				cr.top += dy;
			}

			hdwp = ::DeferWindowPos(hdwp, hWnd, NULL, cr.left, cr.top,
									cr.Width(), cr.Height(),
									SWP_NOZORDER | SWP_NOOWNERZORDER// | SWP_NOACTIVATE | SWP_NOSENDCHANGING
									);
			if (0) TRACE("DeferWindowPos hwnd=%x dw=%d dy=%d x=%d, y=%d returned %X\n",
						hWnd, dx, dy, cr.left, cr.top, hdwp);
		}

		if (NULL != hdwp)
		{
			::EndDeferWindowPos(hdwp);
		}

	}

	// invalidate an area which is (after resizing)
	// occupied by size grip
	int size = GetSystemMetrics(SM_CXVSCROLL);
	CRect r(cx - size, cy - size, cx, cy);
	InvalidateRect( & r, TRUE);
}

void CResizableDialog::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);

	// invalidate an area currently (before resizing)
	// occupied by size grip
	CRect r;
	GetClientRect( & r);
	int size = GetSystemMetrics(SM_CXVSCROLL);
	r.left = r.right - size;
	r.top = r.bottom - size;
	InvalidateRect( & r, FALSE);
}

void CResizableDialog::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (m_mmxi.ptMaxSize.x != 0)
	{
		*lpMMI = m_mmxi;
	}
	else
	{
		CDialog::OnGetMinMaxInfo(lpMMI);
	}
}

BOOL CResizableDialog::OnEraseBkgnd(CDC* pDC)
{
	if (CDialog::OnEraseBkgnd(pDC))
	{
		// draw size grip
		CRect r;
		GetClientRect( & r);
		int size = GetSystemMetrics(SM_CXVSCROLL);
		r.left = r.right - size;
		r.top = r.bottom - size;
		pDC->DrawFrameControl( & r, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UINT CResizableDialog::OnNcHitTest(CPoint point)
{
	// return HTBOTTOMRIGHT for sizegrip area
	CRect r;
	GetClientRect( & r);
	int size = GetSystemMetrics(SM_CXVSCROLL);
	r.left = r.right - size;
	r.top = r.bottom - size;
	ScreenToClient( & point);

	if (r.PtInRect(point))
	{
		return HTBOTTOMRIGHT;
	}
	else
		return CDialog::OnNcHitTest(point);
}


BOOL CResizableDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// init MINMAXINFO
	OnMetricsChange();

	// set dialog size
	if (m_DlgWidth < m_mmxi.ptMinTrackSize.x)
	{
		m_DlgWidth = m_mmxi.ptMinTrackSize.x;
	}
	if (m_DlgWidth > m_mmxi.ptMaxTrackSize.x)
	{
		m_DlgWidth = m_mmxi.ptMaxTrackSize.x;
	}
	if (m_DlgHeight < m_mmxi.ptMinTrackSize.y)
	{
		m_DlgHeight = m_mmxi.ptMinTrackSize.y;
	}
	if (m_DlgHeight > m_mmxi.ptMaxTrackSize.y)
	{
		m_DlgHeight = m_mmxi.ptMaxTrackSize.y;
	}

	SetWindowPos(NULL, 0, 0, m_DlgWidth, m_DlgHeight,
				SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CResizableDialog::OnOK()
{
	CRect r;
	GetWindowRect( & r);
	m_DlgWidth = r.Width();
	m_DlgHeight = r.Height();

	CDialog::OnOK();
}
