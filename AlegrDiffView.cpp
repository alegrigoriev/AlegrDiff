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
#include "FilesPropertiesDialog.h"
#include <shlwapi.h>
#include <atlpath.h>

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
	ON_UPDATE_COMMAND_UI(ID_LISTVIEW_OPEN1, OnUpdateListviewOpen)
	ON_COMMAND(ID_LISTVIEW_OPEN1, OnListviewOpen)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
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
	ON_NOTIFY(HDN_TRACK, 0, OnHdnTrack)
	ON_NOTIFY(HDN_BEGINTRACK, 0, OnHdnBeginTrack)
	ON_NOTIFY(HDN_ENDTRACK, 0, OnHdnEndTrack)
	ON_COMMAND(ID_VIEW_RESETCOLUMNS, OnViewResetcolumns)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_COMMAND(ID_FILE_PROPERTIES, OnFileProperties)
	ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateFileProperties)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnLvnItemchanged)
	ON_NOTIFY_REFLECT(LVN_ODSTATECHANGED, OnLvnOdItemchanged)
	ON_NOTIFY(LVN_ODSTATECHANGED, 0, OnLvnOdItemchanged)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_SHOW_DIFFERENTFILES, OnShowDifferentfiles)
	ON_UPDATE_COMMAND_UI(ID_SHOW_DIFFERENTFILES, OnUpdateShowDifferentfiles)
	ON_COMMAND(ID_SHOW_DIFFERENTINVERSIONINFO, OnShowDifferentinversioninfo)
	ON_UPDATE_COMMAND_UI(ID_SHOW_DIFFERENTINVERSIONINFO, OnUpdateShowDifferentinversioninfo)
	ON_COMMAND(ID_SHOW_IDENTICALFILES, OnShowIdenticalfiles)
	ON_UPDATE_COMMAND_UI(ID_SHOW_IDENTICALFILES, OnUpdateShowIdenticalfiles)
	ON_COMMAND(ID_SHOW_FILESIN2NDDIRECTORYONLY, OnShowFilesin2nddirectoryonly)
	ON_UPDATE_COMMAND_UI(ID_SHOW_FILESIN2NDDIRECTORYONLY, OnUpdateShowFilesin2nddirectoryonly)
	ON_COMMAND(ID_SHOW_FILESIN1STDIRECTORYONLY, OnShowFilesin1stdirectoryonly)
	ON_UPDATE_COMMAND_UI(ID_SHOW_FILESIN1STDIRECTORYONLY, OnUpdateShowFilesin1stdirectoryonly)
	ON_COMMAND(ID_SHOW_SUBDIRECTORIESIN1STDIRONLY, OnShowSubdirectoriesin1stdironly)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SUBDIRECTORIESIN1STDIRONLY, OnUpdateShowSubdirectoriesin1stdironly)
	ON_COMMAND(ID_SHOW_SUBDIRECTORIESIN2NDDIR, OnShowSubdirectoriesin2nddir)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SUBDIRECTORIESIN2NDDIR, OnUpdateShowSubdirectoriesin2nddir)
	ON_COMMAND(ID_SHOW_DIFFERENTINSPACESONLY, OnShowDifferentinspacesonly)
	ON_UPDATE_COMMAND_UI(ID_SHOW_DIFFERENTINSPACESONLY, OnUpdateShowDifferentinspacesonly)
	ON_COMMAND(ID_SHOW_LONGERFILESIN1STDIRECTORY, OnShowLongerfilesin1stdirectory)
	ON_UPDATE_COMMAND_UI(ID_SHOW_LONGERFILESIN1STDIRECTORY, OnUpdateShowLongerfilesin1stdirectory)
	ON_COMMAND(ID_SHOW_LONGERFILESIN2NDDIRECTORY, OnShowLongerfilesin2nddirectory)
	ON_UPDATE_COMMAND_UI(ID_SHOW_LONGERFILESIN2NDDIRECTORY, OnUpdateShowLongerfilesin2nddirectory)
	ON_COMMAND(ID_INFIRSTDIRECTORYONLY_SUBDIRECTORIESCONTENTS, OnInFirstDirectoryOnlySubdirectoriesContents)
	ON_UPDATE_COMMAND_UI(ID_INFIRSTDIRECTORYONLY_SUBDIRECTORIESCONTENTS, OnUpdateInFirstDirectoryOnlySubdirectoriesContents)
	ON_COMMAND(ID_INSECONDDIRECTORY_SUBDIRECTORIESCONTENTS, OnInSecondDirectoryOnlySubdirectoriesContents)
	ON_UPDATE_COMMAND_UI(ID_INSECONDDIRECTORY_SUBDIRECTORIESCONTENTS, OnUpdateInSecondDirectoryOnlySubdirectoriesContents)
	//}}AFX_MSG_MAP
	// Standard printing commands
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

CAlegrDiffView::CAlegrDiffView() noexcept
	: m_PresentFilesMask(0)
{
	CThisApp const * const pApp = GetApp();
	m_ShowFilesMask = pApp->m_ShowFilesMask & ~(1 << FilePair::DirectoriesBothPresent);

	if (m_ShowFilesMask & ShowIdenticalFiles)
	{
		// add missing bit in this mask
		m_ShowFilesMask |= ShowIdenticalFiles;
	}

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
	for (FilePair* pPair : m_PairArray)
	{
		pPair->m_ListSortOrder = ULONG_MAX;
	}
	GetApp()->m_ShowFilesMask = m_ShowFilesMask;
}

BOOL CAlegrDiffView::PreCreateWindow(CREATESTRUCT& cs)
{
	// Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style |= LVS_SHOWSELALWAYS | LVS_REPORT | LVS_OWNERDATA;
	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView drawing

void CAlegrDiffView::OnDraw(CDC* /*pDC*/)
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

	pList->SetExtendedStyle(pList->GetExtendedStyle()
							| LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP
							| LVS_EX_HEADERDRAGDROP
							| LVS_EX_INFOTIP);

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

void CAlegrDiffView::ResetColumnsArray() noexcept
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
	m_ColumnWidthArray[ColumnLength1] = 150;
	m_ColumnWidthArray[ColumnLength2] = 150;
	m_ColumnWidthArray[ColumnComparisionResult] = 400;

}

