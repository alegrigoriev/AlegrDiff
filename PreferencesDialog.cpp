// PreferencesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "PreferencesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog dialog


CPreferencesDialog::CPreferencesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPreferencesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPreferencesDialog)
	m_bUseBinaryFilesFilter = FALSE;
	m_bUseCppFilter = FALSE;
	m_bUseIgnoreFilter = FALSE;
	m_sBinaryFilesFilter = _T("");
	m_sCppFilesFilter = _T("");
	m_sIgnoreFilesFilter = _T("");
	m_nTabIndent = 0;
	m_AutoReloadChangedFiles = FALSE;
	//}}AFX_DATA_INIT
	m_FontPointSize = 100;
	m_bFontChanged = false;
}


void CPreferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPreferencesDialog)
	DDX_Control(pDX, IDC_SPIN1, m_Spin);
	DDX_Check(pDX, IDC_CHECK_BINARY_FILES, m_bUseBinaryFilesFilter);
	DDX_Check(pDX, IDC_CHECK_C_CPP, m_bUseCppFilter);
	DDX_Check(pDX, IDC_CHECK_IGNORE, m_bUseIgnoreFilter);
	DDX_Text(pDX, IDC_EDIT_BINARY_FILES, m_sBinaryFilesFilter);
	DDX_Text(pDX, IDC_EDIT_C_CPP, m_sCppFilesFilter);
	DDX_Text(pDX, IDC_EDIT_IGNORE, m_sIgnoreFilesFilter);
	DDX_Text(pDX, IDC_EDIT_TAB_INDENT, m_nTabIndent);
	DDV_MinMaxUInt(pDX, m_nTabIndent, 0, 32);
	DDX_Check(pDX, IDC_CHECK_AUTO_RELOAD, m_AutoReloadChangedFiles);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPreferencesDialog, CDialog)
	//{{AFX_MSG_MAP(CPreferencesDialog)
	ON_BN_CLICKED(IDC_CHECK_BINARY_FILES, OnCheckBinaryFiles)
	ON_BN_CLICKED(IDC_CHECK_C_CPP, OnCheckCCpp)
	ON_BN_CLICKED(IDC_CHECK_IGNORE, OnCheckIgnore)
	ON_BN_CLICKED(IDC_BUTTON_NORMAL_FONT, OnButtonNormalFont)
	ON_BN_CLICKED(IDC_BUTTON_INSERTED_FONT, OnButtonInsertedFont)
	ON_BN_CLICKED(IDC_BUTTON_ERASED_FONT, OnButtonErasedFont)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog message handlers

void CPreferencesDialog::OnCheckBinaryFiles()
{
	CWnd * pEdit = GetDlgItem(IDC_EDIT_BINARY_FILES);
	if (pEdit)
	{
		pEdit->EnableWindow(IsDlgButtonChecked(IDC_CHECK_BINARY_FILES));
	}
}

void CPreferencesDialog::OnCheckCCpp()
{
	CWnd * pEdit = GetDlgItem(IDC_EDIT_C_CPP);
	if (pEdit)
	{
		pEdit->EnableWindow(IsDlgButtonChecked(IDC_CHECK_C_CPP));
	}
}

void CPreferencesDialog::OnCheckIgnore()
{
	CWnd * pEdit = GetDlgItem(IDC_EDIT_IGNORE);
	if (pEdit)
	{
		pEdit->EnableWindow(IsDlgButtonChecked(IDC_CHECK_IGNORE));
	}
}

BOOL CPreferencesDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Spin.SetRange(1, 32);

	CWnd * pWnd = GetDlgItem(IDC_EDIT_C_CPP);
	if (pWnd)
	{
		pWnd->EnableWindow(m_bUseCppFilter);
	}

	pWnd = GetDlgItem(IDC_EDIT_BINARY_FILES);
	if (pWnd)
	{
		pWnd->EnableWindow(m_bUseBinaryFilesFilter);
	}

	pWnd = GetDlgItem(IDC_EDIT_IGNORE);
	if (pWnd)
	{
		pWnd->EnableWindow(m_bUseIgnoreFilter);
	}

	FontChanged();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CPreferencesDialog::FontChanged()
{
	m_NormalFont.DeleteObject();
	m_AddedFont.DeleteObject();
	m_ErasedFont.DeleteObject();

	// recalculate font size
	CDC * pDC = CWnd::GetDesktopWindow()->GetWindowDC();
	m_NormalLogFont.lfHeight = -MulDiv(m_FontPointSize, pDC->GetDeviceCaps(LOGPIXELSY), 720);
	CWnd::GetDesktopWindow()->ReleaseDC(pDC);

	m_AddedLogFont.lfHeight = m_NormalLogFont.lfHeight;
	m_ErasedLogFont.lfHeight = m_NormalLogFont.lfHeight;

	m_NormalFont.CreateFontIndirect( & m_NormalLogFont);
	m_AddedFont.CreateFontIndirect( & m_AddedLogFont);
	m_ErasedFont.CreateFontIndirect( & m_ErasedLogFont);

	CWnd * pWnd = GetDlgItem(IDC_STATIC_NORMAL_TEXT);
	if (pWnd)
	{
		pWnd->SetFont( & m_NormalFont, TRUE);
	}

	pWnd = GetDlgItem(IDC_STATIC_ADDED_TEXT);
	if (pWnd)
	{
		pWnd->SetFont( & m_AddedFont, TRUE);
	}

	pWnd = GetDlgItem(IDC_STATIC_DELETED_TEXT);
	if (pWnd)
	{
		pWnd->SetFont( & m_ErasedFont, TRUE);
	}
}

