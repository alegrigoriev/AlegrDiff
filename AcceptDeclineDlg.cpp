// AcceptDeclineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "AcceptDeclineDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAcceptDeclineDlg dialog


CAcceptDeclineDlg::CAcceptDeclineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAcceptDeclineDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAcceptDeclineDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAcceptDeclineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAcceptDeclineDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAcceptDeclineDlg, CDialog)
	//{{AFX_MSG_MAP(CAcceptDeclineDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAcceptDeclineDlg message handlers
