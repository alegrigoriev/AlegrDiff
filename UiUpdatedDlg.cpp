// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
// UiUpdatedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UiUpdatedDlg.h"
#include <afxpriv.h>

// CUiUpdatedDlg dialog

//IMPLEMENT_DYNAMIC(CUiUpdatedDlg, CDialog)
CUiUpdatedDlg::CUiUpdatedDlg(UINT Id, CWnd* pParent /*=NULL*/)
	: BaseClass(Id, pParent)
	, m_bNeedUpdateControls(TRUE)
{
}

BEGIN_MESSAGE_MAP(CUiUpdatedDlg, BaseClass)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()


// CUiUpdatedDlg message handlers
LRESULT CUiUpdatedDlg::OnKickIdle(WPARAM, LPARAM)
{
	if (m_bNeedUpdateControls)
	{
		UpdateDialogControls(this, FALSE);
	}
	m_bNeedUpdateControls = FALSE;
	return 0;
}

INT_PTR CUiUpdatedDlg::DoModal()
{
	m_bNeedUpdateControls = TRUE;
	return BaseClass::DoModal();
}
