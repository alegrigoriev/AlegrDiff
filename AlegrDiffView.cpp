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
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_LISTVIEW_OPEN, OnListviewOpen)
	ON_UPDATE_COMMAND_UI(ID_LISTVIEW_OPEN, OnUpdateListviewOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_COPY_FIRST_DIR, OnUpdateFileCopyFirstDir)
	ON_COMMAND(ID_FILE_COPY_FIRST_DIR, OnFileCopyFirstDir)
	ON_UPDATE_COMMAND_UI(ID_FILE_COPY_SECOND_DIR, OnUpdateFileCopySecondDir)
	ON_COMMAND(ID_FILE_COPY_SECOND_DIR, OnFileCopySecondDir)
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
	CThisApp * pApp = GetApp();
	if (pApp->m_FileListSort >= 0)
	{
		m_bAscendingOrder = true;
		m_SortColumn = eColumns(pApp->m_FileListSort);
	}
	else
	{
		m_bAscendingOrder = false;
		m_SortColumn = eColumns(~pApp->m_FileListSort);
	}
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
	pList->InsertColumn(ColumnComparisionResult, "Comparison result", LVCFMT_LEFT, 400, ColumnComparisionResult);
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
	CThisApp * pApp = GetApp();
	if (m_bAscendingOrder)
	{
		pApp->m_FileListSort = m_SortColumn;
	}
	else
	{
		pApp->m_FileListSort = ~m_SortColumn;
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

template <class _Operation>
class binder3rd
	: public binary_function<typename _Operation::first_argument_type,
							typename _Operation::second_argument_type,
							typename _Operation::result_type>
{
protected:
	_Operation op;
	typename _Operation::third_argument_type value;
public:
	binder3rd(const _Operation& __x,
			const typename _Operation::third_argument_type& __y)
		: op(__x), value(__y) {}
	typename _Operation::result_type
	operator()(const typename _Operation::first_argument_type& __x,
				const typename _Operation::second_argument_type& __y) const
	{
		return op(__x, __y, value);
	}
};

template <class _Operation, class _Tp>
inline binder3rd<_Operation>
	bind3rd(const _Operation& __fn, const _Tp& __x)
{
	typedef typename _Operation::third_argument_type _Arg3_type;
	return binder3rd<_Operation>(__fn, _Arg3_type(__x));
}

void CAlegrDiffView::BuildSortedPairArray(vector<FilePair *> & PairArray, FilePair * pPairs, int nCount)
{
	PairArray.resize(nCount);
	for (int i = 0; i < nCount && pPairs != NULL; i++, pPairs = pPairs->pNext)
	{
		PairArray[i] = pPairs;
	}
	PairArray.resize(i);

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
	int nSort =
		sort(PairArray.begin(), PairArray.end(), bind3rd(
														qsort(PairArray.GetData(), i, sizeof (FilePair *), SortFunc);
}

void CAlegrDiffView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CListCtrl * pListCtrl = &GetListCtrl();
	if (OnUpdateListViewItem == lHint)
	{
		AddListViewItemStruct * alvi = static_cast<AddListViewItemStruct *>(pHint);
		if (NULL != alvi)
		{
			for (int i = 0; i < m_PairArray.size(); i++)
			{
				if (m_PairArray[i] == alvi->pPair)
				{
					CString ComparisionResult = alvi->pPair->GetComparisionResult();
					pListCtrl->SetItemText(i, ColumnComparisionResult, ComparisionResult);
					break;
				}
			}
		}
		return;
	}
	// fill the list control
	LockWindowUpdate();
	pListCtrl->DeleteAllItems();
	CAlegrDiffDoc * pDoc = GetDocument();

	FilePair * pPair = pDoc->m_pPairList;

	BuildSortedPairArray(m_PairArray, pDoc->m_pPairList, pDoc->m_nFilePairs);

	pListCtrl->SetItemCount(m_PairArray.size());

	for (int item = 0; item < m_PairArray.size(); item++)
	{
		FilePair * pPair = m_PairArray[item];
		AddListViewItem(pPair, item);
	}
	if (! pDoc->m_bRecurseSubdirs)
	{
		// Hide the subdirectory column
		pListCtrl->SetColumnWidth(ColumnSubdir, 0);
	}
	pListCtrl->SetItemState(0, LVIS_FOCUSED, 0xFF);
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
		if (pNmlv->iItem >= 0 && pNmlv->iItem < m_PairArray.size())
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
		if (nItem < m_PairArray.size())
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
		&& nItem < m_PairArray.size()
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
	FileItem * pFile = NULL;
	if (-1 != nItem
		&& nItem < m_PairArray.size()
		&& NULL != m_PairArray[nItem])
	{
		pFile = m_PairArray[nItem]->pFirstFile;
	}
	ModifyOpenFileMenu(pCmdUI, pFile,
						IDS_OPEN_FIRST_FILE_MENU, IDS_OPEN_FIRST_FILE_MENU_DISABLED);
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
		&& nItem < m_PairArray.size()
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
	FileItem * pFile = NULL;
	if (-1 != nItem
		&& nItem < m_PairArray.size()
		&& NULL != m_PairArray[nItem])
	{
		pFile = m_PairArray[nItem]->pSecondFile;
	}
	ModifyOpenFileMenu(pCmdUI, pFile,
						IDS_OPEN_SECOND_FILE_MENU, IDS_OPEN_SECOND_FILE_MENU_DISABLED);
}

void CAlegrDiffView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// make sure window is active
	GetParentFrame()->ActivateFrame();

	CMenu menu;
	if (menu.LoadMenu(IDR_MENU_LISTVIEW_CONTEXT))
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

void CAlegrDiffView::OnListviewOpen()
{
	CListCtrl * pListCtrl = & GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	while(-1 != nItem)
	{
		if (nItem < m_PairArray.size())
		{
			GetApp()->OpenFilePairView(m_PairArray[nItem]);
		}
		nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
	}
}

void CAlegrDiffView::OnUpdateListviewOpen(CCmdUI* pCmdUI)
{
	CListCtrl * pListCtrl = & GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 != nItem)
	{
		if (NULL != pCmdUI->m_pMenu)
		{
			pCmdUI->m_pMenu->SetDefaultItem(ID_LISTVIEW_OPEN, FALSE);
		}
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CAlegrDiffView::OnUpdateFileCopyFirstDir(CCmdUI* pCmdUI)
{
	CString s;
	s.Format(IDS_COPY_FROM_DIR_FORMAT, LPCTSTR(GetDocument()->m_sFirstDir));
	pCmdUI->SetText(s);
}

void CAlegrDiffView::OnFileCopyFirstDir()
{
	CopySelectedFiles(false);
}

void CAlegrDiffView::OnUpdateFileCopySecondDir(CCmdUI* pCmdUI)
{
	CString s;
	s.Format(IDS_COPY_FROM_DIR_FORMAT, LPCTSTR(GetDocument()->m_sSecondDir));
	pCmdUI->SetText(s);
}

void CAlegrDiffView::OnFileCopySecondDir()
{
	CopySelectedFiles(true);
}

BOOL CAlegrDiffView::CopySelectedFiles(bool bSecondDir)
{
	CThisApp * pApp = GetApp();

	CListCtrl * pListCtrl = & GetListCtrl();
	CArray<FileItem *, FileItem *> FilesArray;

	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	while(-1 != nItem)
	{
		if (nItem < m_PairArray.size())
		{
			FilePair * pPair = m_PairArray[nItem];
			FileItem * pFile;
			if (bSecondDir)
			{
				pFile = pPair->pSecondFile;
			}
			else
			{
				pFile = pPair->pFirstFile;
			}
			if (NULL != pFile)
			{
				FilesArray.Add(pFile);
			}
		}
		nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
	}
	if (0 == FilesArray.GetSize())
	{
		return FALSE;
	}

	CopyFilesToFolder(FilesArray.GetData(), FilesArray.GetSize(), true);
	return TRUE;
}

void CAlegrDiffView::AddListViewItem(FilePair *pPair, int item)
{
	CListCtrl * pListCtrl = &GetListCtrl();
	CAlegrDiffDoc * pDoc = GetDocument();

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
