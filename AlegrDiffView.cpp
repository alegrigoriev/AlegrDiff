// AlegrDiffView.cpp : implementation of the CAlegrDiffView class
//

#include "stdafx.h"
#include "AlegrDiff.h"

#include "AlegrDiffDoc.h"
#include "AlegrDiffView.h"
#include "DiffFileView.h"
#include <functional>
#include <algorithm>
#include "SaveFileListDlg.h"

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
	ON_COMMAND(ID_FILE_SAVE_LIST, OnFileSaveList)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_LIST, OnUpdateFileSaveList)
	ON_COMMAND(ID_VIEW_HIDESELECTEDFILES, OnViewHideselectedfiles)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIDESELECTEDFILES, OnUpdateViewHideselectedfiles)
	ON_COMMAND(ID_VIEW_SHOWALLFILES, OnViewShowallfiles)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWALLFILES, OnUpdateViewShowallfiles)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_UPDATE_COMMAND_UI(ID_LISTVIEW_OPEN1, OnUpdateListviewOpen)
	ON_COMMAND(ID_LISTVIEW_OPEN1, OnListviewOpen)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
//    ON_COMMAND(ID_LISTVIEW__FILELENGTH, OnListview)
ON_COMMAND(ID_LISTVIEW_FILELENGTH, OnListviewFilelength)
ON_UPDATE_COMMAND_UI(ID_LISTVIEW_FILELENGTH, OnUpdateListviewFilelength)
ON_COMMAND(ID_LISTVIEW_MODIFICATIONTIME, OnListviewModificationtime)
ON_UPDATE_COMMAND_UI(ID_LISTVIEW_MODIFICATIONTIME, OnUpdateListviewModificationtime)
ON_COMMAND(ID_LISTVIEW_SORTBY_1STLENGTH, OnListviewSortby1stlength)
ON_UPDATE_COMMAND_UI(ID_LISTVIEW_SORTBY_1STLENGTH, OnUpdateListviewSortby1stlength)
ON_COMMAND(ID_LISTVIEW_SORTBY_1STMODIFICATIONDATE, OnListviewSortby1stmodificationdate)
ON_UPDATE_COMMAND_UI(ID_LISTVIEW_SORTBY_1STMODIFICATIONDATE, OnUpdateListviewSortby1stmodificationdate)
ON_COMMAND(ID_LISTVIEW_SORTBY_2NDLENGTH, OnListviewSortby2ndlength)
ON_UPDATE_COMMAND_UI(ID_LISTVIEW_SORTBY_2NDLENGTH, OnUpdateListviewSortby2ndlength)
ON_COMMAND(ID_LISTVIEW_SORTBY_COMPARISONRESULT, OnListviewSortbyComparisonresult)
ON_UPDATE_COMMAND_UI(ID_LISTVIEW_SORTBY_COMPARISONRESULT, OnUpdateListviewSortbyComparisonresult)
ON_COMMAND(ID_LISTVIEW_SORTBY_FOLDER, OnListviewSortbyFolder)
ON_UPDATE_COMMAND_UI(ID_LISTVIEW_SORTBY_FOLDER, OnUpdateListviewSortbyFolder)
ON_COMMAND(ID_LISTVIEW_SORTBY_NAME, OnListviewSortbyName)
ON_UPDATE_COMMAND_UI(ID_LISTVIEW_SORTBY_NAME, OnUpdateListviewSortbyName)
ON_COMMAND(ID_LISTVIEW_SORTBY_DESCENDINGORDER, OnListviewSortbyDescendingorder)
ON_UPDATE_COMMAND_UI(ID_LISTVIEW_SORTBY_DESCENDINGORDER, OnUpdateListviewSortbyDescendingorder)
ON_COMMAND(ID_LISTVIEW_SORTBY_2NDMODIFICATIONDATE, OnListviewSortby2ndmodificationdate)
ON_UPDATE_COMMAND_UI(ID_LISTVIEW_SORTBY_2NDMODIFICATIONDATE, OnUpdateListviewSortby2ndmodificationdate)
ON_NOTIFY(HDN_BEGINDRAG, 0, OnHdnBegindrag)
ON_NOTIFY(HDN_ENDDRAG, 0, OnHdnEnddrag)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView construction/destruction

