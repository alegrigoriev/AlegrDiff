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

/////////////////////////////////////////////////////////////////////////////
// CFilesPreferencePage property page

IMPLEMENT_DYNCREATE(CFilesPreferencePage, CPropertyPage)

CFilesPreferencePage::CFilesPreferencePage() : CPropertyPage(CFilesPreferencePage::IDD)
{
	//{{AFX_DATA_INIT(CFilesPreferencePage)
	m_sBinaryFilesFilter = _T("");
	m_sCppFilesFilter = _T("");
	m_sIgnoreFilesFilter = _T("");
	m_AutoReloadChangedFiles = FALSE;
	//}}AFX_DATA_INIT
	LoadHistory(m_Profile, _T("History"), _T("BinaryFiles%d"), m_sBinaryFilterHistory,
				sizeof m_sBinaryFilterHistory / sizeof m_sBinaryFilterHistory[0], true);

	LoadHistory(m_Profile, _T("History"), _T("CppFiles%d"), m_sCppFilterHistory,
				sizeof m_sCppFilterHistory / sizeof m_sCppFilterHistory[0], true);

	LoadHistory(m_Profile, _T("History"), _T("IgnoreFiles%d"), m_sIgnoreFilterHistory,
				sizeof m_sIgnoreFilterHistory / sizeof m_sIgnoreFilterHistory[0], true);
}

CFilesPreferencePage::~CFilesPreferencePage()
{
}

void CFilesPreferencePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilesPreferencePage)
	DDX_CBString(pDX, IDC_EDIT_BINARY_FILES, m_sBinaryFilesFilter);
	DDX_CBString(pDX, IDC_EDIT_C_CPP, m_sCppFilesFilter);
	DDX_CBString(pDX, IDC_EDIT_IGNORE, m_sIgnoreFilesFilter);
	DDX_Check(pDX, IDC_CHECK_AUTO_RELOAD, m_AutoReloadChangedFiles);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_EDIT_C_CPP, m_cbCppFilter);
	DDX_Control(pDX, IDC_EDIT_BINARY_FILES, m_cbBinaryFilter);
	DDX_Control(pDX, IDC_EDIT_IGNORE, m_cbIgnoreFilter);
	if (pDX->m_bSaveAndValidate)
	{
		AddStringToHistory(m_sBinaryFilesFilter, m_sBinaryFilterHistory,
							sizeof m_sBinaryFilterHistory / sizeof m_sBinaryFilterHistory[0], false);

		AddStringToHistory(m_sCppFilesFilter, m_sCppFilterHistory,
							sizeof m_sCppFilterHistory / sizeof m_sCppFilterHistory[0], false);

		AddStringToHistory(m_sIgnoreFilesFilter, m_sIgnoreFilterHistory,
							sizeof m_sIgnoreFilterHistory / sizeof m_sIgnoreFilterHistory[0], false);

		m_Profile.FlushAll();
	}
}


BEGIN_MESSAGE_MAP(CFilesPreferencePage, CPropertyPage)
	//{{AFX_MSG_MAP(CFilesPreferencePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilesPreferencePage message handlers

BOOL CFilesPreferencePage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	LoadHistoryCombo(m_cbBinaryFilter, m_sBinaryFilterHistory,
					sizeof m_sBinaryFilterHistory / sizeof m_sBinaryFilterHistory[0]);

	LoadHistoryCombo(m_cbCppFilter, m_sCppFilterHistory,
					sizeof m_sCppFilterHistory / sizeof m_sCppFilterHistory[0]);

	LoadHistoryCombo(m_cbIgnoreFilter, m_sIgnoreFilterHistory,
					sizeof m_sIgnoreFilterHistory / sizeof m_sIgnoreFilterHistory[0]);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


/////////////////////////////////////////////////////////////////////////////
// CComparisionPreferencesPage property page

IMPLEMENT_DYNCREATE(CComparisionPreferencesPage, CPropertyPage)

CComparisionPreferencesPage::CComparisionPreferencesPage() : CPropertyPage(CComparisionPreferencesPage::IDD)
{
	//{{AFX_DATA_INIT(CComparisionPreferencesPage)
	m_MinimalLineLength = 0;
	m_NumberOfIdenticalLines = 0;
	m_PercentsOfLookLikeDifference = 0;
	m_MinMatchingChars = 0;
	m_bUseMd5 = TRUE;
	//}}AFX_DATA_INIT
}

