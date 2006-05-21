// BinaryGoToDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "BinaryGoToDlg.h"
#include <limits.h>

// CBinaryGoToDlg dialog

CBinaryGoToDlg::CBinaryGoToDlg(ULONGLONG FileOffset, ULONGLONG FileLength, CWnd* pParent /*=NULL*/)
	: BaseClass(IDD, pParent)
	, m_FileOffset(FileOffset)
	, m_InitialOffset(FileOffset)
	, m_FileLength(FileLength)
{
}

CBinaryGoToDlg::~CBinaryGoToDlg()
{
}

#define INVALID_STRING_RESULT (ULLONG_MAX)
#define OUT_OF_BOUNDS_RESULT (ULLONG_MAX-1)

void CBinaryGoToDlg::DoDataExchange(CDataExchange* pDX)
{
	BaseClass::DoDataExchange(pDX);
	CString OffsetStr;
	if ( ! pDX->m_bSaveAndValidate)
	{
		OffsetStr.Format(_T("%I64X"), m_FileOffset);
	}

	DDX_Text(pDX, IDC_EDIT1, OffsetStr);

	if (pDX->m_bSaveAndValidate)
	{
		m_FileOffset = GetNewOffset(OffsetStr);
		if (INVALID_STRING_RESULT == m_FileOffset)
		{
			CString s;
			s.Format(IDS_WRONG_BINARY_OFFSET_FORMAT, LPCTSTR(OffsetStr));
			AfxMessageBox(s);
			pDX->Fail();
		}
		else if (OUT_OF_BOUNDS_RESULT == m_FileOffset)
		{
			CString s;
			s.Format(IDS_OUT_OF_BOUNDS_OFFSET_FORMAT, LPCTSTR(OffsetStr));
			AfxMessageBox(s);
			pDX->Fail();
		}
	}
}

ULONGLONG CBinaryGoToDlg::GetNewOffset(LPCTSTR str)
{
	TCHAR * endptr = NULL;
	ULONGLONG num;
	if (str[0] == _T('+'))
	{
		num = _tcstoui64(str + 1, & endptr, 16);
		if (NULL == endptr
			|| 0 != *endptr)
		{
			return INVALID_STRING_RESULT;
		}
		if (num > m_FileLength - m_InitialOffset)
		{
			return OUT_OF_BOUNDS_RESULT;
		}
		return m_InitialOffset + num;
	}
	else if (str[0] == _T('-'))
	{
		num = _tcstoui64(str + 1, & endptr, 16);
		if (NULL == endptr
			|| 0 != *endptr)
		{
			return INVALID_STRING_RESULT;
		}
		if (num > m_InitialOffset)
		{
			return OUT_OF_BOUNDS_RESULT;
		}
		return m_InitialOffset - num;
	}
	else
	{
		num = _tcstoui64(str, & endptr, 16);
		if (NULL == endptr
			|| 0 != *endptr)
		{
			return INVALID_STRING_RESULT;
		}
		if (num > m_FileLength)
		{
			return OUT_OF_BOUNDS_RESULT;
		}
		return num;
	}
}

BOOL CBinaryGoToDlg::OnInitDialog()
{
	BaseClass::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CBinaryGoToDlg, BaseClass)
END_MESSAGE_MAP()

// CBinaryGoToDlg message handlers