inline void CAlegrDiffView::PrintColumnOrder()
{
#ifdef _DEBUG
	int ColumnArray[MaxColumns];
	memzero(ColumnArray);

	CListCtrl * pList = &GetListCtrl();

	pList->GetColumnOrderArray(ColumnArray, countof(ColumnArray));
	CHeaderCtrl* pHeaderCtrl = pList->GetHeaderCtrl();

	int Columns = pHeaderCtrl->GetItemCount();
	for (int i = 0; i < Columns; i++)
	{
		TRACE("Column %d = %d\n", i, ColumnArray[i]);
	}
#endif
}

CAlegrDiffView::CAlegrDiffView()
{
	CThisApp * pApp = GetApp();

	for (int i = 0; i < countof(m_ColumnArray); i++)
	{
		m_ColumnArray[i] = pApp->m_ColumnArray[i];
		m_ColumnWidthArray[i] = pApp->m_ColumnWidthArray[i];
		TRACE("Column %d width=%d\n", i, m_ColumnWidthArray[i]);

		m_SortColumns[i] = eColumns(pApp->m_ColumnSort[i] & 0x7F);
		m_AscendingSortOrder[i] = 0 == (pApp->m_ColumnSort[i] & 0x80);
	}
}

CAlegrDiffView::~CAlegrDiffView()
{
}

BOOL CAlegrDiffView::PreCreateWindow(CREATESTRUCT& cs)
{
	// Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style |= LVS_SHOWSELALWAYS | LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView drawing

void CAlegrDiffView::OnDraw(CDC* pDC)
{
	CAlegrDiffDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
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

	// set style, header columns
	CListCtrl * pList = &GetListCtrl();
//    CAlegrDiffDoc * pDoc = GetDocument();

	pList->SetExtendedStyle(pList->GetExtendedStyle()
							| LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);

	// validate the column array, make the initial value for it
	int i, j;
	if (m_ColumnArray[ColumnName] != 0
		|| m_ColumnWidthArray[ColumnName] <= 0)
	{
		ResetColumnsArray();
	}
	// if one column is hidden, another should be hidden, too
	// only file date and size columns may be hidden
	if ((m_ColumnWidthArray[ColumnDate1] >= 0)
		!= (m_ColumnWidthArray[ColumnDate2] >= 0)
		|| (m_ColumnWidthArray[ColumnLength1] >= 0)
		!= (m_ColumnWidthArray[ColumnLength2] >= 0))
	{
		ResetColumnsArray();
	}

	int ScreenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
	for (i = 0; i < countof(m_ColumnArray); i++)
	{
		// check that the index happens only once, that there is no gap in the index sequence
		int n = 0;
		for (j = 0; j < countof(m_ColumnArray); j++)
		{
			if (m_ColumnArray[j] == i)
			{
				n++;
			}
		}
		if (n > 1)
		{
			ResetColumnsArray();
			break;
		}

		if (m_ColumnWidthArray[i] <= 0)
		{
			// only file date and size columns may be hidden
			if (i != ColumnDate1
				&& i != ColumnDate2
				&& i != ColumnLength1
				&& i != ColumnLength2)
			{
				ResetColumnsArray();
				break;
			}
			if ( -m_ColumnWidthArray[i] > ScreenWidth * 8)
			{
				ResetColumnsArray();
				break;
			}
		}
		else if (m_ColumnWidthArray[i] > ScreenWidth * 8)
		{
			ResetColumnsArray();
			break;
		}
	}

	CListView::OnInitialUpdate();
}

void CAlegrDiffView::ResetColumnsArray()
{
	m_ColumnArray[ColumnName] = 0;
	m_ColumnArray[ColumnSubdir] = 1;
	m_ColumnArray[ColumnDate1] = 2;
	m_ColumnArray[ColumnDate2] = 3;
	m_ColumnArray[ColumnLength1] = 4;
	m_ColumnArray[ColumnLength2] = 5;
	m_ColumnArray[ColumnComparisionResult] = 6;

	m_ColumnWidthArray[ColumnName] = 200;
	m_ColumnWidthArray[ColumnSubdir] = 200;
	m_ColumnWidthArray[ColumnDate1] = 150;
	m_ColumnWidthArray[ColumnDate2] = 150;
	m_ColumnWidthArray[ColumnLength1] = 100;
	m_ColumnWidthArray[ColumnLength2] = 100;
	m_ColumnWidthArray[ColumnComparisionResult] = 400;

}

void CAlegrDiffView::SetSortColumn(eColumns nColumn, eSetSortColumnOrder Order)
{
	if (nColumn >= MaxColumns)
	{
		return;
	}

	if (m_SortColumns[0] == nColumn)
	{
		if (Order != SetSortColumnMouseClick
			&& (Order == SetSortColumnAscending) == m_AscendingSortOrder[0])
		{
			// sort order remains the same
			return;
		}
		m_AscendingSortOrder[0] = ! m_AscendingSortOrder[0];
	}
	else
	{
		// remove the column from the array
		int i, j;
		for (i = 0, j = 0; i < countof (m_SortColumns); i++)
		{
			if (m_SortColumns[i] != nColumn)
			{
				m_SortColumns[j] = m_SortColumns[i];
				m_AscendingSortOrder[j] = m_AscendingSortOrder[i];
				j++;
			}
			else if (Order == SetSortColumnMouseClick)
			{
				// if column header was clicked, retain previous sort order
				if (m_AscendingSortOrder[i])
				{
					Order = SetSortColumnAscending;
				}
				else
				{
					Order = SetSortColumnDescending;
				}
			}
		}

		for (i = MaxColumns - 1; i > 0; i--)
		{
			m_SortColumns[i] = m_SortColumns[i - 1];
			m_AscendingSortOrder[i] = m_AscendingSortOrder[i - 1];
		}

		m_SortColumns[0] = eColumns(nColumn);
		m_AscendingSortOrder[0] = (Order != SetSortColumnDescending);
	}

	UpdateAppSort();

	OnUpdate(NULL, 0, NULL);
}

void CAlegrDiffView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// change sort order
	unsigned nColumn = pNMListView->iSubItem;
	if (nColumn >= countof (m_ColumnToItem))
	{
		return;
	}

	SetSortColumn(m_ColumnToItem[nColumn], SetSortColumnMouseClick);

	*pResult = 0;
}

