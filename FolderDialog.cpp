// FolderDialog.cpp : implementation file
//

#include "stdafx.h"
#include "FolderDialog.h"
#include "ApplicationProfile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFolderDialog

IMPLEMENT_DYNAMIC(CFolderDialog, CDialog)

CFolderDialog::CFolderDialog(LPCTSTR lpszTitle, LPCTSTR lpszStartingDirectory,
							bool EnableCreateFolder,
							DWORD dwFlags, CWnd* pParentWnd, class CStringHistory * pHistory)
	: CCommonDialog(pParentWnd),
	szStartupDir(lpszStartingDirectory),
	m_pStringHistory(pHistory),
	m_bEnableCreateDir(EnableCreateFolder)
{
	memzero(m_bi);
	m_bi.ulFlags = dwFlags;
	if (EnableCreateFolder)
	{
		m_bi.ulFlags |= BIF_EDITBOX;
	}

	if ( ! m_bEnableCreateDir)
	{
		m_bi.ulFlags |= BIF_NONEWFOLDERBUTTON;
	}

	m_bi.pszDisplayName = szBuffer;
	szBuffer[0] = 0;
	m_bi.lpszTitle = lpszTitle;
	m_bi.lpfn = BrowseCallbackProc;
	m_bi.lParam = (LPARAM) this;
}

BEGIN_MESSAGE_MAP(CFolderDialog, CCommonDialog)
	//{{AFX_MSG_MAP(CFolderDialog)
	ON_CBN_SELENDOK(0x3744, OnComboSelendOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void AFXAPI AfxHookWindowCreate(CWnd* pWnd);
BOOL AFXAPI AfxUnhookWindowCreate();
int CFolderDialog::DoModal()
{
	ASSERT_VALID(this);
	ASSERT(m_bi.lpfn != NULL); // can still be a user hook

	// WINBUG: This is a special case for the file open/save dialog,
	//  which sometimes pumps while it is coming up but before it has
	//  disabled the main window.
	HWND hWndFocus = ::GetFocus();
	BOOL bEnableParent = FALSE;
	m_bi.hwndOwner = PreModal();
	AfxUnhookWindowCreate();
	if (m_bi.hwndOwner != NULL && ::IsWindowEnabled(m_bi.hwndOwner))
	{
		bEnableParent = TRUE;
		::EnableWindow(m_bi.hwndOwner, FALSE);
	}

	HRESULT hr = OleInitialize(NULL);
//#if 0
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	ASSERT(pThreadState->m_pAlternateWndInit == NULL);

	if (m_bi.ulFlags & BIF_NEWDIALOGSTYLE)
		pThreadState->m_pAlternateWndInit = this;
	else
		AfxHookWindowCreate(this);
//#endif

	ASSERT(m_bi.lParam == (LPARAM) this);

	LPITEMIDLIST lpResult = ::SHBrowseForFolder(&m_bi);
	if (lpResult)
	{
		szDisplayName = m_bi.pszDisplayName;
		IMalloc * pMalloc = NULL;
		TCHAR Buffer[MAX_PATH + 1];
		if (SHGetPathFromIDList(lpResult, Buffer))
		{
			szPath = Buffer;
		}
		if (SUCCEEDED(SHGetMalloc( & pMalloc)))
		{
			pMalloc->Free(lpResult);
			pMalloc->Release();
			pMalloc = NULL;
		}
	}

	if (SUCCEEDED(hr))
	{
		OleUninitialize();
	}

//#if 0
	//if (lpResult)
	//	ASSERT(pThreadState->m_pAlternateWndInit == NULL);
	pThreadState->m_pAlternateWndInit = NULL;
//#endif

	// WINBUG: Second part of special case for file open/save dialog.
	if (bEnableParent)
		::EnableWindow(m_bi.hwndOwner, TRUE);
	if (::IsWindow(hWndFocus))
		::SetFocus(hWndFocus);

	PostModal();
	return (lpResult != 0) ? IDOK : IDCANCEL;
}

void CFolderDialog::LoadHistoryCb()
{
	if (NULL == m_pStringHistory)
	{
		return;
	}
	// find the edit box
	CRect r;
	CWnd * pEdit = GetDlgItem(0x3744);
	if (NULL == pEdit
		|| 0 == (WS_VISIBLE & pEdit->GetStyle()))
	{
		return;
	}
	pEdit->GetWindowRect( & r);
	ScreenToClient( & r);

	CFont * pFont = pEdit->GetFont();

	pEdit->DestroyWindow();

	if (m_HistoryCombo.Create(WS_CHILD
							| WS_VISIBLE
							| WS_VSCROLL
							| WS_TABSTOP
							| CBS_AUTOHSCROLL
							| CBS_DROPDOWN,
							r, this, 0x3744))
	{
		//m_HistoryCombo.Detach();
		m_HistoryCombo.SetFont(pFont);
		m_pStringHistory->LoadCombo( & m_HistoryCombo);
		m_HistoryCombo.SetExtendedUI();
	}
}

int CALLBACK CFolderDialog::BrowseCallbackProc(HWND hwnd,
												UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	CFolderDialog * pDlg = (CFolderDialog *) lpData;
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		TRACE("CFolderDialog::BrowseCallbackProc BFFM_INITIALIZED hwnd=%X\n", hwnd);
		if(pDlg->m_hWnd == NULL)
		{
			pDlg->SubclassWindow(hwnd);
		}
		ASSERT(pDlg->m_hWnd == hwnd);

		pDlg->LoadHistoryCb();
		return pDlg->OnInitDone();
		break;
	case BFFM_SELCHANGED:
		TRACE("CFolderDialog::BrowseCallbackProc BFFM_SELCHANGED hwnd=%X\n", hwnd);
		if(pDlg->m_hWnd == NULL)
		{
			pDlg->SubclassWindow(hwnd);
		}
		ASSERT(pDlg->m_hWnd == hwnd);
		return pDlg->OnFolderChange((LPITEMIDLIST) lParam);
		break;

	case BFFM_VALIDATEFAILED:
		TRACE("CFolderDialog::BrowseCallbackProc BFFM_VALIDATEFAILED hwnd=%X\n", hwnd);
		ASSERT(pDlg->m_hWnd == hwnd);
		return pDlg->OnValidateFailed((LPCTSTR)lParam);
		break;

	default:
		return 0;
		break;
	}
}