bool CAlegrDiffView::ChangeSortItem(eColumns nColumn, eSetSortColumnOrder Order) noexcept
{
	// if actual sort order changed, return 'true'
	if (nColumn >= MaxColumns)
	{
		return false;
	}

	if (m_SortColumns[0] == nColumn)
	{
		if (Order != SetSortColumnMouseClick
			&& (SetSortColumnUnchanged == Order
				|| (Order == SetSortColumnAscending) == m_AscendingSortOrder[0]))
		{
			// sort order remains the same
			return false;
		}
		m_AscendingSortOrder[0] = ! m_AscendingSortOrder[0];
		return true;
	}
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
		else if (Order == SetSortColumnMouseClick
				|| SetSortColumnUnchanged == Order)
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

	return true;
}

void CAlegrDiffView::SetSortColumn(eColumns nColumn, eSetSortColumnOrder Order)
{
	if (ChangeSortItem(nColumn, Order))
	{
		UpdateAppColumns();

		OnUpdate(NULL, OnUpdateRebuildListView, NULL);
	}
}

void CAlegrDiffView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// change sort order
	unsigned nColumn = pNMListView->iSubItem;
	if (nColumn >= countof (m_ViewItemToColumnType))
	{
		return;
	}

	SetSortColumn(m_ViewItemToColumnType[nColumn], SetSortColumnMouseClick);

	*pResult = 0;
}

void CAlegrDiffView::BuildSortedPairArray(vector<FilePair *> & PairArray, FilePairList * pPairList)
{
	PairArray.clear();
	PairArray.reserve(pPairList->NumFilePairs);

	FilePair * pPair = pPairList->First();

	m_PresentFilesMask = 0;
	unsigned PairCount = 0;

	while (pPairList->NotEnd(pPair))
	{
		pPair->m_ListSortOrder = ULONG_MAX;
		if (pPair->m_bDeleted)
		{
			FilePair* tmp = pPair;
			pPair = pPair->Next();

			pPairList->RemovePair(tmp);
			continue;
		}

		// set mask bit
		m_PresentFilesMask |= 1 << pPair->GetComparisonResult();

		if (!pPair->m_bHideFromListView
			&& (m_ShowFilesMask & (1 << pPair->GetComparisonResult())))
		{
			PairArray.push_back(pPair);
			PairCount++;
		}

		pPair = pPairList->Next(pPair);
	}

	std::sort(PairArray.begin(), PairArray.end(),
			FilePairComparePredicate(m_SortColumns, m_AscendingSortOrder, countof(m_SortColumns)));

	for (unsigned i = 0; i < PairCount; i++)
	{
		PairArray[i]->m_ListSortOrder = i;
	}
}

void CAlegrDiffView::BuildArrowBitmaps()
{
	if (_AfxGetComCtlVersion() >= 0x00060000)
	{
		return;
	}

	CHeaderCtrl* pHeader = GetListCtrl().GetHeaderCtrl();
	CFont* pFont = pHeader->GetFont();
	CDC* pDC = GetWindowDC();
	CDC cdc;
	cdc.CreateCompatibleDC(pDC);

	CFont * pOldFont = pDC->SelectObject(pFont);

	TEXTMETRIC tm;
	pDC->GetTextMetrics( & tm);
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	unsigned const ArrowWidth = (tm.tmAscent - tm.tmInternalLeading) | 1;
	unsigned const ArrowHeight = ArrowWidth / 2 + 1;

	int const ArrayWidth = ((ArrowWidth / 8) + 1) & ~1;
	int const ArraySize = ArrowHeight * 2 * ArrayWidth;

	UCHAR * pBmp = new UCHAR[ArraySize];
	if (pBmp)
	{
		memset(pBmp, 0xFF, ArraySize);

		for (unsigned y = 0; y < ArrowHeight; y++)
		{
			for (unsigned x = y; x < ArrowWidth - y; x++)
			{
				pBmp[y * ArrayWidth + x / 8] &= 0xFF7F >> (x & 7);
				pBmp[(ArrowHeight * 2 - 1 - y) * ArrayWidth + x / 8] &= 0xFF7F >> (x & 7);
			}
		}

		m_ArrowDownBitmap.DeleteObject();
		m_ArrowUpBitmap.DeleteObject();

		m_ArrowDownBitmap.CreateBitmap(ArrowWidth, ArrowHeight, 1, 1, pBmp);
		m_ArrowUpBitmap.CreateBitmap(ArrowWidth, ArrowHeight, 1, 1, pBmp + ArrowHeight * ArrayWidth);

		delete[] pBmp;
	}
}

