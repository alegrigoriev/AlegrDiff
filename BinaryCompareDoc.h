#pragma once


// CBinaryCompareDoc document

class CBinaryCompareDoc : public CDocument
{
	DECLARE_DYNCREATE(CBinaryCompareDoc)

public:
	TCHAR m_ComparisonResult[MAX_PATH];
	int m_TotalLines;

	void SetFilePair(FilePair * pPair);
	FilePair * GetFilePair() const { return m_pFilePair; }
	void SetSelection(TextPos CaretPos, TextPos AnchorPos, int flags = SetPositionMakeCentered);
	void CaretToHome(int flags);
	void CaretToEnd(int flags);

	CBinaryCompareDoc();
	virtual ~CBinaryCompareDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual BOOL OnNewDocument();
	FilePair * m_pFilePair;
	TextPos m_CaretPos;
	TextPos m_SelectionAnchor;
	TextPos m_OriginalSelectionAnchor;

	DECLARE_MESSAGE_MAP()
};
