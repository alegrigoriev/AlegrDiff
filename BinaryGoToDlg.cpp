// BinaryGoToDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "BinaryGoToDlg.h"


// CBinaryGoToDlg dialog

CBinaryGoToDlg::CBinaryGoToDlg(ULONGLONG FileOffset, CWnd* pParent /*=NULL*/)
	: CDialog(CBinaryGoToDlg::IDD, pParent)
	, m_OffsetStr(_T(""))
	, m_FileOffset(FileOffset)
{
}

CBinaryGoToDlg::~CBinaryGoToDlg()
{
}

void CBinaryGoToDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	if ( ! pDX->m_bSaveAndValidate)
	{
		m_OffsetStr.Format(_T("%I64X"), m_FileOffset);
	}

	DDX_Text(pDX, IDC_EDIT1, m_OffsetStr);
	DDV_MaxChars(pDX, m_OffsetStr, 16);

	if (pDX->m_bSaveAndValidate)
	{
		TCHAR * endptr = NULL;
		m_FileOffset = _tcstoui64(m_OffsetStr, & endptr, 16);
	}
}

BOOL CBinaryGoToDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CBinaryGoToDlg, CDialog)
END_MESSAGE_MAP()

// CBinaryGoToDlg message handlers

