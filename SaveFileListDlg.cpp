// SaveFileListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "SaveFileListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveFileListDlg dialog


CSaveFileListDlg::CSaveFileListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSaveFileListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaveFileListDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSaveFileListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveFileListDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveFileListDlg, CDialog)
	//{{AFX_MSG_MAP(CSaveFileListDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveFileListDlg message handlers