void CAlegrDiffView::BuildListViewHeader()
{
	CAlegrDiffDoc* pDoc = GetDocument();
	CListCtrl* pListCtrl = &GetListCtrl();
	CHeaderCtrl* pHeader = pListCtrl->GetHeaderCtrl();
	// Delete all of the items.
	int i;
	for (i = pHeader->GetItemCount(); i > 0; i--)
	{
		pListCtrl->DeleteColumn(i - 1);
	}

	BuildArrowBitmaps();

	CString titles[MaxColumns];

	titles[ColumnName].LoadString(IDS_STRING_COLUMN_FILENAME);
	titles[ColumnSubdir].LoadString(IDS_STRING_COLUMN_SUBDIRECTORY);
	titles[ColumnComparisionResult].LoadString(IDS_STRING_COLUMN_COMPARISON_RESULT);

	if (pDoc->m_bCheckingFingerprint)
	{
		titles[ColumnDate1].LoadString(IDS_STRING_COLUMN_ORIGINAL_MODIFIED);
		titles[ColumnDate2].LoadString(IDS_STRING_COLUMN_CURRENT_MODIFIED);
		titles[ColumnLength1].LoadString(IDS_STRING_COLUMN_ORIGINAL_LENGTH);
		titles[ColumnLength2].LoadString(IDS_STRING_COLUMN_CURRENT_LENGTH);
	}
	else
	{
		titles[ColumnDate1].LoadString(IDS_STRING_COLUMN_1ST_MODIFIED);
		titles[ColumnDate2].LoadString(IDS_STRING_COLUMN_2ND_MODIFIED);
		titles[ColumnLength1].LoadString(IDS_STRING_COLUMN_1ST_LENGTH);
		titles[ColumnLength2].LoadString(IDS_STRING_COLUMN_2ND_LENGTH);
	}

	ASSERT(MaxColumns == countof(m_ColumnTypeToViewItem));
	ASSERT(MaxColumns == countof(m_ViewItemToColumnType));
	ASSERT(MaxColumns == countof(m_ColumnArray));

	for (i = 0; i < countof(m_ViewItemToColumnType); i++)
	{
		m_ViewItemToColumnType[i] = MaxColumns;
		m_ColumnTypeToViewItem[i] = MaxColumns;
	}

	int col = 0;    // real column

	// if the current sort column is not visible, set it to Name
	if (m_SortColumns[0] >= MaxColumns
		|| m_ColumnWidthArray[m_SortColumns[0]] <= 0
		|| (! pDoc->m_bRecurseSubdirs
			&& ColumnSubdir == m_SortColumns[0]))
	{
		ChangeSortItem(ColumnName, SetSortColumnUnchanged);
	}

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
				pListCtrl->InsertColumn(col, titles[i],
										LVCFMT_BITMAP_ON_RIGHT | LVCFMT_LEFT, m_ColumnWidthArray[i], i);

				m_ViewItemToColumnType[col] = eColumns(i);
				m_ColumnTypeToViewItem[i] = col;

				// set sort direction arrow
				if (i == m_SortColumns[0])
				{
					HDITEM hdi;
					hdi.mask = HDI_FORMAT;
					pHeader->GetItem(col, & hdi);
					if (_AfxGetComCtlVersion() >= 0x00060000)
					{
						hdi.mask = HDI_FORMAT;
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
					}
					else
					{
						hdi.mask = HDI_BITMAP | HDI_FORMAT;
						hdi.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;

						if (m_AscendingSortOrder[0])
						{
							hdi.hbm = m_ArrowDownBitmap;
						}
						else
						{
							hdi.hbm = m_ArrowUpBitmap;
						}
					}
					pHeader->SetItem(col, & hdi);
				}

				col++;
				break;
			}
		}
		m_ColumnPositionToViewItem[j] = j;
	}
}

void CAlegrDiffView::ReBuildListView()
{
	CListCtrl * pListCtrl = &GetListCtrl();
	CAlegrDiffDoc * pDoc = GetDocument();

	// fill the list control
	pListCtrl->DeleteAllItems();

	BuildListViewHeader();

	BuildSortedPairArray(m_PairArray, &pDoc->m_PairList);
	pListCtrl->SetCallbackMask(LVIS_CUT);

	pListCtrl->SetItemCount((int)m_PairArray.size());

	int nSel = -1;

	unsigned item = 0;
	for (FilePair* pPair : m_PairArray)
	{
		if (pPair->m_bFocused)
		{
			if (-1 == nSel)
			{
				nSel = item;
			}
			else
			{
				pPair->m_bFocused = false;
			}
		}
		item++;
	}

	if (!m_PairArray.empty())
	{
		if (-1 == nSel)
		{
			nSel = 0;
			m_PairArray[0]->m_bFocused = true;
		}
		pListCtrl->SetItemState(nSel, LVIS_FOCUSED, LVIS_FOCUSED);
		pListCtrl->EnsureVisible(nSel, false);
		pListCtrl->RedrawItems(0, item - 1);
	}
	UnlockWindowUpdate();
}

void CAlegrDiffView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
	if (UpdateViewsFilePairDeleteFromList == lHint
		|| OnUpdateListViewItem == lHint)
	{
		FilePairChangedArg * pArg = dynamic_cast<FilePairChangedArg *>(pHint);
		if (nullptr == pArg)
		{
			return;
		}
		FilePair const * const pPair = pArg->m_pPair;
		if (nullptr == pPair)
		{
			return;
		}

		if (ValidateFilePairIndex(pPair))
		{
			GetListCtrl().RedrawItems(pPair->m_ListSortOrder, pPair->m_ListSortOrder);
		}

		UpdateStatusText(WA_ACTIVE);
		return;
	}
	else if (UpdateViewsFilePairChanged == lHint)
	{
		FilePairChangedArg * pArg = dynamic_cast<FilePairChangedArg *>(pHint);
		if (nullptr == pArg)
		{
			return;
		}
		FilePair * pPair = pArg->m_pPair;
		if (nullptr == pPair)
		{
			return;
		}

		FilePair* const pNewPair = pArg->m_pNewPair;

		if (ValidateFilePairIndex(pPair))
		{
			if (pNewPair != nullptr)
			{
				pNewPair->m_ListSortOrder = pPair->m_ListSortOrder;
				m_PairArray[pPair->m_ListSortOrder] = pNewPair;
				pPair->m_ListSortOrder = ULONG_MAX;

				pPair = pNewPair;
			}

			GetListCtrl().RedrawItems(pPair->m_ListSortOrder, pPair->m_ListSortOrder);
			UpdateStatusText(WA_ACTIVE);

			pArg->m_pPair = nullptr;
		}

		return;
	}
	else if (OnUpdateRebuildListView != lHint)
	{
		return;
	}

	ReBuildListView();

	UpdateStatusText(WA_ACTIVE);
}

FilePair* CAlegrDiffView::GetFilePair(size_t index) const noexcept
{
	if (index >= m_PairArray.size())
	{
		return nullptr;
	}
	return m_PairArray[index];
}

FilePair* CAlegrDiffView::GetValidFilePair(size_t index) const noexcept
{
	if (index >= m_PairArray.size())
	{
		return nullptr;
	}
	FilePair* pPair = m_PairArray[index];
	if (pPair->m_bDeleted)
	{
		return nullptr;
	}
	return pPair;
}

FilePair* CAlegrDiffView::ValidateFilePairIndex(FilePair const* pPair) const noexcept
{
	if (pPair != nullptr
		&& pPair->m_ListSortOrder < m_PairArray.size())
	{
		FilePair* tmp = m_PairArray[pPair->m_ListSortOrder];
		if (tmp == pPair)
		{
			return tmp;
		}
	}
	return nullptr;
}

