// FindDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "FindDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyFindDialog dialog


CMyFindDialog::CMyFindDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMyFindDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMyFindDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMyFindDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMyFindDialog)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMyFindDialog, CDialog)
	//{{AFX_MSG_MAP(CMyFindDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyFindDialog message handlers
