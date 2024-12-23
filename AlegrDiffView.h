// AlegrDiffView.h : interface of the CAlegrDiffView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_)
#define AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afxcview.h>
#include <vector>
using namespace std;

class CAlegrDiffView : public CListView
{
protected: // create from serialization only
	CAlegrDiffView() noexcept;
	DECLARE_DYNCREATE(CAlegrDiffView)

// Attributes
public:
	CAlegrDiffDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlegrDiffView)
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetItemsState(size_t from, size_t to, UINT OldState, UINT NewState);
	BOOL CopySelectedFiles(int DirIndex);

	// keeps order of columns (even those non-visible)
	//
	eColumns m_SortColumns[MaxColumns];
	bool m_AscendingSortOrder[MaxColumns];
	enum ShowFilesMask
	{
		ShowIdenticalFiles = (1 << FilePair::FilesIdentical) | (1 << FilePair::FilesAttributesIdentical),
		ShowDifferentFiles = 1 << FilePair::FilesDifferent,
		ShowDifferentInSpacesFiles = 1 << FilePair::DifferentInSpaces,
		ShowVersionInfoDifferentFiles = 1 << FilePair::VersionInfoDifferent,
		ShowOnlyFirstFileFiles = (1 << FilePair::OnlyFirstFile) | (1 << FilePair::FileInFingerprintFileOnly),
		ShowOnlySecondFileFiles = 1 << FilePair::OnlySecondFile,
		ShowOnlyFirstDirectory =
			(1 << FilePair::OnlyFirstDirectory) | (1 << FilePair::DirectoryInFingerprintFileOnly),
		ShowOnlySecondDirectory = 1 << FilePair::OnlySecondDirectory,
		ShowFirstFileLongerFiles = 1 << FilePair::FirstFileLonger,
		ShowSecondFileLongerFiles = 1 << FilePair::SecondFileLonger,
		ShowFileFromSubdirInFirstDirOnly =
			(1 << FilePair::FileFromSubdirInFirstDirOnly) | (1 << FilePair::SubdirsParentInFirstDirOnly),
		ShowFileFromSubdirInSecondDirOnly =
			(1 << FilePair::SubdirsParentInSecondDirOnly) | (1 << FilePair::FileFromSubdirInSecondDirOnly),
	};
	ULONG m_ShowFilesMask;
	ULONG m_PresentFilesMask;

	void ToggleShowFilesMask(ShowFilesMask mask);
	void UpdateShowFilesMask(CCmdUI* pCmdUI, ShowFilesMask mask);

	enum eSetSortColumnOrder
	{
		SetSortColumnUnchanged,
		SetSortColumnAscending,
		SetSortColumnDescending,
		SetSortColumnMouseClick,
	};
	void SetSortColumn(eColumns nColumn, eSetSortColumnOrder Order);
	bool ChangeSortItem(eColumns nColumn, eSetSortColumnOrder Order) noexcept;
	// index: column type, result: column position (including hidden columns)
	int m_ColumnArray[MaxColumns];
	int m_ColumnWidthArray[MaxColumns];
	// index - item associated with the column, result: column type
	eColumns m_ViewItemToColumnType[MaxColumns];
	// index: column position as seen, result:item associated with the column
	int m_ColumnPositionToViewItem[MaxColumns];
	int m_ColumnTypeToViewItem[MaxColumns];

	void ResetColumnsArray() noexcept;

	void UpdateAppColumns();
	void BuildListViewHeader();
	void BuildArrowBitmaps();
	void ReBuildListView();

	void PrintColumnOrder();

	virtual ~CAlegrDiffView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	typedef std::vector<FilePair *> FilePairVector;
	typedef FilePairVector::iterator FilePairVectorIterator;
	FilePairVector m_PairArray;

	CBitmap m_ArrowDownBitmap;
	CBitmap m_ArrowUpBitmap;

	void BuildSortedPairArray(vector<FilePair *> & PairArray, FilePairList * pPairList);

	CString GetNumberOfFilesString();
	void UpdateStatusText(UINT nState);

	// Return a FilePair from the array by the index, only if the index is inside range
	FilePair* GetFilePair(size_t index) const noexcept;
	// Return a FilePair from the array by the index, but only if it doesn't have "deleted" flag set
	FilePair* GetValidFilePair(size_t index) const noexcept;
	// Return pPair if its index in the array matches
	FilePair* ValidateFilePairIndex(FilePair const *pPair) const noexcept;
	// Generated message map functions