void CAlegrDiffView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW * pNmlv = (NMLISTVIEW *)pNMHDR;
	// open new view for the files
	// compare two files
	FilePair* pPair = GetValidFilePair(unsigned(pNmlv->iItem));
	if (pPair != nullptr)
	{
		GetApp()->OpenFilePairView(pPair);
	}

//    int nItem = p;
	*pResult = 0;
}

void CAlegrDiffView::OnReturn(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	CListCtrl * pListCtrl = &GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	while(-1 != nItem)
	{
		FilePair* pPair = GetValidFilePair(nItem);
		if (pPair != nullptr)
		{
			GetApp()->OpenFilePairView(pPair);
		}
		nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
	}
	*pResult = 0;
}

void CAlegrDiffView::OnFileEditFirst()
{
	CListCtrl * pListCtrl = &GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	FilePair* pPair = GetValidFilePair(nItem);
	if (pPair != nullptr)
	{
		OpenFileForEditing(pPair->pFirstFile);
	}
}

void CAlegrDiffView::OnUpdateFileEditFirst(CCmdUI* pCmdUI)
{
	CListCtrl * pListCtrl = &GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	FileItem * pFile = NULL;
	FilePair* pPair = GetValidFilePair(nItem);
	if (pPair != nullptr)
	{
		pFile = pPair->pFirstFile;
	}
	ModifyOpenFileMenu(pCmdUI, pFile,
						IDS_OPEN_FIRST_FILE_MENU, IDS_OPEN_FIRST_FILE_MENU_DISABLED);
}

void CAlegrDiffView::OnFileEditSecond()
{
	CListCtrl * pListCtrl = &GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	FilePair* pPair = GetValidFilePair(nItem);
	if (pPair != nullptr)
	{
		OpenFileForEditing(pPair->pSecondFile);
	}
}

void CAlegrDiffView::OnUpdateFileEditSecond(CCmdUI* pCmdUI)
{
	CListCtrl * pListCtrl = &GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	FileItem* pFile = NULL;
	FilePair* pPair = GetValidFilePair(nItem);
	if (pPair != nullptr)
	{
		pFile = pPair->pSecondFile;
	}
	ModifyOpenFileMenu(pCmdUI, pFile,
						IDS_OPEN_SECOND_FILE_MENU, IDS_OPEN_SECOND_FILE_MENU_DISABLED);
}

void CAlegrDiffView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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
	CListCtrl * const pListCtrl = &GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	while (-1 != nItem)
	{
		FilePair* const pPair = GetValidFilePair(nItem);
		if (pPair != nullptr)
		{
			GetApp()->OpenFilePairView(pPair);
		}
		nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
	}
}

