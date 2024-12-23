// AlegrDiffDoc.h : interface of the CAlegrDiffDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALEGRDIFFDOC_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_)
#define AFX_ALEGRDIFFDOC_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "FileListSupport.h"

enum {
	OnUpdateRebuildListView = 1,
};

class CAlegrDiffBaseDoc : public CDocument
{
protected:
	DECLARE_DYNCREATE(CAlegrDiffBaseDoc)
public:
	CAlegrDiffBaseDoc() noexcept {}
	virtual void OnUpdateAllViews(CView* pSender,
								LPARAM lHint = 0L, CObject* pHint = NULL);
};

class CAlegrDiffDoc : public CAlegrDiffBaseDoc
{
protected: // create from serialization only
	CAlegrDiffDoc();
	DECLARE_DYNCREATE(CAlegrDiffDoc)

	// Attributes

	unsigned CompareDirectoriesFunction(class CComparisonProgressDlg * pDlg);

public:
	CString m_sInclusionPattern;
	CString m_sExclusionPattern;
	CString m_sIgnoreDirsPattern;
	CString m_sCFilesPattern;
	CString m_sBinaryFilesPattern;

	CString	m_sFirstDir;
	CString	m_sSecondDir;

	FilePairList m_PairList;

	bool m_bRecurseSubdirs;
	bool m_bCheckingFingerprint;
	bool m_bDoNotCompareFileContents;

	FilePair* GetFirstFilePair() noexcept
	{
		return m_PairList.First();
	}
	FilePair* GetNextFilePair(FilePair const *Pair) noexcept
	{
		return m_PairList.Next(Pair);
	}
	bool FilePairNotEnd(FilePair const *Pair) noexcept
	{
		return m_PairList.NotEnd(Pair);
	}
// Operations
public:
	bool RunDirectoriesComparison(LPCTSTR dir1, LPCTSTR dir2,
								LPCTSTR FilenameFilter, LPCTSTR IgnoreFolders,
								bool bRecurseSubdirs, bool BinaryComparison,
								bool DoNotCompareFileContents);

	bool CanCancelComparison(CProgressDialog * pDlg);
	// if returns true, call UpdateAllViews
	bool BuildFilePairList(OPTIONAL LPCTSTR FirstDirOrFingerprint, LPCTSTR SecondDir);
	void FreeFilePairList() noexcept;
	void SetFingerprintCheckingMode(LPCTSTR DirectoryToCheck,
									LPCTSTR FingerprintFilename);

// Overrides
	virtual void OnUpdateAllViews(CView* pSender,
								LPARAM lHint = 0L, CObject* pHint = NULL);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlegrDiffDoc)
public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAlegrDiffDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAlegrDiffDoc)
	afx_msg void OnFileSave();
	afx_msg void OnViewRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateViewRefresh(CCmdUI *pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CTextFilePairDoc document

class CFilePairDoc : public CAlegrDiffBaseDoc
{
protected:
	DECLARE_DYNAMIC(CFilePairDoc)
public:
	CFilePairDoc() noexcept
	{}

	virtual void SetFilePair(FilePair* pPair) = 0;
	virtual FilePair* GetFilePair() const = 0;
protected:
};

class CTextFilePairDoc : public CFilePairDoc
{
	typedef CFilePairDoc BaseDoc;
protected:
	CTextFilePairDoc() noexcept;           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTextFilePairDoc)

// Attributes
public:
	virtual TextFilePair* GetFilePair() const noexcept override
	{
		return m_pFilePair;
	}
	TextPosDisplay m_CaretPos;
	TextPosDisplay m_SelectionAnchor;
	TextPosDisplay m_OriginalSelectionAnchor;
	bool m_WordSelectionMode;
	bool m_bIgnoreWhitespaces;
	TCHAR m_ComparisonResult[MAX_PATH];

// Operations
public:
	virtual void SetFilePair(FilePair * pPair) override;
	LinePair * GetLinePair(int line) const noexcept;
	int GetTotalLines() const noexcept { return m_TotalLines; }
	void SetCaretPosition(int pos, int line, int flags);
	void SetCaretPosition(TextPosDisplay pos, int flags);
	void SetCaretPosition(TextPosLine pos, eFileScope FileScope, int flags);

	enum {
		CaretPositionChanged = 1,
		InvalidateRange,
	};

	void SetSelection(TextPosDisplay CaretPos, TextPosDisplay AnchorPos, int flags = SetPositionMakeCentered);
	void CaretToHome(int flags);
	void CaretToEnd(int flags);
	ULONG CopyTextToMemory(LPTSTR pBuf, ULONG BufLen,
							TextPosDisplay pFrom, TextPosDisplay pTo, int FileSelect);
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextFilePairDoc)
public:
protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// Implementation
	int m_TotalLines;
	TextFilePair* m_pFilePair;

