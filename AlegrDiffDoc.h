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
// Operations
public:
	bool BuildFilePairList(LPCTSTR dir1, LPCTSTR dir2, bool bRecurseSubdirs);
	void FreeFilePairList();
	//bool BuildFileList(LPCTSTR dir);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlegrDiffDoc)
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CFilePairDoc document

class CFilePairDoc : public CDocument
{
protected:
	CFilePairDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFilePairDoc)

// Attributes
public:

// Operations
public:
	void SetFilePair(FilePair * pPair);
	FilePair * GetFilePair() const { return m_pFilePair; }
	int GetTotalLines() const { return m_TotalLines; }
	bool UseLinePairArray() const { return m_UseLinePairArray; }
	bool BaseOnFirstFile() const { return m_BaseOnFirstFile; }
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilePairDoc)
public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
	bool m_BaseOnFirstFile;
	int m_TotalLines;
	bool m_UseLinePairArray;
	FilePair * m_pFilePair;
public:
	virtual ~CFilePairDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CFilePairDoc)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFFDOC_H__E1805E7E_66CF_4BE2_A1A1_7C1E818B9987__INCLUDED_)
