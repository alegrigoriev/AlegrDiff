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
	m_GoToLineFileSelection = -1;
	m_LineNumber = 0;
	//}}AFX_DATA_INIT
	m_FirstFileNumLines = 0;
	m_SecondFileNumLines = 0;
	m_CombinedFileNumLines = 0;
	m_FirstFileLine = 0;
	m_SecondFileLine = 0;
	m_CombinedFileLine = 0;
	m_bSingleFile = TRUE;
}


void CGotoLineDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	if ( ! pDX->m_bSaveAndValidate)
	{
		switch (m_GoToLineFileSelection)
		{
		case 0:
			m_LineNumber = m_FirstFileLine;
			break;
		case 1:
			m_LineNumber = m_SecondFileLine;
			break;
		case 2:
			m_LineNumber = m_CombinedFileLine;
			break;
		}
	}
	//{{AFX_DATA_MAP(CGotoLineDialog)
	DDX_Radio(pDX, IDC_RADIO_FIRST_FILE, m_GoToLineFileSelection);
	DDX_Text(pDX, IDC_EDIT_LINE_NUMBER, m_LineNumber);
	//}}AFX_DATA_MAP
	if (pDX->m_bSaveAndValidate)
	{
		switch (m_GoToLineFileSelection)
		{
		case 0:
			DDV_MinMaxUInt(pDX, m_LineNumber, 1, m_FirstFileNumLines + 1);
			m_FirstFileLine = m_LineNumber;
			break;
		case 1:
			DDV_MinMaxUInt(pDX, m_LineNumber, 1, m_SecondFileNumLines + 1);
			m_SecondFileLine = m_LineNumber;
			break;
		case 2:
			DDV_MinMaxUInt(pDX, m_LineNumber, 1, m_CombinedFileNumLines + 1);
			m_CombinedFileLine = m_LineNumber;
			break;
		}
	}
}


BEGIN_MESSAGE_MAP(CGotoLineDialog, CDialog)
	//{{AFX_MSG_MAP(CGotoLineDialog)
	ON_BN_CLICKED(IDC_RADIO_FIRST_FILE, OnRadioFirstFile)
	ON_BN_CLICKED(IDC_RADIO2, OnRadioSecondFile)
	ON_BN_CLICKED(IDC_RADIO3, OnRadioCombinedFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGotoLineDialog message handlers

void CGotoLineDialog::OnRadioFirstFile()
{
	switch (m_GoToLineFileSelection)
	{
	case 0:
		return;
		break;
	case 1:
		m_SecondFileLine = GetDlgItemInt(IDC_EDIT_LINE_NUMBER);
		break;
	case 2:
		m_CombinedFileLine = GetDlgItemInt(IDC_EDIT_LINE_NUMBER);
		break;
	}
	m_GoToLineFileSelection	= 0;
	SetDlgItemInt(IDC_EDIT_LINE_NUMBER, m_FirstFileLine);
}

void CGotoLineDialog::OnRadioSecondFile()
{
	switch (m_GoToLineFileSelection)
	{
	case 0:
		m_FirstFileLine = GetDlgItemInt(IDC_EDIT_LINE_NUMBER);
		break;
	case 1:
		return;
		break;
	case 2:
		m_CombinedFileLine = GetDlgItemInt(IDC_EDIT_LINE_NUMBER);
		break;
	}
	m_GoToLineFileSelection	= 1;
	SetDlgItemInt(IDC_EDIT_LINE_NUMBER, m_SecondFileLine);
}

void CGotoLineDialog::OnRadioCombinedFile()
{
	switch (m_GoToLineFileSelection)
	{
	case 0:
		m_FirstFileLine = GetDlgItemInt(IDC_EDIT_LINE_NUMBER);
		break;
	case 1:
		m_SecondFileLine = GetDlgItemInt(IDC_EDIT_LINE_NUMBER);
		break;
	case 2:
		return;
		break;
	}
	m_GoToLineFileSelection	= 2;
	SetDlgItemInt(IDC_EDIT_LINE_NUMBER, m_CombinedFileLine);
}

BOOL CGotoLineDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_bSingleFile)
	{
		// disable file selection controls
		CWnd * pWnd = GetDlgItem(IDC_RADIO_FIRST_FILE);
		if (pWnd)
		{
			pWnd->EnableWindow(FALSE);
		}
		pWnd = GetDlgItem(IDC_RADIO2);
		if (pWnd)
		{
			pWnd->EnableWindow(FALSE);
		}
		pWnd = GetDlgItem(IDC_RADIO3);
		if (pWnd)
		{
			pWnd->EnableWindow(FALSE);
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
