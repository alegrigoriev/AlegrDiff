// AlegrDiffDoc.h : interface of the CAlegrDiffDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALEGRDIFFDOC_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_)
#define AFX_ALEGRDIFFDOC_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "FileListSupport.h"

class CAlegrDiffDoc : public CDocument
{
protected: // create from serialization only
	CAlegrDiffDoc();
	DECLARE_DYNCREATE(CAlegrDiffDoc)

// Attributes
public:
	CString m_sInclusionPattern;
	CString m_sExclusionPattern;
	CString m_sCFilesPattern;
	CString m_sBinaryFilesPattern;
	CString	m_sFirstDir;
	CString	m_sSecondDir;
	FilePair * m_pPairList;
	int m_nFilePairs;
	bool m_bRecurseSubdirs;
// Operations
public:
	bool BuildFilePairList(LPCTSTR dir1, LPCTSTR dir2, bool bRecurseSubdirs);
	void FreeFilePairList();
	//bool BuildFileList(LPCTSTR dir);


// Overrides
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
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CFilePairDoc document

enum {
	SetPositionMakeVisible = 1,
	SetPositionMakeCentered = 2,
	SetPositionCancelSelection = 4,
};

class InvalidatedRange : public CObject
{
public:
	TextPos begin;
	TextPos end;
};
class CFilePairDoc : public CDocument
{
protected:
	CFilePairDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFilePairDoc)

// Attributes
public:
	TextPos m_CaretPos;
	TextPos m_SelectionAnchor;
	bool m_bIgnoreWhitespaces;

// Operations
public:
	void SetFilePair(FilePair * pPair);
	FilePair * GetFilePair() const { return m_pFilePair; }
	LinePair * GetLinePair(int line) const;
	int GetTotalLines() const { return m_TotalLines; }
	void SetCaretPosition(int pos, int line, int flags);

	enum {
		CaretPositionChanged = 1,
		FileLoaded,
		InvalidateRange,
		FontChanged,
	};

	void SetSelection(TextPos CaretPos, TextPos AnchorPos, int flags = SetPositionMakeCentered);
	void OnEditGotonextdiff();
	void OnEditGotoprevdiff();
	void CaretToHome(int flags);
	void CaretToEnd(int flags);
	ULONG CopyTextToMemory(PUCHAR pBuf, ULONG BufLen, TextPos pFrom, TextPos pTo);
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilePairDoc)
public:
protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// Implementation
	int m_TotalLines;
	FilePair * m_pFilePair;
public:
	virtual ~CFilePairDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// Generated message map functions
public:
	int GetAcceptDeclineFlags(TextPos begin, TextPos end);
	BOOL DoSaveMerged(BOOL bOpenResultFile);
	BOOL SaveMergedFile(LPCTSTR Name, int DefaultFlags);

	LPCTSTR GetLineText(int nLineNum, LPTSTR buf, size_t BufChars, int * pStrLen);
	// recalculates offset in the raw line to offset in the line with or without whitespaces shown
	TextPos LinePosToDisplayPos(TextPos position);
	// recalculates offset in the line with or without whitespaces shown to offset in the raw line
	TextPos DisplayPosToLinePos(TextPos position);

	bool GetWordUnderCaret(TextPos & Start, TextPos & End);
	bool FindWordOrSelection(bool bBackwards);
	void CaretLeftToWord(int SelectionFlags);
	void CaretRightToWord(int SelectionFlags);

	bool FindTextString(LPCTSTR pStrToFind, bool bBackward, bool bCaseSensitive);
	bool OnEditFind();
	bool OnEditFindNext();
	bool OnEditFindPrev();
	bool OnEditFindWordNext();
	bool OnEditFindWordPrev();
	afx_msg void OnUpdateCaretPosIndicator(CCmdUI* pCmdUI);
	//{{AFX_MSG(CFilePairDoc)
	afx_msg void OnUpdateEditGotonextdiff(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditGotoprevdiff(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
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
	afx_msg void OnUpdateViewIgnoreWhitespaces(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileCopyFirstDirFile(CCmdUI* pCmdUI);
	afx_msg void OnFileCopyFirstDirFile();
	afx_msg void OnUpdateFileCopySecondDirFile(CCmdUI* pCmdUI);
	afx_msg void OnFileCopySecondDirFile();
	//}}AFX_MSG
protected:
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFFDOC_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_)
