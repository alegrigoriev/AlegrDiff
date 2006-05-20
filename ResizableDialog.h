// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#if !defined(AFX_RESIZABLEDIALOG_H__1E590ECD_BEE6_4C1C_9E9C_A8F58A8FA54A__INCLUDED_)
#define AFX_RESIZABLEDIALOG_H__1E590ECD_BEE6_4C1C_9E9C_A8F58A8FA54A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResizableDialog.h : header file
//
#include "UiUpdatedDlg.h"
#include "MessageMapT.h"

/////////////////////////////////////////////////////////////////////////////
// CResizableDialogT dialog
template<class Base = CUiUpdatedDlg>
class CResizableDialogT : public Base
{
	typedef Base BaseClass;
	// Construction
protected:
	CResizableDialogT(UINT id, CWnd* pParent);   // standard constructor
public:

	// Dialog Data
	//{{AFX_DATA(CResizableDialogT)
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizableDialogT)
	//}}AFX_VIRTUAL
	virtual INT_PTR DoModal();

	// Implementation
protected:

	CSize m_PrevSize;
	MINMAXINFO m_mmxi;
	int m_DlgWidth;
	int m_DlgHeight;

	enum
	{
		CenterHorizontally = 1,
		ExpandRight = 2,
		MoveRight = 4,
		ExpandDown = 8,
		MoveDown = 0x10,
		ThisIsDropCombobox = 0x20,  // combo fix required
	};
	struct ResizableDlgItem
	{
		UINT Id;
		UINT flags;
	};

	void SetResizeableItems(ResizableDlgItem const * pItems, int count)
	{
		m_pResizeItems = pItems;
		m_ResizeItemsCount = count;
	}
	void SetBigAndSmallIcons(UINT id);

	virtual void OnMetricsChange();
	// cx, cy - new size, dx, dy - size delta
	virtual HDWP OnDeferredSize(HDWP hdwp, int /*cx*/, int /*cy*/, int /*dx*/, int /*dy*/)
	{
		return hdwp;
	}
	// Generated message map functions
	//{{AFX_MSG(CResizableDialogT)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg UINT OnNcHitTest(CPoint point);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	ResizableDlgItem const * m_pResizeItems;
	int m_ResizeItemsCount;
	struct Combobox_data
	{
		CComboBox * pCombo;
		CString EditText;
		DWORD sel;
	};
};

template<class Base>
CResizableDialogT<Base>::CResizableDialogT(UINT id, CWnd* pParent)
	: BaseClass(id, pParent)
	, m_pResizeItems(NULL)
	, m_ResizeItemsCount(0)
	, m_DlgWidth(0)
	, m_DlgHeight(0)
{
	//{{AFX_DATA_INIT(CResizableDialogT)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_PrevSize.cx = -1;
	m_PrevSize.cy = -1;

	memzero(m_mmxi);
}

/////////////////////////////////////////////////////////////////////////////
// CResizableDialogT message handlers
template<class Base>
void CResizableDialogT<Base>::SetBigAndSmallIcons(UINT id)
{
	HICON hIcon = (HICON) LoadImage(AfxFindResourceHandle
									(MAKEINTRESOURCE(id), RT_GROUP_ICON),
									MAKEINTRESOURCE(id),
									IMAGE_ICON,
									GetSystemMetrics(SM_CXICON),
									GetSystemMetrics(SM_CYICON), 0);

	if (NULL != hIcon)
	{
		SetIcon(hIcon, TRUE);			// Set big icon
	}

	hIcon = (HICON) LoadImage(AfxFindResourceHandle
							(MAKEINTRESOURCE(id), RT_GROUP_ICON),
							MAKEINTRESOURCE(id),
							IMAGE_ICON,
							GetSystemMetrics(SM_CXSMICON),
							GetSystemMetrics(SM_CYSMICON), 0);

	if (NULL != hIcon)
	{
		SetIcon(hIcon, FALSE);			// Set small icon
	}
}

template<class Base>
void CResizableDialogT<Base>::OnMetricsChange()
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