void CAlegrDiffView::BuildSortedPairArray(vector<FilePair *> & PairArray, FilePair * pPairs, int nCount)
{
	PairArray.clear();
	PairArray.reserve(nCount);
	for (int i = 0; i < nCount && pPairs != NULL; i++, pPairs = pPairs->pNext)
	{
		if (NULL != pPairs->pFirstFile
			&& NULL != pPairs->pSecondFile
			&& pPairs->pFirstFile->IsFolder())
		{
			continue;
		}
		if ( ! pPairs->m_bHideFromListView)
		{
			PairArray.push_back(pPairs);
		}
	}

	std::sort(PairArray.begin(), PairArray.end(),
			FilePairComparePredicate(m_SortColumns, m_AscendingSortOrder, countof (m_SortColumns)));
}

void CAlegrDiffView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CListCtrl * pListCtrl = &GetListCtrl();
	CAlegrDiffDoc * pDoc = GetDocument();
	if (OnUpdateListViewItem == lHint)
	{
		AddListViewItemStruct * alvi = static_cast<AddListViewItemStruct *>(pHint);
		if (NULL != alvi)
		{
			for (unsigned i = 0; i < m_PairArray.size(); i++)
			{
				if (m_PairArray[i] == alvi->pPair)
				{
					CString ComparisionResult = alvi->pPair->GetComparisionResult();
					pListCtrl->SetItemText(i,
											m_ColumnArray[ColumnComparisionResult],
											ComparisionResult);
					break;
				}
			}
		}
		return;
	}
	// fill the list control
	for (unsigned i = 0; i < m_PairArray.size(); i++)
	{
		m_PairArray[i]->m_bSelected = false;
	}
	// get all currently selected items
	unsigned nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	while(-1 != nItem)
	{
		if (nItem < m_PairArray.size())
		{
			m_PairArray[nItem]->m_bSelected = true;
		}
		nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
	}

	LockWindowUpdate();
	pListCtrl->DeleteAllItems();

	CHeaderCtrl * pHeader = pListCtrl->GetHeaderCtrl();

	// Delete all of the items.
	for (int i = pHeader->GetItemCount(); i > 0; i--)
	{
		pListCtrl->DeleteColumn(i - 1);
	}

	CString titles[MaxColumns];

	titles[ColumnName].LoadString(IDS_STRING_COLUMN_FILENAME);
	titles[ColumnSubdir].LoadString(IDS_STRING_COLUMN_SUBDIRECTORY);
	titles[ColumnDate1].LoadString(IDS_STRING_COLUMN_1ST_MODIFIED);
	titles[ColumnDate2].LoadString(IDS_STRING_COLUMN_2ND_MODIFIED);
	titles[ColumnComparisionResult].LoadString(IDS_STRING_COLUMN_COMPARISON_RESULT);
	titles[ColumnLength1].LoadString(IDS_STRING_COLUMN_1ST_LENGTH);
	titles[ColumnLength2].LoadString(IDS_STRING_COLUMN_2ND_LENGTH);

	ASSERT(MaxColumns == countof(m_ItemToColumn));
	ASSERT(MaxColumns == countof(m_ColumnToItem));
	ASSERT(MaxColumns == countof(m_ColumnArray));

	for (i = 0; i < countof(m_ColumnToItem); i++)
	{
		m_ColumnToItem[i] = MaxColumns;
		m_ItemToColumn[i] = MaxColumns;
	}

	int col = 0;    // real column
	for (int j = 0; j < countof(m_ColumnArray); j++)
	{
		// j is column number (including hidden ones)
		for (i = 0; i < countof(m_ColumnArray); i++)
		{
			// i is subitem number
			if (! pDoc->m_bRecurseSubdirs
				&& ColumnSubdir == i)
			{
				continue;
			}
			if (m_ColumnArray[i] == j && m_ColumnWidthArray[i] > 0)
			{
				TRACE(_T("Inserting column \"%s\" %d, subitem %d\n"),
					LPCTSTR(titles[i]), col, i);
				pListCtrl->InsertColumn(col, titles[i], LVCFMT_LEFT, m_ColumnWidthArray[i], i);

				m_ColumnToItem[col] = eColumns(i);
				m_ItemToColumn[i] = col;

				// set sort direction
				if (i == m_SortColumns[0])
				{
					HDITEM hdi;
					hdi.mask = HDI_FORMAT;
					pHeader->GetItem(col, & hdi);
					if (m_AscendingSortOrder[0])
					{
						hdi.fmt &= ~HDF_SORTUP;
						hdi.fmt |= HDF_SORTDOWN;
					}
					else
					{
						hdi.fmt &= ~HDF_SORTDOWN;
						hdi.fmt |= HDF_SORTUP;
					}
					pHeader->SetItem(col, & hdi);
				}

				col++;
				break;
			}
		}
	}

	BuildSortedPairArray(m_PairArray, pDoc->m_pPairList, pDoc->m_nFilePairs);

	pListCtrl->SetItemCount(m_PairArray.size());

	for (unsigned item = 0; item < m_PairArray.size(); item++)
	{
		FilePair * pPair = m_PairArray[item];
		AddListViewItem(pPair, item);
	}
	pListCtrl->SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);
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
		if (unsigned(pNmlv->iItem) < m_PairArray.size())
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
	unsigned nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
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
	unsigned nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
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
	unsigned nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
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
	unsigned nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
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
	unsigned nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
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

	CListCtrl * pListCtrl = & GetListCtrl();

	CHeaderCtrl * pHeader = pListCtrl->GetHeaderCtrl();

	CRect HeaderRect;
	pHeader->GetWindowRect( & HeaderRect);

	UINT MenuId = IDR_MENU_LISTVIEW_CONTEXT;

	if (HeaderRect.PtInRect(point))
	{
		MenuId = IDR_MENU_LISTVIEW_HEADER;
	}

	if (menu.LoadMenu(MenuId))
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
	unsigned nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
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
	unsigned nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	while (-1 != nItem)
	{
		if (nItem >= m_PairArray.size())
		{
			break;
		}
		FilePair * pPair = m_PairArray[nItem];
		if (pPair->m_ComparisionResult == pPair->ResultUnknown
			|| (pPair->pFirstFile != NULL && pPair->pFirstFile->IsFolder())
			|| (pPair->pSecondFile != NULL && pPair->pSecondFile->IsFolder()))
		{
			nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
			continue;
		}
		if (NULL != pCmdUI->m_pMenu)
		{
			pCmdUI->m_pMenu->SetDefaultItem(ID_LISTVIEW_OPEN, FALSE);
		}
		pCmdUI->Enable(TRUE);
		return;
	}
	pCmdUI->Enable(FALSE);
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

	unsigned nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
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
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.state = 0;
	lvi.stateMask = 0;
	if (pFileItem->IsFolder())
	{
		lvi.pszText = _T("");
	}
	else
	{
		lvi.pszText = (LPTSTR)pFileItem->GetName();
	}
	lvi.lParam = LPARAM(pFileItem);
	if (pPair->m_bSelected)
	{
		lvi.state = LVIS_SELECTED;
		lvi.stateMask = LVIS_SELECTED;
	}
	pListCtrl->InsertItem(& lvi);

	if (m_ColumnWidthArray[ColumnSubdir] >= 0)
	{
		if (pFileItem->IsFolder())
		{
			pListCtrl->SetItemText(item, m_ItemToColumn[ColumnSubdir], pFileItem->GetName());
		}
		else
		{
			pListCtrl->SetItemText(item, m_ItemToColumn[ColumnSubdir], pFileItem->GetSubdir());
		}
	}
	// set modified time/date
	if (m_ColumnWidthArray[ColumnDate1] >= 0)
	{
		CString datetime;
		if (NULL != pPair->pFirstFile && ! pPair->pFirstFile->IsFolder())
		{
			datetime = FileTimeToStr(pPair->pFirstFile->GetLastWriteTime());
			pListCtrl->SetItemText(item, m_ItemToColumn[ColumnDate1], datetime);
		}
		else
		{
			//pListCtrl->SetItemText(item, m_ItemToColumn[ColumnDate1], _T(""));
		}

		if (NULL != pPair->pSecondFile && ! pPair->pSecondFile->IsFolder())
		{
			datetime = FileTimeToStr(pPair->pSecondFile->GetLastWriteTime());
			pListCtrl->SetItemText(item, m_ItemToColumn[ColumnDate2], datetime);
		}
		else
		{
			//pListCtrl->SetItemText(item, m_ItemToColumn[ColumnDate2], _T(""));
		}
	}
	if (m_ColumnWidthArray[ColumnLength1] >= 0)
	{
		CString Length;
		if (NULL != pPair->pFirstFile && ! pPair->pFirstFile->IsFolder())
		{
			Length = FileLengthToStr(pPair->pFirstFile->GetFileLength());
			pListCtrl->SetItemText(item, m_ItemToColumn[ColumnLength1], Length);
		}
		else
		{
			//pListCtrl->SetItemText(item, m_ItemToColumn[ColumnLength1], _T(""));
		}

		if (NULL != pPair->pSecondFile && ! pPair->pSecondFile->IsFolder())
		{
			Length = FileLengthToStr(pPair->pSecondFile->GetFileLength());
			pListCtrl->SetItemText(item, m_ItemToColumn[ColumnLength2], Length);
		}
		else
		{
			//pListCtrl->SetItemText(item, m_ItemToColumn[ColumnLength2], _T(""));
		}
	}

	CString ComparisionResult = pPair->GetComparisionResult();
	pListCtrl->SetItemText(item, m_ItemToColumn[ColumnComparisionResult], ComparisionResult);
}

