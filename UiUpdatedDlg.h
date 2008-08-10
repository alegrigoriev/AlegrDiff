// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#pragma once
#include <afxpriv.h>
//#include "MessageMapT.h"

// CUiUpdatedDlg dialog
template<typename B>
class CUiUpdatedDlgT : public B
{
	typedef B BaseClass;

public:
	CUiUpdatedDlgT(UINT Id, CWnd* pParent /*=NULL*/)
		: BaseClass(Id, pParent)
		, m_bNeedUpdateControls(TRUE)
	{
	}

	// to use with property page
	CUiUpdatedDlgT(UINT nIDTemplate, UINT nIDCaption, DWORD dwSize = sizeof(PROPSHEETPAGE))
		: BaseClass(nIDTemplate, nIDCaption, dwSize)
		, m_bNeedUpdateControls(TRUE)
	{
	}

	// Dialog Data

protected:
	void NeedUpdateControls()
	{
		m_bNeedUpdateControls = TRUE;
	}

	virtual void DoDataExchange(CDataExchange* pDX)    // DDX/DDV support
	{
		BaseClass::DoDataExchange(pDX);
		NeedUpdateControls();
	}
	virtual LRESULT OnKickIdle(WPARAM, LPARAM)
	{
		if (m_bNeedUpdateControls)
		{
			UpdateDialogControls(this, FALSE);
		}
		m_bNeedUpdateControls = FALSE;
		return 0;
	}

	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bNeedUpdateControls;
};

BEGIN_TEMPLATE_MESSAGE_MAP(CUiUpdatedDlgT, B, BaseClass)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

typedef CUiUpdatedDlgT<CDialog> CUiUpdatedDlg;