int CFolderDialog::OnInitDone()
{
	if ( ! m_bEnableCreateDir)
	{
		// disable and hide CreateDir button
		CWnd * button = GetDlgItem(0x3746);
		if (button)
		{
			button->EnableWindow(FALSE);
			button->ShowWindow(SW_HIDE);
		}
	}
	if (! szStartupDir.IsEmpty())
	{
		::SendMessage(m_hWnd, BFFM_SETSELECTION, TRUE,
					(LPARAM)(LPCTSTR)szStartupDir);
	}

	return 0;
}

int CFolderDialog::OnFolderChange(LPITEMIDLIST lpItem)
{
	return 0;
}

int CFolderDialog::OnValidateFailed(LPCTSTR /*ErrorName*/)
{
	return 0;
}

CString CFolderDialog::GetFolderDisplayName() const
{
	return szDisplayName;
}

CString CFolderDialog::GetFolderPath() const
{
	return szPath;
}

void CFolderDialog::EnableOK(BOOL Enable)
{
	SendMessage(BFFM_ENABLEOK, 0, Enable);
}

void CFolderDialog::SetExpanded(LPCWSTR Path)
{
	SendMessage(BFFM_SETEXPANDED, TRUE, LPARAM(Path));
}

void CFolderDialog::SetExpanded(LPITEMIDLIST Path)
{
	SendMessage(BFFM_SETEXPANDED, FALSE, LPARAM(Path));
}

void CFolderDialog::SetOkText(LPCWSTR Text)
{
	SendMessage(BFFM_SETOKTEXT, 0, LPARAM(Text));
}

void CFolderDialog::SetSelection(LPCTSTR Path)
{
	SendMessage(BFFM_SETSELECTION, TRUE, LPARAM(Path));
}

void CFolderDialog::SetSelection(LPITEMIDLIST Path)
{
	SendMessage(BFFM_SETSELECTION, FALSE, LPARAM(Path));
}

void CFolderDialog::SetStatusText(LPCTSTR Text)
{
	SendMessage(BFFM_SETSTATUSTEXT, 0, LPARAM(Text));
}

void CFolderDialog::OnIUnknown(IUnknown * pUnknown)
{
}

#ifdef _DEBUG
void CFolderDialog::Dump(CDumpContext& dc) const
{
}
#endif

#if 0
LRESULT CFolderDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WM_COMMAND == message && 0x3744 == LOWORD(wParam))
	{
		TRACE("WM_COMMAND %X\n", HIWORD(wParam));
	}
	return CCommonDialog::WindowProc(message, wParam, lParam);
}
#endif

void CFolderDialog::OnComboSelendOK()
{
	TRACE("CFolderDialog::OnComboSelendOK()\n");
	CString str;
	int sel = m_HistoryCombo.GetCurSel();
	if (-1 == sel
		|| sel >= m_HistoryCombo.GetCount())
	{
		return;
	}

	m_HistoryCombo.GetLBText(sel, str);
	TRACE(_T("CFolderDialog::OnComboSelendOK: %s selected\n"), str);
	if (str.IsEmpty())
	{
		return;
	}
	// check if the selected text is a folder
	// make sure we can find a file in the folder
	CString dir(str);
	TCHAR c = dir[dir.GetLength() - 1];
	if (c != ':'
		&& c != '\\'
		&& c != '/')
	{
		dir += '\\';
	}
	dir += '*';

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(dir, & wfd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		DWORD error = GetLastError();
		TRACE("FindFirstFile failed, last error = %d\n", error);
		CString s;
		if (ERROR_ACCESS_DENIED == error)
		{
			s.Format(IDS_DIRECTORY_ACCESS_DENIED, LPCTSTR(str));
		}
		else if (1 || ERROR_DIRECTORY == error
				|| ERROR_PATH_NOT_FOUND == error
				|| ERROR_INVALID_NAME == error
				|| ERROR_BAD_NETPATH)
		{
			s.Format(IDS_DIRECTORY_NOT_FOUND, LPCTSTR(str));
		}
		AfxMessageBox(s);
		// delete the string from combobox
		// delete also from the application list
		m_pStringHistory->DeleteString(str);

		m_HistoryCombo.DeleteString(sel);
		m_HistoryCombo.SetCurSel(-1); // no selection
		return;
	}

	TRACE("FindFirstFile success\n");
	FindClose(hFind);

	SendMessage(BFFM_SETSELECTION, TRUE,
				(LPARAM)(LPCTSTR)str);
}
