// AlegrDiff.h : main header file for the ALEGRDIFF application
//

#if !defined(AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_)
#define AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <afxtempl.h>

struct TextToken
{
	int m_Offset;
	int m_Len;
	DWORD m_Hash;
	class FileLine * m_pLine;
};

class FileLine
{
public:

	FileLine(const char * src, size_t length, int OrdNum);

	void * operator new (size_t structLen, size_t addLen)
	{
		return new char[structLen + addLen];
	}
	void operator delete(void * ptr, size_t)
	{
		::delete ptr;
	}
public:
	DWORD GetHash() const { return m_HashCode; }
	bool IsEqual(const FileLine & OtherLine) const;
	bool GetNextToken(TextToken & token);
private:
	mutable DWORD m_HashCode;
	mutable DWORD m_Flags;
	enum { HashValid = 1,
	};
	int m_Number; // ordinal number
	int m_Length;
	int m_FirstTokenIndex;

	// allocated with the buffer, must be the last member
	char m_Data[1];
};

class FileItem
{
public:
	FileItem(const CString & Name, const CString & Dir);
	~FileItem();
	bool Load();
	void Unload();
	const char * GetLine(int LineNum) const;
	LPCTSTR GetName() const { return m_Name; }
private:
	CString m_Name;
	CString m_Subdir;
	CArray<FileLine *, FileLine *> m_Lines;
	CArray<TextToken, TextToken> m_Tokens;
};
struct FilePair
{
};
/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffApp:
// See AlegrDiff.cpp for the implementation of this class
//

class CAlegrDiffApp : public CWinApp
{
public:
	CAlegrDiffApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlegrDiffApp)
public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
// Implementation
	//{{AFX_MSG(CAlegrDiffApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_)