protected:
	//{{AFX_MSG(CAlegrDiffView)
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileEditFirst();
	afx_msg void OnUpdateFileEditFirst(CCmdUI* pCmdUI);
	afx_msg void OnFileEditSecond();
	afx_msg void OnUpdateFileEditSecond(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnListviewOpen();
	afx_msg void OnUpdateListviewOpen(_In_ CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileCopyFirstDir(_In_ CCmdUI* pCmdUI);
	afx_msg void OnFileCopyFirstDir();
	afx_msg void OnUpdateFileCopySecondDir(CCmdUI* pCmdUI);
	afx_msg void OnFileCopySecondDir();
	afx_msg void OnFileSaveList();
	afx_msg void OnUpdateFileSaveList(CCmdUI* pCmdUI);
	afx_msg void OnViewHideselectedfiles();
	afx_msg void OnUpdateViewHideselectedfiles(CCmdUI* pCmdUI);
	afx_msg void OnViewShowallfiles();
	afx_msg void OnUpdateViewShowallfiles(CCmdUI* pCmdUI);
	afx_msg void OnListviewFilelength();
	afx_msg void OnUpdateListviewFilelength(CCmdUI *pCmdUI);
	afx_msg void OnListviewModificationtime();
	afx_msg void OnUpdateListviewModificationtime(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortby1stlength();
	afx_msg void OnUpdateListviewSortby1stlength(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortby1stmodificationdate();
	afx_msg void OnUpdateListviewSortby1stmodificationdate(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortby2ndlength();
	afx_msg void OnUpdateListviewSortby2ndlength(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortbyComparisonresult();
	afx_msg void OnUpdateListviewSortbyComparisonresult(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortbyFolder();
	afx_msg void OnUpdateListviewSortbyFolder(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortbyName();
	afx_msg void OnUpdateListviewSortbyName(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortbyDescendingorder();
	afx_msg void OnUpdateListviewSortbyDescendingorder(CCmdUI *pCmdUI);
	afx_msg void OnListviewSortby2ndmodificationdate();
	afx_msg void OnUpdateListviewSortby2ndmodificationdate(CCmdUI *pCmdUI);
	afx_msg void OnHdnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnEnddrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnEndTrack(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnTrack(NMHDR * /*pNMHDR*/, LRESULT *pResult);
	afx_msg void OnHdnBeginTrack(NMHDR * /*pNMHDR*/, LRESULT *pResult);
	afx_msg void OnViewResetcolumns();
	afx_msg void OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFileProperties();
	afx_msg void OnUpdateFileProperties(CCmdUI *pCmdUI);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnOdItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEditSelectAll();
	afx_msg void OnShowDifferentfiles();
	afx_msg void OnUpdateShowDifferentfiles(CCmdUI *pCmdUI);
	afx_msg void OnShowDifferentinversioninfo();
	afx_msg void OnUpdateShowDifferentinversioninfo(CCmdUI *pCmdUI);
	afx_msg void OnShowIdenticalfiles();
	afx_msg void OnUpdateShowIdenticalfiles(CCmdUI *pCmdUI);
	afx_msg void OnShowFilesin2nddirectoryonly();
	afx_msg void OnUpdateShowFilesin2nddirectoryonly(CCmdUI *pCmdUI);
	afx_msg void OnShowFilesin1stdirectoryonly();
	afx_msg void OnUpdateShowFilesin1stdirectoryonly(CCmdUI *pCmdUI);
	afx_msg void OnShowSubdirectoriesin1stdironly();
	afx_msg void OnUpdateShowSubdirectoriesin1stdironly(CCmdUI *pCmdUI);
	afx_msg void OnShowSubdirectoriesin2nddir();
	afx_msg void OnUpdateShowSubdirectoriesin2nddir(CCmdUI *pCmdUI);
	afx_msg void OnShowDifferentinspacesonly();
	afx_msg void OnUpdateShowDifferentinspacesonly(CCmdUI *pCmdUI);
	afx_msg void OnShowLongerfilesin1stdirectory();
	afx_msg void OnUpdateShowLongerfilesin1stdirectory(CCmdUI *pCmdUI);
	afx_msg void OnShowLongerfilesin2nddirectory();
	afx_msg void OnUpdateShowLongerfilesin2nddirectory(CCmdUI *pCmdUI);
	afx_msg void OnInFirstDirectoryOnlySubdirectoriesContents();
	afx_msg void OnUpdateInFirstDirectoryOnlySubdirectoriesContents(CCmdUI *pCmdUI);
	afx_msg void OnInSecondDirectoryOnlySubdirectoriesContents();
	afx_msg void OnUpdateInSecondDirectoryOnlySubdirectoriesContents(CCmdUI *pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	virtual void OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame);
};

#ifndef _DEBUG  // debug version in AlegrDiffView.cpp
inline CAlegrDiffDoc* CAlegrDiffView::GetDocument()
{ return (CAlegrDiffDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFFVIEW_H__D826A56C_6EA7_493A_AABF_21AC1A51C0DE__INCLUDED_)