void CAlegrDiffView::OnUpdateListviewOpen(_In_ CCmdUI* pCmdUI)
{
	CListCtrl * const pListCtrl = &GetListCtrl();
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	while (-1 != nItem)
	{
		FilePair* const pPair = GetValidFilePair(nItem);
		if (!pPair->HasContents())
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

void CAlegrDiffView::OnUpdateFileCopyFirstDir(_In_ CCmdUI* pCmdUI)
{
	// check if there is anything to copy
	CListCtrl * const pListCtrl = &GetListCtrl();

	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	while(-1 != nItem)
	{
		FilePair* const pPair = GetValidFilePair(nItem);
		if (nullptr != pPair && pPair->pFirstFile->HasContents())
		{
			CString s;
			s.Format(IDS_COPY_FROM_DIR_FORMAT, LPCTSTR(GetDocument()->m_sFirstDir));
			pCmdUI->SetText(s);
			return;
		}
		nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
	}
	pCmdUI->Enable(FALSE);
}

void CAlegrDiffView::OnFileCopyFirstDir()
{
	CopySelectedFiles(1);
}

void CAlegrDiffView::OnUpdateFileCopySecondDir(CCmdUI* pCmdUI)
{
	// check if there is anything to copy
	CListCtrl * const pListCtrl = &GetListCtrl();

	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	while(-1 != nItem)
	{
		FilePair* const pPair = GetValidFilePair(nItem);
		if (nullptr != pPair && pPair->pSecondFile->HasContents())
		{
			CString s;
			s.Format(IDS_COPY_FROM_DIR_FORMAT, LPCTSTR(GetDocument()->m_sSecondDir));
			pCmdUI->SetText(s);
			return;
		}
		nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
	}
	pCmdUI->Enable(FALSE);
}

void CAlegrDiffView::OnFileCopySecondDir()
{
	CopySelectedFiles(2);
}

BOOL CAlegrDiffView::CopySelectedFiles(int DirIndex)
{
//    CThisApp * pApp = GetApp();

	CListCtrl * const pListCtrl = &GetListCtrl();
	vector<FileItem *> FilesArray;

	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
	while(-1 != nItem)
	{
		FilePair* const pPair = GetValidFilePair(nItem);
		if (pPair != nullptr)
		{
			FileItem * pFile;
			if (DirIndex == 2)
			{
				pFile = pPair->pSecondFile;
			}
			else
			{
				pFile = pPair->pFirstFile;
			}
			if (NULL != pFile)
			{
				FilesArray.push_back(pFile);
			}
		}
		nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED);
	}
	if (FilesArray.empty())
	{
		return FALSE;
	}

	CopyFilesToFolder(& FilesArray.front(), (int)FilesArray.size(), true);
	return TRUE;
}

void CAlegrDiffView::OnFileSaveList()
{
	CSaveFileListDlg dlg;
	CListCtrl * pListCtrl = &GetListCtrl();
	CAlegrDiffDoc * pDoc = GetDocument();

	if (-1 != pListCtrl->GetNextItem(-1, LVNI_SELECTED))
	{
		dlg.EnableSelectedItems();
	}

	if (IDOK != dlg.DoModal())
	{
		return;
	}

	ATL::CPath DstFile(dlg.GetFilename());

	BOOL GenerateCsv = (0 == DstFile.GetExtension().CompareNoCase(_T(".csv")));

	FILE * file = NULL;
	_tfopen_s(&file, DstFile, _T("wt,ccs=UTF-8"));
	if (NULL == file)
	{
		CString s;
		s.Format(IDS_UNABLE_TO_CREATE_FILE, LPCTSTR(DstFile));
		AfxMessageBox(s);
		return;
	}

	if ( ! GenerateCsv)
	{
		CString s1;
		s1.LoadString(IDS_DIFF_FILE_BANNER);
		_ftprintf(file, s1, LPCTSTR(pDoc->m_sFirstDir), LPCTSTR(pDoc->m_sSecondDir));
	}

	int MaxNameLength = 0;
	int MaxDateTimeLength = 0;
	int MaxFileLengthLength = 0;

	CString line;
	CString tmp;

	for (int pass = 0; pass < (1 + ! GenerateCsv); pass++)
	{
		for (unsigned item = 0; item < m_PairArray.size(); item++)
		{
			FilePair * pFilePair = m_PairArray[item];
			if (dlg.IncludeSelectedFiles == dlg.GetFilesFilter())  // selected files
			{
				if ( ! pListCtrl->GetItemState(item, LVIS_SELECTED))
				{
					continue;
				}
			}
			else if (dlg.IncludeGroups == dlg.GetFilesFilter()) // include groups
			{
				switch (pFilePair->GetComparisonResult())
				{
				case FilePair::FilesIdentical:
				case FilePair::FilesAttributesIdentical:
					if (!dlg.IncludeIdenticalFiles())
					{
						continue;
					}
					break;
				case FilePair::DifferentInSpaces:
					if ( ! dlg.IncludeDifferentInSpacesFiles())
					{
						continue;
					}
					break;
				case FilePair::VersionInfoDifferent:
					if ( ! dlg.IncludeVersionInfoDifferentFiles())
					{
						continue;
					}
					break;
				case FilePair::FilesDifferent:
					if ( ! dlg.IncludeDifferentFiles())
					{
						continue;
					}
					break;
				case FilePair::OnlyFirstFile:
					if ( ! dlg.IncludeFolder1Files())
					{
						continue;
					}
					break;
				case FilePair::OnlySecondFile:
					if ( ! dlg.IncludeFolder2Files())
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

			int AddLength;
			line.Empty();

			if (GenerateCsv || pass != 0)
			{
				if (dlg.IncludeSubdirectoryName())
				{
					line = pItem->GetSubdir();
					line += pItem->GetName();
				}
				else
				{
					line = pItem->GetName();
				}

				if (GenerateCsv)
				{
					line += TCHAR(',');
				}
				else
				{
					AddLength = MaxNameLength + 4 - line.GetLength();
					if (AddLength > 0)
					{
						line += CString(' ', AddLength);
					}
				}

				if (dlg.IncludeTimestamp())
				{
					AddLength = MaxDateTimeLength + 4;
					if (NULL != pFilePair->pFirstFile)
					{
						tmp = FileTimeToStr(pFilePair->pFirstFile->GetLastWriteTime());
						AddLength -= tmp.GetLength();
						line += tmp;
					}

					if (GenerateCsv)
					{
						line += TCHAR(',');
					}
					else
					{
						if (AddLength > 0)
						{
							line += CString(' ', AddLength);
						}
					}


					AddLength = MaxDateTimeLength + 4;
					if (NULL != pFilePair->pSecondFile)
					{
						tmp = FileTimeToStr(pFilePair->pSecondFile->GetLastWriteTime());
						AddLength -= tmp.GetLength();
						line += tmp;
					}

					if (GenerateCsv)
					{
						line += TCHAR(',');
					}
					else
					{
						if (AddLength > 0)
						{
							line += CString(' ', AddLength);
						}
					}
				}

				if (dlg.IncludeLength())
				{
					if (GenerateCsv)
					{
						if (NULL != pFilePair->pFirstFile)
						{
							tmp.Format(_T("%I64d"), pFilePair->pFirstFile->GetFileLength());
							line += tmp;
						}
						line += TCHAR(',');
					}
					else
					{
						AddLength = MaxFileLengthLength + 4;
						if (NULL != pFilePair->pFirstFile)
						{
							tmp.Format(_T("%*I64d"), MaxFileLengthLength, pFilePair->pFirstFile->GetFileLength());
							AddLength -= tmp.GetLength();
							line += tmp;
						}
						if (AddLength > 0)
						{
							line += CString(' ', AddLength);
						}
					}


					if (GenerateCsv)
					{
						if (NULL != pFilePair->pSecondFile)
						{
							tmp.Format(_T("%I64d"), pFilePair->pSecondFile->GetFileLength());
							line += tmp;
						}
						line += TCHAR(',');
					}
					else
					{
						AddLength = MaxFileLengthLength + 4;
						if (NULL != pFilePair->pSecondFile)
						{
							tmp.Format(_T("%*I64d"), MaxFileLengthLength, pFilePair->pSecondFile->GetFileLength());
							AddLength -= tmp.GetLength();
							line += tmp;
						}
						if (AddLength > 0)
						{
							line += CString(' ', AddLength);
						}
					}
				}

				if (dlg.IncludeComparisonResult())
				{
					line += pFilePair->GetComparisonResultStr();
				}

				line.TrimRight();
				line += TCHAR('\n');
				_fputts(line, file);
			}
			else
			{
				int NameLength = pItem->GetNameLength();
				if (dlg.IncludeSubdirectoryName())
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

					tmp.Format(_T("%I64d"), pFilePair->pFirstFile->GetFileLength());
					int LengthLength = tmp.GetLength();
					if (LengthLength > MaxFileLengthLength)
					{
						MaxFileLengthLength = LengthLength;
					}
				}

				if (NULL != pFilePair->pSecondFile)
				{
					int DateLength = FileTimeToStr(pFilePair->pSecondFile->GetLastWriteTime()).GetLength();
					if (DateLength > MaxDateTimeLength)
					{
						MaxDateTimeLength = DateLength;
					}

					tmp.Format(_T("%I64d"), pFilePair->pSecondFile->GetFileLength());
					int LengthLength = tmp.GetLength();
					if (LengthLength > MaxFileLengthLength)
					{
						MaxFileLengthLength = LengthLength;
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
	int nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);

	if (-1 == nItem)
	{
		return;
	}

	for ( ; -1 != nItem; nItem = pListCtrl->GetNextItem(nItem, LVNI_SELECTED))
	{
		FilePair* pPair = GetValidFilePair(nItem);
		if (nullptr == pPair)
		{
			continue;
		}
		pPair->m_bHideFromListView = true;
		if (pPair->m_bFocused)
		{
			// move focus on a next item
			pPair->m_bFocused = false;
			FilePairVectorIterator ii;
			for (ii = m_PairArray.begin() + (nItem + 1); ii != m_PairArray.end(); ++ii)
			{
				if ( ! (*ii)->m_bHideFromListView)
				{
					(*ii)->m_bFocused = true;
					(*ii)->m_bSelected = true;
					break;
				}
			}
			if (ii == m_PairArray.end())
			{
				for (ii = m_PairArray.begin() + nItem; ii != m_PairArray.begin(); )
				{
					--ii;
					if ( ! (*ii)->m_bHideFromListView)
					{
						(*ii)->m_bFocused = true;
						(*ii)->m_bSelected = true;
						break;
					}
				}
			}
		}
	}
	GetDocument()->UpdateAllViews(NULL, OnUpdateRebuildListView);
}

void CAlegrDiffView::OnUpdateViewHideselectedfiles(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetListCtrl().GetNextItem(-1, LVNI_SELECTED) != -1);
}

void CAlegrDiffView::OnViewShowallfiles()
{
	CAlegrDiffDoc* const pDoc = GetDocument();
	for (FilePair * pPair = pDoc->GetFirstFilePair(); pDoc->FilePairNotEnd(pPair); pPair = pDoc->GetNextFilePair(pPair))
	{
		pPair->m_bHideFromListView = false;
		pPair->m_bSelected = true;
	}
	pDoc->UpdateAllViews(NULL, OnUpdateRebuildListView);
	for (FilePair * pPair = pDoc->GetFirstFilePair(); pDoc->FilePairNotEnd(pPair); pPair = pDoc->GetNextFilePair(pPair))
	{
		pPair->m_bSelected = false;
	}
}

void CAlegrDiffView::OnUpdateViewShowallfiles(CCmdUI* pCmdUI)
{
	CAlegrDiffDoc * const pDoc = GetDocument();
	for (FilePair * pPair = pDoc->GetFirstFilePair(); pDoc->FilePairNotEnd(pPair); pPair = pDoc->GetNextFilePair(pPair))
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
	UpdateAppColumns();
	OnUpdate(NULL, OnUpdateRebuildListView, NULL);
}

void CAlegrDiffView::OnUpdateListviewFilelength(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ColumnWidthArray[ColumnLength1] >= 0);
}

void CAlegrDiffView::OnListviewModificationtime()
{
	m_ColumnWidthArray[ColumnDate1] = ~m_ColumnWidthArray[ColumnDate1];
	m_ColumnWidthArray[ColumnDate2] = ~m_ColumnWidthArray[ColumnDate2];
	UpdateAppColumns();
	OnUpdate(NULL, OnUpdateRebuildListView, NULL);
}

void CAlegrDiffView::OnUpdateListviewModificationtime(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ColumnWidthArray[ColumnDate1] >= 0);
}

void CAlegrDiffView::UpdateAppColumns()
{
	CThisApp * pApp = GetApp();

	for (int i = 0; i < MaxColumns; i++)
	{
		pApp->m_ColumnSort[i] = (UCHAR)m_SortColumns[i];
		if ( ! m_AscendingSortOrder[i])
		{
			pApp->m_ColumnSort[i] |= 0x80;
		}
		pApp->m_ColumnWidthArray[i] = (SHORT)m_ColumnWidthArray[i];
		pApp->m_ColumnArray[i] = (UCHAR)m_ColumnArray[i];
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

	*pResult = TRUE;

	if (NULL == phdr
		|| NULL == phdr->pitem)
	{
		return;
	}
	int ItemToMove = phdr->iItem;
	int NewItemPosition = phdr->pitem->iOrder;

	if (ItemToMove >= MaxColumns
		|| 0 == (phdr->pitem->mask & HDI_ORDER)
		|| 0 == NewItemPosition
		|| NewItemPosition >= MaxColumns)
	{
		return;
	}
	// real column order not changed yet
	// rearrange ColumnArray. Don't change ItemToColumn, ColumnToItem

	eColumns nColumn = m_ViewItemToColumnType[ItemToMove];
	int OldPosition = m_ColumnArray[nColumn];

	nColumn = m_ViewItemToColumnType[m_ColumnPositionToViewItem[NewItemPosition]];
	int NewPosition = m_ColumnArray[nColumn];

	TRACE("phdr->iItem=%d becomes phdr->pitem->iOrder=%d\n",
		ItemToMove, NewItemPosition);

	int i;
	int NumColumns = GetListCtrl().GetHeaderCtrl()->GetItemCount();

	// remove item from the old position
	for (i = 0; i < NumColumns; i++)
	{
		if (m_ColumnPositionToViewItem[i] == ItemToMove)
		{
			break;
		}
	}
	ASSERT(i < NumColumns);
	for ( ; i < NumColumns - 1; i++)
	{
		m_ColumnPositionToViewItem[i] = m_ColumnPositionToViewItem[i + 1];
	}
	// insert an item to new position
	for (i = NumColumns - 1; i > NewItemPosition; i--)
	{
		m_ColumnPositionToViewItem[i] = m_ColumnPositionToViewItem[i - 1];
	}
	m_ColumnPositionToViewItem[NewItemPosition] = ItemToMove;

	// rearrange ColumnArray. Don't change ItemToColumn, ColumnToItem
	for (i = 0; i < MaxColumns; i++)
	{
		if (m_ColumnArray[i] == OldPosition)
		{
			m_ColumnArray[i] = NewPosition;
		}
		else
		{
			if (m_ColumnArray[i] > OldPosition)
			{
				m_ColumnArray[i]--;
			}
			if (m_ColumnArray[i] >= NewPosition)
			{
				m_ColumnArray[i]++;
			}
		}
		TRACE("Column type %d becomes number %d\n", i, m_ColumnArray[i]);
	}

	Invalidate();
	UpdateAppColumns();
	*pResult = 0;
}

void CAlegrDiffView::OnHdnTrack(NMHDR * /*pNMHDR*/, LRESULT * pResult)
{
	*pResult = 0;
}

void CAlegrDiffView::OnHdnBeginTrack(NMHDR * /*pNMHDR*/, LRESULT * pResult)
{
	*pResult = 0;
}

void CAlegrDiffView::OnHdnEndTrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	CListCtrl * pList = &GetListCtrl();

	if (phdr->iItem < MaxColumns
		&& m_ViewItemToColumnType[phdr->iItem] < MaxColumns)
	{
		int item = m_ViewItemToColumnType[phdr->iItem];
		m_ColumnWidthArray[item] = pList->GetColumnWidth(phdr->iItem);
		UpdateAppColumns();
	}
	*pResult = 0;
}

void CAlegrDiffView::OnViewResetcolumns()
{
	ResetColumnsArray();
	UpdateAppColumns();
	OnUpdate(NULL, OnUpdateRebuildListView, NULL);
}

void CAlegrDiffView::OnFileProperties()
{
	UINT item = GetListCtrl().GetNextItem(-1, LVNI_SELECTED);
	if (item != -1
		&& item < m_PairArray.size())
	{
		CFilesPropertiesDialog dlg(m_PairArray[item]);

		dlg.DoModal();
	}
}

void CAlegrDiffView::OnUpdateFileProperties(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetListCtrl().GetNextItem(-1, LVNI_SELECTED) != -1);
}

static const char* LvStateString(UINT state)
{
	switch (state & (LVIS_SELECTED | LVIS_FOCUSED))
	{
	case LVIS_SELECTED | LVIS_FOCUSED:
		return "SELECTED+FOCUSED";
	case LVIS_SELECTED:
		return "SELECTED";
	case LVIS_FOCUSED:
		return "FOCUSED";
	case 0:
	default:
		return "NONE";
	}
}

void CAlegrDiffView::SetItemsState(size_t from, size_t to, UINT OldState, UINT NewState)
{
	if (to < m_PairArray.size())
	{
		for (size_t i = from; i <= to; i++)
		{
			if (OldState & LVIS_SELECTED)
			{
				m_PairArray[i]->m_bSelected = false;
			}
			if (NewState & LVIS_SELECTED)
			{
				m_PairArray[i]->m_bSelected = true;
			}
			if (OldState & LVIS_FOCUSED)
			{
				m_PairArray[i]->m_bFocused = false;
			}
			if (NewState & LVIS_FOCUSED)
			{
				m_PairArray[i]->m_bFocused = true;
			}
		}
	}
}

void CAlegrDiffView::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (0) TRACE("CAlegrDiffView::OnLvnItemchanged: Item %d, uChanged=%#X, old=%s, new=%s\n",
				pNMLV->iItem, pNMLV->uChanged, LvStateString(pNMLV->uOldState),
				LvStateString(pNMLV->uNewState));

	if (pNMLV->uChanged & LVIF_STATE)
	{
		size_t from = size_t(pNMLV->iItem);
		size_t to = from;
		if (pNMLV->iItem == -1)
		{
			// all items
			from = 0;
			to = m_PairArray.size() - 1;
		}
		SetItemsState(from, to, pNMLV->uOldState, pNMLV->uNewState);
	}

	*pResult = 0;
}

void CAlegrDiffView::OnLvnOdItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVODSTATECHANGE *pNMLV = reinterpret_cast<NMLVODSTATECHANGE*>(pNMHDR);
	if (0) TRACE("CAlegrDiffView::OnLvnOdItemchanged: From=%d, to=%d, old=%s, new=%s\n",
				pNMLV->iFrom, pNMLV->iTo, LvStateString(pNMLV->uOldState),
				LvStateString(pNMLV->uNewState));

	SetItemsState(size_t(pNMLV->iFrom), size_t(pNMLV->iTo), pNMLV->uOldState, pNMLV->uNewState);

	*pResult = 0;
}

