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
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_UPDATE_COMMAND_UI(ID_LISTVIEW_OPEN1, OnUpdateListviewOpen)
	ON_COMMAND(ID_LISTVIEW_OPEN1, OnListviewOpen)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffView construction/destruction

CAlegrDiffView::CAlegrDiffView()
{
	m_bSubdirColumnPresent = true;
	CThisApp * pApp = GetApp();
	if (pApp->m_FileListSort >= 0)
	{
		m_SortColumn = eColumns(0xFF & pApp->m_FileListSort);
		m_PrevSortColumn = eColumns((0xFF0000 & pApp->m_FileListSort) >> 16);
		m_bAscendingOrder = ! (0xFF00 & pApp->m_FileListSort);
		m_bPrevAscendingOrder = ! (0xFF000000 & pApp->m_FileListSort);
	}
	else
	{
		// old format, remove support from release!
		m_bAscendingOrder = false;
		m_SortColumn = eColumns(~pApp->m_FileListSort);
		m_PrevSortColumn = ColumnName;
		m_bPrevAscendingOrder = true;
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
//    CAlegrDiffDoc * pDoc = GetDocument();

	pList->InsertColumn(ColumnName, "File Name", LVCFMT_LEFT, 200, ColumnName);
	pList->InsertColumn(ColumnSubdir, "Subdirectory", LVCFMT_LEFT, 200, ColumnSubdir);
	pList->InsertColumn(ColumnDate1, "1st Modified at", LVCFMT_LEFT, 150, ColumnDate1);
	pList->InsertColumn(ColumnDate2, "2nd Modified at", LVCFMT_LEFT, 150, ColumnDate2);
	pList->InsertColumn(ColumnComparisionResult, "Comparison result", LVCFMT_LEFT, 400, ColumnComparisionResult);
	m_bSubdirColumnPresent = true;

	pList->SetExtendedStyle(pList->GetExtendedStyle() | LVS_EX_FULLROWSELECT);
}

void CAlegrDiffView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// change sort order
	int nColumn = pNMListView->iSubItem;
	if ( ! m_bSubdirColumnPresent
		&& nColumn > ColumnName)
	{
		nColumn++;
	}
	if (m_SortColumn == nColumn)
	{
		m_bAscendingOrder = ! m_bAscendingOrder;
	}
	else
	{
		m_bPrevAscendingOrder = m_bAscendingOrder;
		m_PrevSortColumn = m_SortColumn;

		m_SortColumn = eColumns(nColumn);
		m_bAscendingOrder = true;
	}
	CThisApp * pApp = GetApp();

	pApp->m_FileListSort = m_SortColumn | (m_PrevSortColumn << 16);
	if ( ! m_bAscendingOrder)
	{
		pApp->m_FileListSort |= 0x100;
	}
	if ( ! m_bPrevAscendingOrder)
	{
		pApp->m_FileListSort |= 0x1000000;
	}

	OnUpdate(NULL, 0, NULL);
	*pResult = 0;
}

		// TEMPLATE STRUCT binary_function
template<class _A1, class _A2, class _A3, class _R>
struct ternary_function {
	typedef _A1 first_argument_type;
	typedef _A2 second_argument_type;
	typedef _A3 third_argument_type;
	typedef _R result_type;
};

template <class R, class T, class A>
struct TernaryFunc : public ternary_function<T, T, A, R>
{
	TernaryFunc(R (* __f)(T, T, A)): f(__f) {}

	R (* f)(T, T, A);
	bool operator()(first_argument_type A1, second_argument_type A2, third_argument_type A3) const
	{ return f(A1, A2, A3); }
};

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

typedef TernaryFunc<bool, const FilePair *, FilePair::CompareParam > FilePairSortFunc;

