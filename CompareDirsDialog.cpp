// CompareDirsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "CompareDirsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCompareDirsDialog dialog


CCompareDirsDialog::CCompareDirsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCompareDirsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCompareDirsDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCompareDirsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCompareDirsDialog)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCompareDirsDialog, CDialog)
	//{{AFX_MSG_MAP(CCompareDirsDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompareDirsDialog message handlers