void CAlegrDiffView::OnLvnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO * plvdi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	FilePair* pPair = GetFilePair(plvdi->item.iItem);
	if (pPair == nullptr)
	{
		*pResult = 1;
		return;
	}
	FileItem* pFileItem = pPair->pFirstFile;
	if (nullptr == pFileItem)
	{
		pFileItem = pPair->pSecondFile;
	}

	*pResult = 0;
	if (plvdi->item.mask & LVIF_STATE)
	{
		plvdi->item.state = pPair->m_bDeleted ? LVIS_CUT : 0;
	}
	if (plvdi->item.mask & LVIF_TEXT)
	{
		unsigned nColumn = plvdi->item.iSubItem;
		if (nColumn >= countof(m_ViewItemToColumnType))
		{
			return;
		}

		eColumns ColumnType = m_ViewItemToColumnType[nColumn];
		static TCHAR TextBuffer[1024];
		plvdi->item.pszText = TextBuffer;
		TextBuffer[0] = 0;

		switch (ColumnType)
		{
		case ColumnName:
			plvdi->item.pszText = (LPTSTR)(LPCTSTR)pFileItem->GetName();
			break;
		case ColumnSubdir:
			plvdi->item.pszText = (LPTSTR)(LPCTSTR)pFileItem->GetSubdir();
			break;
		case ColumnDate1:
			if (NULL != pPair->pFirstFile && !pPair->pFirstFile->IsFolder())
			{
				FileTimeToStr(pPair->pFirstFile->GetLastWriteTime(), TextBuffer);
			}
			break;
		case ColumnDate2:
			if (NULL != pPair->pSecondFile && !pPair->pSecondFile->IsFolder())
			{
				FileTimeToStr(pPair->pSecondFile->GetLastWriteTime(), TextBuffer);
			}
			break;
		case ColumnLength1:
			if (NULL != pPair->pFirstFile && !pPair->pFirstFile->IsFolder())
			{
				FileLengthToStrKb(pPair->pFirstFile->GetFileLength(), TextBuffer);
			}
			break;
		case ColumnLength2:
			if (NULL != pPair->pSecondFile && !pPair->pSecondFile->IsFolder())
			{
				FileLengthToStrKb(pPair->pSecondFile->GetFileLength(), TextBuffer);
			}
			break;
		case ColumnComparisionResult:
			plvdi->item.pszText = (LPTSTR)pPair->GetComparisonResultStr(TextBuffer, sizeof TextBuffer / sizeof TextBuffer[0]);
			break;
		default:
			break;
		}
	}
}