void CAlegrDiffView::OnFileSaveList()
{
	CSaveFileListDlg dlg;
	CListCtrl * pListCtrl = &GetListCtrl();
	CAlegrDiffDoc * pDoc = GetDocument();
	if (IDOK != dlg.DoModal())
	{
		return;
	}
	FILE * file = _tfopen(dlg.m_sFilename, _T("wt"));
	if (NULL == file)
	{
		CString s;
		s.Format(IDS_UNABLE_TO_CREATE_FILE, LPCTSTR(dlg.m_sFilename));
		AfxMessageBox(s);
		return;
	}
	CString s1;
	s1.LoadString(IDS_DIFF_FILE_BANNER);
	_ftprintf(file, s1, LPCTSTR(pDoc->m_sFirstDir), LPCTSTR(pDoc->m_sSecondDir));

	int MaxNameLength = 0;
	int MaxDateTimeLength = 0;
	for (int pass = 0; pass < 2; pass++)
	{
		for (unsigned item = 0; item < m_PairArray.size(); item++)
		{
			FilePair * pFilePair = m_PairArray[item];
			if (1 == dlg.m_IncludeFilesSelect)  // selected files
			{
				if ( ! pListCtrl->GetItemState(item, LVIS_SELECTED))
				{
					continue;
				}
			}
			else if (2 == dlg.m_IncludeFilesSelect) // include groups
			{
				switch (pFilePair->m_ComparisionResult)
				{
				case pFilePair->FilesIdentical:
					if ( ! dlg.m_bIncludeIdenticalFiles)
					{
						continue;
					}
					break;
				case pFilePair->DifferentInSpaces:
					if ( ! dlg.m_bIncludeDifferentInBlanksFiles)
					{
						continue;
					}
					break;
				case pFilePair->VersionInfoDifferent:
					// TODO, but now just different
				case pFilePair->FilesDifferent:
					if ( ! dlg.m_bIncludeDifferentFiles)
					{
						continue;
					}
					break;
				case pFilePair->OnlyFirstFile:
					if ( ! dlg.m_bIncludeFolder1OnlyFiles)
					{
						continue;
					}
					break;
				case pFilePair->OnlySecondFile:
					if ( ! dlg.m_bIncludeFolder2OnlyFiles)
					{
						continue;
					}
					break;
				}
			}
			// else: all files
			FileItem * pItem = pFilePair->pFirstFile;
			if (NULL == pItem)
			{
				pItem = pFilePair->pSecondFile;
				if (NULL == pItem)
				{
					continue;
				}
			}

			if (pass != 0)
			{
				CString time1, time2, line;
				if (dlg.m_bIncludeSubdirectoryName)
				{
					line += pItem->GetSubdir();
				}
				line += pItem->GetName();
				line += CString(' ', MaxNameLength + 4 - line.GetLength());

				if (dlg.m_bIncludeTimestamp)
				{
					if (NULL != pFilePair->pFirstFile)
					{
						time1 = FileTimeToStr(pFilePair->pFirstFile->GetLastWriteTime());
					}
					time1 += CString(' ', MaxDateTimeLength + 4 - time1.GetLength());

					if (NULL != pFilePair->pSecondFile)
					{
						time2 = FileTimeToStr(pFilePair->pSecondFile->GetLastWriteTime());
					}
					time2 += CString(' ', MaxDateTimeLength + 4 - time2.GetLength());

					line += time1;
					line += time2;
				}
				if (dlg.m_bIncludeComparisonResult)
				{
					line += pFilePair->GetComparisionResult();
				}
				line.TrimRight();
				line += '\n';
				_fputts(line, file);
			}
			else
			{
				int NameLength = pItem->GetNameLength();
				if (dlg.m_bIncludeSubdirectoryName)
				{
					NameLength += pItem->GetSubdirLength();
				}
				if (NameLength > MaxNameLength)
				{
					MaxNameLength = NameLength;
				}
				if (NULL != pFilePair->pFirstFile)
				{
					int DateLength = FileTimeToStr(pFilePair->pFirstFile->GetLastWriteTime()).GetLength();
					if (DateLength > MaxDateTimeLength)
					{
						MaxDateTimeLength = DateLength;
					}
				}
				if (NULL != pFilePair->pSecondFile)
				{
					int DateLength = FileTimeToStr(pFilePair->pSecondFile->GetLastWriteTime()).GetLength();
					if (DateLength > MaxDateTimeLength)
					{
						MaxDateTimeLength = DateLength;
					}
				}
			}
		}
	}
	fclose(file);
}

