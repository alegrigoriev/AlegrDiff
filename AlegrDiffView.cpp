// AlegrDiffView.cpp : implementation of the CAlegrDiffView class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "AlegrDiffDoc.h"
#include "AlegrDiffView.h"
#include "DiffFileView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView

DWORD AFXAPI _AfxGetComCtlVersion();

IMPLEMENT_DYNCREATE(CAlegrDiffView, CListView)

BEGIN_MESSAGE_MAP(CAlegrDiffView, CListView)
	//{{AFX_MSG_MAP(CAlegrDiffView)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
	ON_COMMAND(ID_FILE_EDIT_FIRST, OnFileEditFirst)
	ON_UPDATE_COMMAND_UI(ID_FILE_EDIT_FIRST, OnUpdateFileEditFirst)
	ON_COMMAND(ID_FILE_EDIT_SECOND, OnFileEditSecond)
	ON_UPDATE_COMMAND_UI(ID_FILE_EDIT_SECOND, OnUpdateFileEditSecond)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView construction/destruction

CAlegrDiffView::CAlegrDiffView()
	:m_SortColumn(ColumnSubdir),
	m_bAscendingOrder(true)
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
	CAlegrDiffDoc * pDoc = GetDocument();

	pList->InsertColumn(ColumnName, "File Name", LVCFMT_LEFT, 200, ColumnName);
	pList->InsertColumn(ColumnSubdir, "Subdirectory", LVCFMT_LEFT, 200, ColumnSubdir);
	pList->InsertColumn(ColumnDate1, "1st Modified at", LVCFMT_LEFT, 150, ColumnDate1);
	pList->InsertColumn(ColumnDate2, "2nd Modified at", LVCFMT_LEFT, 150, ColumnDate2);
	pList->InsertColumn(ColumnComparisionResult, "Comparision result", LVCFMT_LEFT, 400, ColumnComparisionResult);
	pList->SetExtendedStyle(pList->GetExtendedStyle() | LVS_EX_FULLROWSELECT);
}

void CAlegrDiffView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// change sort order
	if (m_SortColumn == pNMListView->iSubItem)
	{
		m_bAscendingOrder = ! m_bAscendingOrder;
	}
	else
	{
		m_SortColumn = eColumns(pNMListView->iSubItem);
		m_bAscendingOrder = true;
	}
	OnUpdate(NULL, 0, NULL);
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

void CAlegrDiffView::BuildSortedPairArray(CArray<FilePair *,FilePair *> & PairArray, FilePair * pPairs, int nCount)
{
	PairArray.SetSize(nCount);
	for (int i = 0; i < nCount && pPairs != NULL; i++, pPairs = pPairs->pNext)
	{
		PairArray[i] = pPairs;
	}
	PairArray.SetSize(i);
	int (_cdecl * SortFunc)(const void * , const void * );
	if (m_bAscendingOrder)
	{
		switch (m_SortColumn)
		{
		case ColumnName:
			SortFunc = FilePair::NameSortFunc;
			break;
		default:
		case ColumnSubdir:
			SortFunc = FilePair::DirNameSortFunc;
			break;
		case ColumnDate1:
			SortFunc = FilePair::Time1SortFunc;
			break;
		case ColumnDate2:
			SortFunc = FilePair::Time2SortFunc;
			break;
		case ColumnComparisionResult:
			SortFunc = FilePair::ComparisionSortFunc;
			break;
		}
	}
	else
	{
		switch (m_SortColumn)
		{
		case ColumnName:
			SortFunc = FilePair::NameSortBackwardsFunc;
			break;
		default:
		case ColumnSubdir:
			SortFunc = FilePair::DirNameSortBackwardsFunc;
			break;
		case ColumnDate1:
			SortFunc = FilePair::Time1SortBackwardsFunc;
			break;
		case ColumnDate2:
			SortFunc = FilePair::Time2SortBackwardsFunc;
			break;
		case ColumnComparisionResult:
			SortFunc = FilePair::ComparisionSortBackwardsFunc;
			break;
		}
	}

	qsort(PairArray.GetData(), i, sizeof (FilePair *), SortFunc);
}