void CAlegrDiffView::BuildSortedPairArray(vector<FilePair *> & PairArray, FilePair * pPairs, int nCount)
{
	PairArray.resize(nCount);
	for (int i = 0; i < nCount && pPairs != NULL; i++, pPairs = pPairs->pNext)
	{
		PairArray[i] = pPairs;
	}
	PairArray.resize(i);

	FilePair::CompareParam comp;
	comp.PrimaryBackward = ! m_bAscendingOrder;
	comp.SecondaryBackward = ! m_bPrevAscendingOrder;

	switch (m_SortColumn)
	{
	case ColumnName:
		comp.PrimarySort = FilePair::CompareSubitemName;
		break;
	default:
	case ColumnSubdir:
		comp.PrimarySort = FilePair::CompareSubitemDir;
		break;
	case ColumnDate1:
		comp.PrimarySort = FilePair::CompareSubitemDate1;
		break;
	case ColumnDate2:
		comp.PrimarySort = FilePair::CompareSubitemDate2;
		break;
	case ColumnComparisionResult:
		comp.PrimarySort = FilePair::CompareSubitemResult;
		break;
	}

	switch (m_PrevSortColumn)
	{
	case ColumnName:
		comp.SecondarySort = FilePair::CompareSubitemName;
		break;
	default:
	case ColumnSubdir:
		comp.SecondarySort = FilePair::CompareSubitemDir;
		break;
	case ColumnDate1:
		comp.SecondarySort = FilePair::CompareSubitemDate1;
		break;
	case ColumnDate2:
		comp.SecondarySort = FilePair::CompareSubitemDate2;
		break;
	case ColumnComparisionResult:
		comp.SecondarySort = FilePair::CompareSubitemResult;
		break;
	}

	std::sort(PairArray.begin(), PairArray.end(), bind3rd(FilePairSortFunc(FilePair::Compare), comp));
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
					pListCtrl->SetItemText(i,
											ColumnComparisionResult - ! m_bSubdirColumnPresent,
											ComparisionResult);
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

	if (! pDoc->m_bRecurseSubdirs
		&& m_bSubdirColumnPresent)
	{
		// delete the subdirectory column
		m_bSubdirColumnPresent = false;
		pListCtrl->DeleteColumn(ColumnSubdir);
	}
	for (int item = 0; item < m_PairArray.size(); item++)
	{
		FilePair * pPair = m_PairArray[item];
		AddListViewItem(pPair, item);
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
	if (-1 == nItem)
	{
		nItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED);
	}
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
		pListCtrl->SetItemText(item, ColumnDate1 - ! m_bSubdirColumnPresent, datetime);
	}
	else
	{
		pListCtrl->SetItemText(item, ColumnDate1 - ! m_bSubdirColumnPresent, _T(""));
	}
	if (NULL != pPair->pSecondFile)
	{
		datetime = FileTimeToStr(pPair->pSecondFile->GetLastWriteTime());
		pListCtrl->SetItemText(item, ColumnDate2 - ! m_bSubdirColumnPresent, datetime);
	}
	else
	{
		pListCtrl->SetItemText(item, ColumnDate2 - ! m_bSubdirColumnPresent, _T(""));
	}
	CString ComparisionResult = pPair->GetComparisionResult();
	pListCtrl->SetItemText(item, ColumnComparisionResult - ! m_bSubdirColumnPresent, ComparisionResult);
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
	FILE * file = fopen(dlg.m_sFilename, "wt");
	if (NULL == file)
	{
		CString s;
		s.Format(IDS_UNABLE_TO_CREATE_FILE, LPCTSTR(dlg.m_sFilename));
		AfxMessageBox(s);
		return;
	}
	CString s1;
	s1.LoadString(IDS_DIFF_FILE_BANNER);
	fprintf(file, s1, LPCTSTR(pDoc->m_sFirstDir), LPCTSTR(pDoc->m_sSecondDir));

	int MaxNameLength = 0;
	int MaxDateTimeLength = 0;
	for (int pass = 0; pass < 2; pass++)
	{
		for (int item = 0; item < m_PairArray.size(); item++)
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
				fputs(line, file);
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