void CAlegrDiffView::OnUpdateFileSaveList(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_PairArray.size() != 0);
}

void CAlegrDiffView::OnViewHideselectedfiles()
{
	CListCtrl * pListCtrl = &GetListCtrl();
	unsigned nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		return;
	}
	while(-1 != nItem)
	{
		if (nItem < m_PairArray.size())
		{
			m_PairArray[nItem]->m_bHideFromListView = true;
		}
		nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
	}
	GetDocument()->UpdateAllViews(NULL);
}

void CAlegrDiffView::OnUpdateViewHideselectedfiles(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetListCtrl().GetNextItem(-1, LVNI_SELECTED) != -1);
}

void CAlegrDiffView::OnViewShowallfiles()
{
	CAlegrDiffDoc * pDoc = GetDocument();
	FilePair * pPair;
	for (pPair = pDoc->m_pPairList; pPair != NULL; pPair = pPair->pNext)
	{
		pPair->m_bHideFromListView = false;
		pPair->m_bSelected = true;
	}
	pDoc->UpdateAllViews(NULL);
	for (pPair = pDoc->m_pPairList; pPair != NULL; pPair = pPair->pNext)
	{
		pPair->m_bSelected = false;
	}
}

void CAlegrDiffView::OnUpdateViewShowallfiles(CCmdUI* pCmdUI)
{
	for (FilePair * pPair = GetDocument()->m_pPairList; pPair != NULL; pPair = pPair->pNext)
	{
		if (pPair->m_bHideFromListView)
		{
			pCmdUI->Enable(TRUE);
			return;
		}
	}
	pCmdUI->Enable(FALSE);
}