class CFontDialogTitle : public CFontDialog
{
public:
	CFontDialogTitle(LOGFONT * pLogFont, DWORD flags, LPCTSTR Title)
		: CFontDialog(pLogFont, flags),
		m_Title(Title)
	{
	}
	virtual BOOL OnInitDialog( );
	CString m_Title;
};

BOOL CFontDialogTitle::OnInitDialog( )
{
	if (CFontDialog::OnInitDialog())
	{
		SetWindowText(m_Title);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CPreferencesDialog::OnButtonNormalFont()
{
	CFontDialogTitle dlg( & m_NormalLogFont,
						CF_FIXEDPITCHONLY
						| CF_INITTOLOGFONTSTRUCT
						| CF_NOVECTORFONTS
						| CF_SCREENFONTS
						| CF_EFFECTS, "Normal Text Font");
	dlg.m_cf.rgbColors = m_NormalTextColor;
	if (IDOK == dlg.DoModal())
	{
		m_FontPointSize = dlg.GetSize();
		m_NormalTextColor = dlg.GetColor();

		// copy everyting to m_ErasedLogFont, m_AddedLogFont
		m_ErasedLogFont.lfHeight = m_NormalLogFont.lfHeight;
		m_ErasedLogFont.lfCharSet = m_NormalLogFont.lfCharSet;
		memcpy(m_ErasedLogFont.lfFaceName, m_NormalLogFont.lfFaceName,
				sizeof m_ErasedLogFont.lfFaceName);
		m_AddedLogFont.lfHeight = m_NormalLogFont.lfHeight;
		m_AddedLogFont.lfCharSet = m_NormalLogFont.lfCharSet;
		memcpy(m_AddedLogFont.lfFaceName, m_NormalLogFont.lfFaceName,
				sizeof m_AddedLogFont.lfFaceName);
		m_bFontChanged = true;
		FontChanged();
	}
}

void CPreferencesDialog::OnButtonInsertedFont()
{
	CFontDialogTitle dlg( & m_AddedLogFont,
						CF_FIXEDPITCHONLY
						| CF_INITTOLOGFONTSTRUCT
						| CF_NOVECTORFONTS
						| CF_SCREENFONTS
						| CF_EFFECTS, "Added Text Font");

	dlg.m_cf.rgbColors = m_AddedTextColor;
	if (IDOK == dlg.DoModal())
	{
		m_FontPointSize = dlg.GetSize();
		m_AddedTextColor = dlg.GetColor();

		// copy everyting to m_ErasedLogFont, m_NormalLogFont
		m_ErasedLogFont.lfHeight = m_AddedLogFont.lfHeight;
		m_ErasedLogFont.lfCharSet = m_AddedLogFont.lfCharSet;
		memcpy(m_ErasedLogFont.lfFaceName, m_AddedLogFont.lfFaceName,
				sizeof m_ErasedLogFont.lfFaceName);
		m_NormalLogFont.lfHeight = m_AddedLogFont.lfHeight;
		m_NormalLogFont.lfCharSet = m_AddedLogFont.lfCharSet;
		memcpy(m_NormalLogFont.lfFaceName, m_AddedLogFont.lfFaceName,
				sizeof m_AddedLogFont.lfFaceName);
		m_bFontChanged = true;
		FontChanged();
	}
}

void CPreferencesDialog::OnButtonErasedFont()
{
	CFontDialogTitle dlg( & m_ErasedLogFont,
						CF_FIXEDPITCHONLY
						| CF_INITTOLOGFONTSTRUCT
						| CF_NOVECTORFONTS
						| CF_SCREENFONTS
						| CF_EFFECTS, "Deleted Text Font");

	dlg.m_cf.rgbColors = m_ErasedTextColor;
	if (IDOK == dlg.DoModal())
	{
		m_FontPointSize = dlg.GetSize();
		m_ErasedTextColor = dlg.GetColor();

		// copy everyting to m_AddedLogFont, m_NormalLogFont
		m_AddedLogFont.lfHeight = m_ErasedLogFont.lfHeight;
		m_AddedLogFont.lfCharSet = m_ErasedLogFont.lfCharSet;
		memcpy(m_AddedLogFont.lfFaceName, m_ErasedLogFont.lfFaceName,
				sizeof m_ErasedLogFont.lfFaceName);

		m_NormalLogFont.lfHeight = m_ErasedLogFont.lfHeight;
		m_NormalLogFont.lfCharSet = m_ErasedLogFont.lfCharSet;
		memcpy(m_NormalLogFont.lfFaceName, m_ErasedLogFont.lfFaceName,
				sizeof m_ErasedLogFont.lfFaceName);
		m_bFontChanged = true;
		FontChanged();
	}
}

HBRUSH CPreferencesDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT id = pWnd->GetDlgCtrlID();
	if (pDC->m_hDC != NULL)
	{
		switch (id)
		{
		case IDC_STATIC_NORMAL_TEXT:
			TRACE("OnCtlColor IDC_BUTTON_NORMAL_FONT\n");
			pDC->SetTextColor(m_NormalTextColor);
			hbr = GetSysColorBrush(COLOR_WINDOW);
			pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
			pDC->SetBkMode(OPAQUE);
			break;
		case IDC_STATIC_ADDED_TEXT:
			pDC->SetTextColor(m_AddedTextColor);
			hbr = GetSysColorBrush(COLOR_WINDOW);
			pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
			pDC->SetBkMode(OPAQUE);
			break;
		case IDC_STATIC_DELETED_TEXT:
			pDC->SetTextColor(m_ErasedTextColor);
			hbr = GetSysColorBrush(COLOR_WINDOW);
			pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
			pDC->SetBkMode(OPAQUE);
			break;
		}
	}
	// TODO: Return a different brush if the default is not desired
	return hbr;
}
