// GotoLineDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "GotoLineDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGotoLineDialog dialog


CGotoLineDialog::CGotoLineDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CGotoLineDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGotoLineDialog)
	m_LineNumber = 0;
	//}}AFX_DATA_INIT
}


void CGotoLineDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGotoLineDialog)
	DDX_Text(pDX, IDC_EDIT_LINE_NUMBER, m_LineNumber);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGotoLineDialog, CDialog)
	//{{AFX_MSG_MAP(CGotoLineDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGotoLineDialog message handlers