void CAlegrDiffView::OnListviewFilelength()
{
	m_ColumnWidthArray[ColumnLength1] = ~m_ColumnWidthArray[ColumnLength1];
	m_ColumnWidthArray[ColumnLength2] = ~m_ColumnWidthArray[ColumnLength2];
	OnUpdate(NULL, 0, NULL);
}

void CAlegrDiffView::OnUpdateListviewFilelength(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ColumnWidthArray[ColumnLength1] >= 0);
}

void CAlegrDiffView::OnListviewModificationtime()
{
	m_ColumnWidthArray[ColumnDate1] = ~m_ColumnWidthArray[ColumnDate1];
	m_ColumnWidthArray[ColumnDate2] = ~m_ColumnWidthArray[ColumnDate2];
	OnUpdate(NULL, 0, NULL);
}

void CAlegrDiffView::OnUpdateListviewModificationtime(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ColumnWidthArray[ColumnDate1] >= 0);
}

void CAlegrDiffView::UpdateAppSort()
{
	CThisApp * pApp = GetApp();

	for (int i = 0; i < MaxColumns; i++)
	{
		pApp->m_ColumnSort[i] = m_SortColumns[i];
		if ( ! m_AscendingSortOrder[i])
		{
			pApp->m_ColumnSort[i] |= 0x80;
		}
	}
}