void CAlegrDiffView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// fill the list control
	LockWindowUpdate();
	CListCtrl * pListCtrl = &GetListCtrl();
	pListCtrl->DeleteAllItems();
	CAlegrDiffDoc * pDoc = GetDocument();

	FilePair * pPair = pDoc->m_pPairList;

	BuildSortedPairArray(m_PairArray, pDoc->m_pPairList, pDoc->m_nFilePairs);

	pListCtrl->SetItemCount(m_PairArray.GetSize());

	for (int item = 0; item < m_PairArray.GetSize(); item++)
	{
		FilePair * pPair = m_PairArray[item];
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

		if (pDoc->m_bRecurseSubdirs)
		{
			pListCtrl->SetItemText(item, ColumnSubdir, (LPTSTR)pFileItem->GetSubdir());
		}
		// set modified time/date
		CString datetime;
		if (NULL != pPair->pFirstFile)
		{
			datetime = FileTimeToStr(pPair->pFirstFile->GetLastWriteTime());
			pListCtrl->SetItemText(item, ColumnDate1, datetime);
		}
		else
		{
			pListCtrl->SetItemText(item, ColumnDate1, _T(""));
		}
		if (NULL != pPair->pSecondFile)
		{
			datetime = FileTimeToStr(pPair->pSecondFile->GetLastWriteTime());
			pListCtrl->SetItemText(item, ColumnDate2, datetime);
		}
		else
		{
			pListCtrl->SetItemText(item, ColumnDate2, _T(""));
		}
		CString ComparisionResult = pPair->GetComparisionResult();
		pListCtrl->SetItemText(item, ColumnComparisionResult, ComparisionResult);
	}
	if (! pDoc->m_bRecurseSubdirs)
	{
		// Hide the subdirectory column
		pListCtrl->SetColumnWidth(ColumnSubdir, 0);
	}

	UnlockWindowUpdate();
}

void CAlegrDiffView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	CListCtrl * pListCtrl = & GetListCtrl();
	NMLISTVIEW * pNmlv = (NMLISTVIEW *) pNMHDR;
	// open new view for the files
	// compare two files
	if (_AfxGetComCtlVersion() >= 0x00040070)
	{
		if (pNmlv->iItem >= 0 && pNmlv->iItem < m_PairArray.GetSize())
		{
			// try to find if a view is already open
			// view not found, create a new
			GetApp()->OpenFilePairView(m_PairArray[pNmlv->iItem]);
		}
	}

//    int nItem = p;
	*pResult = 0;
}


void CAlegrDiffView::OnReturn(NMHDR* pNMHDR, LRESULT* pResult)
{
	CListCtrl * pListCtrl = & GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	while(-1 != nItem)
	{
		if (nItem < m_PairArray.GetSize())
		{
			GetApp()->OpenFilePairView(m_PairArray[nItem]);
		}
		nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
	}
	*pResult = 0;
}


void CAlegrDiffView::OnFileEditFirst()
{
	CListCtrl * pListCtrl = & GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	if (-1 != nItem
		&& nItem < m_PairArray.GetSize()
		&& NULL != m_PairArray[nItem])
	{
		OpenFileForEditing(m_PairArray[nItem]->pFirstFile);
	}
}

void CAlegrDiffView::OnUpdateFileEditFirst(CCmdUI* pCmdUI)
{
	CListCtrl * pListCtrl = & GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	if (-1 != nItem
		&& nItem < m_PairArray.GetSize()
		&& NULL != m_PairArray[nItem])
	{
		ModifyOpenFileMenu(pCmdUI, m_PairArray[nItem]->pFirstFile, _T("&1 Open "));
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CAlegrDiffView::OnFileEditSecond()
{
	CListCtrl * pListCtrl = & GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	if (-1 != nItem
		&& nItem < m_PairArray.GetSize()
		&& NULL != m_PairArray[nItem])
	{
		OpenFileForEditing(m_PairArray[nItem]->pSecondFile);
	}
}

void CAlegrDiffView::OnUpdateFileEditSecond(CCmdUI* pCmdUI)
{
	CListCtrl * pListCtrl = & GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	if (-1 != nItem
		&& nItem < m_PairArray.GetSize()
		&& NULL != m_PairArray[nItem])
	{
		ModifyOpenFileMenu(pCmdUI, m_PairArray[nItem]->pSecondFile, _T("&2 Open "));
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}