CComparisionPreferencesPage::~CComparisionPreferencesPage()
{
}

void CComparisionPreferencesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CComparisionPreferencesPage)
	DDX_Text(pDX, IDC_EDIT_MINIMAL_LINE_LENGTH, m_MinimalLineLength);
	DDV_MinMaxUInt(pDX, m_MinimalLineLength, 1, 2048);
	DDX_Text(pDX, IDC_EDIT_NUMBER_OF_IDENTICAL_LINES, m_NumberOfIdenticalLines);
	DDV_MinMaxUInt(pDX, m_NumberOfIdenticalLines, 1, 50);
	DDX_Text(pDX, IDC_EDIT_PERCENT_OF_LOOKLIKE_DIFFERENCE, m_PercentsOfLookLikeDifference);
	DDV_MinMaxUInt(pDX, m_PercentsOfLookLikeDifference, 0, 99);
	DDX_Text(pDX, IDC_EDIT_MIN_MATCHING_CHARS, m_MinMatchingChars);
	DDV_MinMaxUInt(pDX, m_MinMatchingChars, 1, 32);
	DDX_Check(pDX, IDC_CHECK_USE_MD5, m_bUseMd5);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CComparisionPreferencesPage, CPropertyPage)
	//{{AFX_MSG_MAP(CComparisionPreferencesPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CComparisionPreferencesPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
/////////////////////////////////////////////////////////////////////////////
// CComparisionPreferencesPage message handlers
/////////////////////////////////////////////////////////////////////////////
// CViewPreferencesPage property page

IMPLEMENT_DYNCREATE(CViewPreferencesPage, CPropertyPage)

CViewPreferencesPage::CViewPreferencesPage() : CPropertyPage(CViewPreferencesPage::IDD)
{
	//{{AFX_DATA_INIT(CViewPreferencesPage)
	m_nTabIndent = 0;
	m_bCancelSelectionOnMerge = FALSE;
	//}}AFX_DATA_INIT
	m_FontPointSize = 100;
	m_bFontChanged = false;
	m_bColorChanged = false;
}

CViewPreferencesPage::~CViewPreferencesPage()
{
}

void CViewPreferencesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewPreferencesPage)
	DDX_Control(pDX, IDC_SPIN1, m_Spin);
	DDX_Text(pDX, IDC_EDIT_TAB_INDENT, m_nTabIndent);
	DDV_MinMaxUInt(pDX, m_nTabIndent, 0, 32);
	DDX_Check(pDX, IDC_CHECK_CANCEL_SELECTION_ON_MERGE, m_bCancelSelectionOnMerge);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewPreferencesPage, CPropertyPage)
	//{{AFX_MSG_MAP(CViewPreferencesPage)
	ON_BN_CLICKED(IDC_BUTTON_NORMAL_FONT, OnButtonNormalFont)
	ON_BN_CLICKED(IDC_BUTTON_INSERTED_FONT, OnButtonInsertedFont)
	ON_BN_CLICKED(IDC_BUTTON_ERASED_FONT, OnButtonErasedFont)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_ADDED_BACKGROUND, OnButtonAddedBackground)
	ON_BN_CLICKED(IDC_BUTTON_ERASED_BACKGROUND, OnButtonErasedBackground)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CViewPreferencesPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Spin.SetRange(1, 32);

	FontChanged();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CViewPreferencesPage::FontChanged()
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

	pWnd = GetDlgItem(IDC_STATIC_DISCARDED_TEXT);
	if (pWnd)
	{
		pWnd->SetFont( & m_NormalFont, TRUE);
	}

	pWnd = GetDlgItem(IDC_STATIC_ACCEPTED_TEXT);
	if (pWnd)
	{
		pWnd->SetFont( & m_NormalFont, TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CViewPreferencesPage message handlers

void CViewPreferencesPage::OnButtonNormalFont()
{
	CString Title;
	Title.LoadString(IDS_STRING_NORMAL_FONT_TITLE);
	CFontDialogTitle dlg( & m_NormalLogFont,
						CF_FIXEDPITCHONLY
						| CF_INITTOLOGFONTSTRUCT
						| CF_NOVECTORFONTS
						| CF_SCREENFONTS
						| CF_EFFECTS, Title);
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

void CViewPreferencesPage::OnButtonInsertedFont()
{
	CString Title;
	Title.LoadString(IDS_STRING_ADDED_TEXT_TITLE);
	CFontDialogTitle dlg( & m_AddedLogFont,
						CF_FIXEDPITCHONLY
						| CF_INITTOLOGFONTSTRUCT
						| CF_NOVECTORFONTS
						| CF_SCREENFONTS
						| CF_EFFECTS, Title);

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

void CViewPreferencesPage::OnButtonErasedFont()
{
	CString Title;
	Title.LoadString(IDS_STRING_DELETED_FONT_TITLE);
	CFontDialogTitle dlg( & m_ErasedLogFont,
						CF_FIXEDPITCHONLY
						| CF_INITTOLOGFONTSTRUCT
						| CF_NOVECTORFONTS
						| CF_SCREENFONTS
						| CF_EFFECTS, Title);

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

HBRUSH CViewPreferencesPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT id = pWnd->GetDlgCtrlID();
	if (pDC->m_hDC != NULL)
	{
		switch (id)
		{
		case IDC_STATIC_NORMAL_TEXT:
			pDC->SetTextColor(m_NormalTextColor);
			hbr = GetSysColorBrush(COLOR_WINDOW);
			pDC->SetBkColor(m_NormalTextBackground);
			pDC->SetBkMode(OPAQUE);
			break;
		case IDC_STATIC_ADDED_TEXT:
			pDC->SetTextColor(m_AddedTextColor);
			hbr = GetSysColorBrush(COLOR_WINDOW);
			pDC->SetBkColor(m_NormalTextBackground);
			pDC->SetBkMode(OPAQUE);
			break;
		case IDC_STATIC_DELETED_TEXT:
			pDC->SetTextColor(m_ErasedTextColor);
			hbr = GetSysColorBrush(COLOR_WINDOW);
			pDC->SetBkColor(m_NormalTextBackground);
			pDC->SetBkMode(OPAQUE);
			break;
		case IDC_STATIC_ACCEPTED_TEXT:
			pDC->SetTextColor(m_NormalTextColor);
			hbr = GetSysColorBrush(COLOR_WINDOW);
			pDC->SetBkColor(m_AddedTextBackground);
			pDC->SetBkMode(OPAQUE);
			break;
		case IDC_STATIC_DISCARDED_TEXT:
			pDC->SetTextColor(m_NormalTextColor);
			hbr = GetSysColorBrush(COLOR_WINDOW);
			pDC->SetBkColor(m_ErasedTextBackground);
			pDC->SetBkMode(OPAQUE);
			break;
		}
	}
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

/////////////////////////////////////////////////////////////////////////////
// CPreferencesPropertySheet

IMPLEMENT_DYNAMIC(CPreferencesPropertySheet, CPropertySheet)

CPreferencesPropertySheet::CPreferencesPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	AddPage( & m_FilesPage);
	AddPage( & m_ComparisionPage);
	AddPage( & m_ViewPage);
}

CPreferencesPropertySheet::CPreferencesPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	AddPage( & m_FilesPage);
	AddPage( & m_ComparisionPage);
	AddPage( & m_ViewPage);
}

CPreferencesPropertySheet::~CPreferencesPropertySheet()
{
}


BEGIN_MESSAGE_MAP(CPreferencesPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CPreferencesPropertySheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CViewPreferencesPage::OnButtonAddedBackground()
{
	CColorDialog dlg(m_AddedTextBackground, CC_RGBINIT | CC_PREVENTFULLOPEN | CC_SOLIDCOLOR);
	if (IDOK == dlg.DoModal())
	{
		m_AddedTextBackground = dlg.GetColor();
		CWnd * pWnd = GetDlgItem(IDC_STATIC_ACCEPTED_TEXT);
		if (pWnd)
		{
			pWnd->Invalidate();
		}
		m_bColorChanged = true;
	}
}

void CViewPreferencesPage::OnButtonErasedBackground()
{
	CColorDialog dlg(m_ErasedTextBackground, CC_RGBINIT | CC_PREVENTFULLOPEN | CC_SOLIDCOLOR);
	if (IDOK == dlg.DoModal())
	{
		m_ErasedTextBackground = dlg.GetColor();
		CWnd * pWnd = GetDlgItem(IDC_STATIC_DISCARDED_TEXT);
		if (pWnd)
		{
			pWnd->Invalidate();
		}
		m_bColorChanged = true;
	}
}
