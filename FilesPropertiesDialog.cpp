// FilesPropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "FilesPropertiesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFilesPropertiesDialog dialog


CFilesPropertiesDialog::CFilesPropertiesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFilesPropertiesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFilesPropertiesDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFilesPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilesPropertiesDialog)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFilesPropertiesDialog, CDialog)
	//{{AFX_MSG_MAP(CFilesPropertiesDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilesPropertiesDialog message handlers
