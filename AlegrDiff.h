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
#include "ApplicationProfile.h"
/////////////////////////////////////////////////////////////////////////////
// CAlegrDiffApp:
// See AlegrDiff.cpp for the implementation of this class
//

class CAlegrDiffApp : public CWinApp
{
public:
	CAlegrDiffApp();

	CApplicationProfile Profile;
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

typedef CAlegrDiffApp CThisApp;
inline CThisApp * GetApp()
{
	return (CThisApp *) AfxGetApp();
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALEGRDIFF_H__48D02BA0_0458_4A17_937D_9DD31867151E__INCLUDED_)