void CAlegrDiffView::OnEditSelectAll()
{
	CListCtrl * pList = &GetListCtrl();
	int Count = pList->GetItemCount();

	for (int i = 0; i < Count; i++)
	{
		pList->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CAlegrDiffView::ToggleShowFilesMask(ShowFilesMask mask)
{
	// don't use XOR, because the mask can have two bits.
	// If one of them gets set (registry screwed), we won't be able to reset them.
	if (m_ShowFilesMask & mask)
	{
		m_ShowFilesMask &= ~mask;
	}
	else
	{
		m_ShowFilesMask |= mask;
	}
	OnUpdate(NULL, OnUpdateRebuildListView, NULL);
}

void CAlegrDiffView::UpdateShowFilesMask(CCmdUI* pCmdUI, ShowFilesMask mask)
{
	if (m_PresentFilesMask & mask)
	{
		pCmdUI->SetCheck(0 != (m_ShowFilesMask & mask));
	}
	else
	{
		// remove this menu item
		CMenu* pMenu = pCmdUI->m_pMenu;
		if (NULL != pMenu)
		{
			pMenu->DeleteMenu(pCmdUI->m_nID, MF_BYCOMMAND);
		}
		else
		{
			pCmdUI->Enable(FALSE);
		}
	}
}

void CAlegrDiffView::OnShowDifferentfiles()
{
	ToggleShowFilesMask(ShowDifferentFiles);
}

void CAlegrDiffView::OnUpdateShowDifferentfiles(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowDifferentFiles);
}

void CAlegrDiffView::OnShowDifferentinversioninfo()
{
	ToggleShowFilesMask(ShowVersionInfoDifferentFiles);
}

void CAlegrDiffView::OnUpdateShowDifferentinversioninfo(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowVersionInfoDifferentFiles);
}