public:
	virtual ~CTextFilePairDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
public:
	virtual void OnUpdateAllViews(CView* pSender,
								LPARAM lHint = 0L, CObject* pHint = NULL);

	int GetAcceptDeclineFlags(TextPosLine begin, TextPosLine end);
	int GetAcceptDeclineFlags(TextPosDisplay begin, TextPosDisplay end);
	BOOL DoSaveMerged(BOOL bOpenResultFile);
	BOOL SaveMergedFile(LPCTSTR Name, int DefaultFlags, BOOL bUnicode);

	// FileScope: 0 - combined file shown, 1 - first file shown, 2 - second file shown
	LPCTSTR GetLineText(int nLineNum, LPTSTR buf, size_t BufChars, int * pStrLen, eFileScope FileScope);

	// recalculates offset in the raw line to offset in the line with or without whitespaces shown
	TextPosDisplay LinePosToDisplayPos(TextPosLine position, eFileScope FileScope);
	// recalculates offset in the line with or without whitespaces shown to offset in the raw line
	TextPosLine DisplayPosToLinePos(TextPosDisplay position);

	bool GetWordOnPos(TextPosDisplay OnPos, TextPosDisplay & Start, TextPosDisplay & End);
	void GetWordUnderCursor(CString & Str);
	void CaretLeftToWord(int SelectionFlags);
	void CaretRightToWord(int SelectionFlags);

	bool FindTextString(LPCTSTR pStrToFind, bool bBackward, bool bCaseSensitive, bool WholeWord, eFileScope SearchScope);

	void OnEditCopy(int FileSelect);
	afx_msg void OnUpdateCaretPosIndicator(CCmdUI* pCmdUI);
	// Generated message map functions
	//{{AFX_MSG(CTextFilePairDoc)
	afx_msg void OnUpdateEditGotonextdiff(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditGotoprevdiff(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnViewRefresh();
	afx_msg void OnUpdateFileEditFirst(CCmdUI* pCmdUI);
	afx_msg void OnFileEditFirst();
	afx_msg void OnUpdateFileEditSecond(CCmdUI* pCmdUI);
	afx_msg void OnFileEditSecond();
	afx_msg void OnEditAccept();
	afx_msg void OnUpdateEditAccept(CCmdUI* pCmdUI);
	afx_msg void OnEditDecline();
	afx_msg void OnUpdateEditDecline(CCmdUI* pCmdUI);
	afx_msg void OnFileMergeSave();
	afx_msg void OnViewIgnoreWhitespaces();
	afx_msg void OnUpdateFileCopyFirstDirFile(CCmdUI* pCmdUI);
	afx_msg void OnFileCopyFirstDirFile();
	afx_msg void OnUpdateFileCopySecondDirFile(CCmdUI* pCmdUI);
	afx_msg void OnFileCopySecondDirFile();
	afx_msg void OnFileProperties();
	afx_msg void OnUpdateFileMergeSave(CCmdUI* pCmdUI);
	afx_msg void OnMergeInclude();
	afx_msg void OnUpdateMergeInclude(CCmdUI* pCmdUI);
	afx_msg void OnMergeExclude();
	afx_msg void OnUpdateMergeExclude(CCmdUI* pCmdUI);
	afx_msg void OnViewAsBinary();
	//}}AFX_MSG
protected:
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFFDOC_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_)