template<class Base>
void CResizableDialogT<Base>::OnSize(UINT nType, int cx, int cy)
{
	BaseClass::OnSize(nType, cx, cy);

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

	if (0 != m_ResizeItemsCount)
	{

		std::vector<Combobox_data> combos;
		combos.reserve(m_ResizeItemsCount);

		HDWP hdwp = ::BeginDeferWindowPos(m_ResizeItemsCount);
		for (int i = 0; i < m_ResizeItemsCount && NULL != hdwp; i++)
		{
			UINT const flags = m_pResizeItems[i].flags;
			CWnd * const pWnd = GetDlgItem(m_pResizeItems[i].Id);

			if (NULL == pWnd)
			{
				continue;
			}

			CRect r;
			pWnd->GetWindowRect(r);
			ScreenToClient(r);

			if (flags & CenterHorizontally)
			{
				r.right += (dx + (cx & 1)) >> 1;
				r.left += (dx + (cx & 1)) >> 1;
			}
			else
			{
				if (flags & (ExpandRight | MoveRight))
				{
					r.right += dx;
				}
				if (flags & MoveRight)
				{
					r.left += dx;
				}
			}

			if (flags & (ExpandDown | MoveDown))
			{
				r.bottom += dy;
			}

			if (flags & MoveDown)
			{
				r.top += dy;
			}

			if ((flags & ThisIsDropCombobox)
				&& dx != 0)
			{
				// special processing for CComboBox
				CComboBox * pCombo = static_cast<CComboBox *>(pWnd);

				Combobox_data data;

				pCombo->GetWindowText(data.EditText);

				if ( ! data.EditText.IsEmpty())
				{
					data.pCombo = pCombo;
					data.sel = pCombo->GetEditSel();
					//pCombo->LockWindowUpdate();
					pCombo->SetWindowText(_T(""));

					combos.push_back(data);
				}
			}

			hdwp = ::DeferWindowPos(hdwp, pWnd->GetSafeHwnd(), NULL, r.left, r.top,
									r.Width(), r.Height(),
									SWP_NOZORDER | SWP_NOOWNERZORDER /*| SWP_NOACTIVATE | SWP_NOSENDCHANGING*/
									);
			if (0) TRACE("DeferWindowPos hwnd=%x dw=%d dy=%d x=%d, y=%d returned %X\n",
						pWnd->GetSafeHwnd(), dx, dy, r.left, r.top, hdwp);
		}

		hdwp = OnDeferredSize(hdwp, cx, cy, dx, dy);

		if (NULL != hdwp)
		{
			::EndDeferWindowPos(hdwp);
		}

		for (std::vector<Combobox_data>::const_iterator i = combos.begin();
			i != combos.end(); i++)
		{
			i->pCombo->SetWindowText(i->EditText);
			i->pCombo->SetEditSel(LOWORD(i->sel), HIWORD(i->sel));
			//i->pCombo->UnlockWindowUpdate();
		}
	}

	// invalidate an area which is (after resizing)
	// occupied by size grip
	int size = GetSystemMetrics(SM_CXVSCROLL);
	CRect r(cx - size, cy - size, cx, cy);
	InvalidateRect( & r, TRUE);
}

template<class Base>
void CResizableDialogT<Base>::OnSizing(UINT fwSide, LPRECT pRect)
{
	BaseClass::OnSizing(fwSide, pRect);

	// invalidate an area currently (before resizing)
	// occupied by size grip
	CRect r;
	GetClientRect( & r);
	int size = GetSystemMetrics(SM_CXVSCROLL);
	r.left = r.right - size;
	r.top = r.bottom - size;
	InvalidateRect( & r, FALSE);
}

template<class Base>
void CResizableDialogT<Base>::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (m_mmxi.ptMaxSize.x != 0)
	{
		*lpMMI = m_mmxi;
	}
	else
	{
		BaseClass::OnGetMinMaxInfo(lpMMI);
	}
}

template<class Base>
BOOL CResizableDialogT<Base>::OnEraseBkgnd(CDC* pDC)
{
	if (BaseClass::OnEraseBkgnd(pDC))
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

template<class Base>
UINT CResizableDialogT<Base>::OnNcHitTest(CPoint point)
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
		return BaseClass::OnNcHitTest(point);
}

template<class Base>
BOOL CResizableDialogT<Base>::OnInitDialog()
{
	BaseClass::OnInitDialog();

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

template<class Base>
void CResizableDialogT<Base>::OnOK()
{
	CRect r;
	GetWindowRect( & r);
	m_DlgWidth = r.Width();
	m_DlgHeight = r.Height();

	BaseClass::OnOK();
}

template<class Base>
INT_PTR CResizableDialogT<Base>::DoModal()
{
	m_PrevSize.cy = -1;
	m_PrevSize.cx = -1;
	memzero(m_mmxi);

	return BaseClass::DoModal();
}

BEGIN_MESSAGE_MAP_T(CResizableDialogT, BaseClass)
	//{{AFX_MSG_MAP(CResizableDialog)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_GETMINMAXINFO()
	ON_WM_ERASEBKGND()
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


typedef CResizableDialogT<> CResizableDialog;
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESIZABLEDIALOG_H__1E590ECD_BEE6_4C1C_9E9C_A8F58A8FA54A__INCLUDED_)