void CAlegrDiffView::OnShowIdenticalfiles()
{
	ToggleShowFilesMask(ShowIdenticalFiles);
}

void CAlegrDiffView::OnUpdateShowIdenticalfiles(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowIdenticalFiles);
}

void CAlegrDiffView::OnShowFilesin2nddirectoryonly()
{
	ToggleShowFilesMask(ShowOnlySecondFileFiles);
}

void CAlegrDiffView::OnUpdateShowFilesin2nddirectoryonly(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowOnlySecondFileFiles);
}

void CAlegrDiffView::OnShowFilesin1stdirectoryonly()
{
	ToggleShowFilesMask(ShowOnlyFirstFileFiles);
}

void CAlegrDiffView::OnUpdateShowFilesin1stdirectoryonly(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowOnlyFirstFileFiles);
}

void CAlegrDiffView::OnShowSubdirectoriesin1stdironly()
{
	ToggleShowFilesMask(ShowOnlyFirstDirectory);
}

void CAlegrDiffView::OnUpdateShowSubdirectoriesin1stdironly(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowOnlyFirstDirectory);
}

void CAlegrDiffView::OnShowSubdirectoriesin2nddir()
{
	ToggleShowFilesMask(ShowOnlySecondDirectory);
}

void CAlegrDiffView::OnUpdateShowSubdirectoriesin2nddir(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowOnlySecondDirectory);
}

void CAlegrDiffView::OnShowDifferentinspacesonly()
{
	ToggleShowFilesMask(ShowDifferentInSpacesFiles);
}

void CAlegrDiffView::OnUpdateShowDifferentinspacesonly(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowDifferentInSpacesFiles);
}

void CAlegrDiffView::OnShowLongerfilesin1stdirectory()
{
	ToggleShowFilesMask(ShowFirstFileLongerFiles);
}

void CAlegrDiffView::OnUpdateShowLongerfilesin1stdirectory(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowFirstFileLongerFiles);
}

void CAlegrDiffView::OnShowLongerfilesin2nddirectory()
{
	ToggleShowFilesMask(ShowSecondFileLongerFiles);
}

void CAlegrDiffView::OnUpdateShowLongerfilesin2nddirectory(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowSecondFileLongerFiles);
}

void CAlegrDiffView::OnInFirstDirectoryOnlySubdirectoriesContents()
{
	ToggleShowFilesMask(ShowFileFromSubdirInFirstDirOnly);
}

void CAlegrDiffView::OnUpdateInFirstDirectoryOnlySubdirectoriesContents(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowFileFromSubdirInFirstDirOnly);
}

void CAlegrDiffView::OnInSecondDirectoryOnlySubdirectoriesContents()
{
	ToggleShowFilesMask(ShowFileFromSubdirInSecondDirOnly);
}

void CAlegrDiffView::OnUpdateInSecondDirectoryOnlySubdirectoriesContents(CCmdUI *pCmdUI)
{
	UpdateShowFilesMask(pCmdUI, ShowFileFromSubdirInSecondDirOnly);
}

void CAlegrDiffView::OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame)
{
	CView::OnActivateFrame(nState, pDeactivateFrame);

	UpdateStatusText(nState);
}

CString CAlegrDiffView::GetNumberOfFilesString()
{
	CString s;
	unsigned TotalFiles = 0;
	unsigned DifferentFiles = 0;
	CAlegrDiffDoc *pDoc = GetDocument();
	// FIXME: Dont walk the whole list which can be huge every time
	// count number of files
	for (FilePair * pPair = pDoc->GetFirstFilePair(); pDoc->FilePairNotEnd(pPair); pPair = pDoc->GetNextFilePair(pPair))
	{
		if (pPair->FilesAreDifferent())
		{
			DifferentFiles++;
		}

		TotalFiles++;
	}

	s.Format(IDS_NUMBER_OF_FILES_FORMAT, DifferentFiles, TotalFiles);

	return s;
}

void CAlegrDiffView::UpdateStatusText(UINT nState)
{
	CFrameWnd * pMainFrm = dynamic_cast<CFrameWnd *>(AfxGetMainWnd());
	if (NULL != pMainFrm)
	{
		if (WA_INACTIVE == nState)
		{
			pMainFrm->SetMessageText(AFX_IDS_IDLEMESSAGE);
		}
		else if (WA_ACTIVE == nState
				|| WA_CLICKACTIVE == nState)
		{
			pMainFrm->SetMessageText(GetNumberOfFilesString());
		}
	}
}