void CAlegrDiffView::OnListviewSortby1stlength()
{
	SetSortColumn(ColumnLength1, SetSortColumnAscending);
}

void CAlegrDiffView::OnUpdateListviewSortby1stlength(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(ColumnLength1 == m_SortColumns[0]);
}

void CAlegrDiffView::OnListviewSortby1stmodificationdate()
{
	SetSortColumn(ColumnDate1, SetSortColumnAscending);
}

void CAlegrDiffView::OnUpdateListviewSortby1stmodificationdate(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(ColumnDate1 == m_SortColumns[0]);
}

void CAlegrDiffView::OnListviewSortby2ndlength()
{
	SetSortColumn(ColumnLength2, SetSortColumnAscending);
}

void CAlegrDiffView::OnUpdateListviewSortby2ndlength(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(ColumnLength2 == m_SortColumns[0]);
}

void CAlegrDiffView::OnListviewSortbyComparisonresult()
{
	SetSortColumn(ColumnComparisionResult, SetSortColumnAscending);
}

void CAlegrDiffView::OnUpdateListviewSortbyComparisonresult(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(ColumnComparisionResult == m_SortColumns[0]);
}

void CAlegrDiffView::OnListviewSortbyFolder()
{
	SetSortColumn(ColumnSubdir, SetSortColumnAscending);
}

void CAlegrDiffView::OnUpdateListviewSortbyFolder(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(ColumnSubdir == m_SortColumns[0]);
}

void CAlegrDiffView::OnListviewSortbyName()
{
	SetSortColumn(ColumnName, SetSortColumnAscending);
}

void CAlegrDiffView::OnUpdateListviewSortbyName(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(ColumnName == m_SortColumns[0]);
}

void CAlegrDiffView::OnListviewSortbyDescendingorder()
{
	SetSortColumn(m_SortColumns[0], SetSortColumnDescending);
}

void CAlegrDiffView::OnUpdateListviewSortbyDescendingorder(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( ! m_AscendingSortOrder[0]);
}

void CAlegrDiffView::OnListviewSortby2ndmodificationdate()
{
	SetSortColumn(ColumnDate2, SetSortColumnAscending);
}

void CAlegrDiffView::OnUpdateListviewSortby2ndmodificationdate(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(ColumnDate2 == m_SortColumns[0]);
}

void CAlegrDiffView::OnHdnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here
	if (0 == phdr->iItem)
	{
		// cannot move item 0
		*pResult = TRUE;
	}
	else
	{
		*pResult = 0;
	}
}

void CAlegrDiffView::OnHdnEnddrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	Invalidate();

	if (NULL != phdr->pitem
		&& 0 != (phdr->pitem->mask & HDI_ORDER)
		&& 0 == phdr->pitem->iOrder)
	{
		// don't move the column to make it leftmost
		*pResult = TRUE;
	}
	else
	{
		*pResult = 0;
	}
	// column order not changed yet
}
