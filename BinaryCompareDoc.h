#pragma once

#include "AlegrDiffDoc.h"
// CBinaryCompareDoc document

class InvalidatedRangeBin : public CObject
{
public:
	LONGLONG begin;
	LONGLONG end;
};

class CBinaryCompareDoc : public CAlegrDiffBaseDoc
{
	DECLARE_DYNCREATE(CBinaryCompareDoc)

public:
	CBinaryCompareDoc();
	virtual ~CBinaryCompareDoc();
	enum {
		CaretPositionChanged = 1,
		InvalidateRange,
		UpdateMakeCaretCentered,
		UpdateMakeCaretVisible,
	};

	TCHAR m_ComparisonResult[MAX_PATH];
	LONGLONG m_CaretPos;
	LONGLONG m_SelectionAnchor;
	LONGLONG m_OriginalSelectionAnchor;

	void SetFilePair(FilePair * pPair);
	FilePair * GetFilePair() const { return m_pFilePair; }
	LONGLONG GetFileSize() const;

	//void SetSelection(TextPos CaretPos, TextPos AnchorPos, int flags = SetPositionMakeCentered);
	void SetCaretPosition(LONGLONG Addr, int flags);

	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	unsigned FindDataProc(class CDifferenceProgressDialog * pDlg);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg void OnUpdateCaretPosIndicator(CCmdUI* pCmdUI);
	virtual BOOL OnNewDocument();
	FilePair * m_pFilePair;

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnUpdateAllViews(CView* pSender,
								LPARAM lHint = 0L, CObject* pHint = NULL);
};
