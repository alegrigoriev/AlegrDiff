// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#pragma once


// CUiUpdatedDlg dialog

class CUiUpdatedDlg : public CDialog
{
	DECLARE_DYNAMIC(CUiUpdatedDlg)

public:
	CUiUpdatedDlg(UINT Id, CWnd* pParent = NULL);   // standard constructor
	virtual ~CUiUpdatedDlg();
	INT_PTR DoModal();

// Dialog Data

protected:
	void NeedUpdateControls()
	{
		m_bNeedUpdateControls = TRUE;
	}

	afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bNeedUpdateControls;
};
