// AlegrDiffView.cpp : implementation of the CAlegrDiffView class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "AlegrDiffDoc.h"
#include "AlegrDiffView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView

IMPLEMENT_DYNCREATE(CAlegrDiffView, CListView)

BEGIN_MESSAGE_MAP(CAlegrDiffView, CListView)
	//{{AFX_MSG_MAP(CAlegrDiffView)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView construction/destruction

CAlegrDiffView::CAlegrDiffView()
{
	// TODO: add construction code here

}

CAlegrDiffView::~CAlegrDiffView()
{
}

BOOL CAlegrDiffView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style |= LVS_SHOWSELALWAYS | LVS_REPORT;
	//cs.dwExStyle |= LVS_EX_FULLROWSELECT;
	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView drawing

void CAlegrDiffView::OnDraw(CDC* pDC)
{
	CAlegrDiffDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView printing

BOOL CAlegrDiffView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CAlegrDiffView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CAlegrDiffView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView diagnostics

#ifdef _DEBUG
void CAlegrDiffView::AssertValid() const
{
	CListView::AssertValid();
}

void CAlegrDiffView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CAlegrDiffDoc* CAlegrDiffView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAlegrDiffDoc)));
	return (CAlegrDiffDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView message handlers

void CAlegrDiffView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	// set style, header columns
	CListCtrl * pList = &GetListCtrl();
	CHeaderCtrl * pHeader = pList->GetHeaderCtrl();
	pList->InsertColumn(0, "File Name", LVCFMT_LEFT, 200, 0);
	pList->InsertColumn(1, "Subdirectory", LVCFMT_LEFT, 200, 1);
	pList->InsertColumn(2, "1st Modified at", LVCFMT_LEFT, 150, 2);
	pList->InsertColumn(3, "2nd Modified at", LVCFMT_LEFT, 150, 3);
	pList->InsertColumn(4, "Comparision result", LVCFMT_LEFT, 400, 4);
	pList->SetExtendedStyle(pList->GetExtendedStyle() | LVS_EX_FULLROWSELECT);
}

void CAlegrDiffView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// change sort order

	*pResult = 0;
}

CString FileTimeToStr(FILETIME FileTime, LCID locale = LOCALE_USER_DEFAULT)
{
	int const TimeBufSize = 256;
	TCHAR str[TimeBufSize] = {0};
	SYSTEMTIME SystemTime;
	SYSTEMTIME LocalTime;
	memset( & LocalTime, 0, sizeof LocalTime);
	FileTimeToSystemTime( & FileTime, & SystemTime);
	SystemTimeToTzSpecificLocalTime(NULL, & SystemTime, & LocalTime);

	GetDateFormat(locale, DATE_SHORTDATE, & LocalTime, NULL, str, TimeBufSize - 1);
	CString result = str;
	result += ' ';

	GetTimeFormat(locale, TIME_NOSECONDS, & LocalTime, NULL, str, TimeBufSize - 1);
	result += str;
	return result;
}

void CAlegrDiffView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// fill the list control
	LockWindowUpdate();
	CListCtrl * pListCtrl = &GetListCtrl();
	pListCtrl->DeleteAllItems();
	CAlegrDiffDoc * pDoc = GetDocument();

	FilePair * pPair = pDoc->m_pPairList;

	for (int item = 0; NULL != pPair; item++, pPair = pPair->pNext)
	{
		FileItem * pFileItem = pPair->pFirstFile;
		if (NULL == pFileItem)
		{
			pFileItem = pPair->pSecondFile;
		}
		LVITEM lvi;
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.iItem = item;
		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)pFileItem->GetName();
		lvi.lParam = LPARAM(pFileItem);
		pListCtrl->InsertItem(& lvi);

		pListCtrl->SetItemText(item, 1, (LPTSTR)pFileItem->GetSubdir());
		// set modified time/date
		CString datetime;
		if (NULL != pPair->pFirstFile)
		{
			datetime = FileTimeToStr(pPair->pFirstFile->GetLastWriteTime());
			pListCtrl->SetItemText(item, 2, datetime);
		}
		else
		{
			pListCtrl->SetItemText(item, 2, _T(""));
		}
		if (NULL != pPair->pSecondFile)
		{
			datetime = FileTimeToStr(pPair->pSecondFile->GetLastWriteTime());
			pListCtrl->SetItemText(item, 3, datetime);
		}
		else
		{
			pListCtrl->SetItemText(item, 3, _T(""));
		}
		CString ComparisionResult = pPair->GetComparisionResult();
		pListCtrl->SetItemText(item, 4, ComparisionResult);
	}

	UnlockWindowUpdate();
}

void CAlegrDiffView::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	FilePair * pPair = (FilePair *)pDispInfo->item.lParam;
	TRACE("CAlegrDiffView::OnGetdispinfo:pPair=%x\n", pPair);
	pDispInfo->item.pszText = _T("OK");
	*pResult = 0;
}
