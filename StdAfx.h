// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__4D517262_B7B6_49AF_B01C_BA14E5F944D2__INCLUDED_)
#define AFX_STDAFX_H__4D517262_B7B6_49AF_B01C_BA14E5F944D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _UNICODE
#error _UNICODE needs to be defined. Not supporting non-UNICODE build
#endif

#define ISOLATION_AWARE_ENABLED 1
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <limits.h>
#include <afxdhtml.h>
#define TCHAR_MASK TCHAR(-1)

template<typename T> inline void memzero(T & obj)
{
	memset( & obj, 0, sizeof obj);
}

#define countof(a) (sizeof (a) / sizeof (a[0]))

#include "MultiString.inl"

#define EnableDlgItem(id, Enable) \
	::EnableWindow(GetDlgItem(id)->GetSafeHwnd(), Enable)

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4D517262_B7B6_49AF_B01C_BA14E5F944D2__INCLUDED_)

