// FolderDialog.cpp : implementation file
//

#include "stdafx.h"
#include "FolderDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFolderDialog

IMPLEMENT_DYNAMIC(CFolderDialog, CDialog)

CFolderDialog::CFolderDialog(LPCTSTR lpszTitle, LPCTSTR lpszStartingDirectory,
							DWORD dwFlags, CWnd* pParentWnd) :
	CCommonDialog(pParentWnd), szStartupDir(lpszStartingDirectory)
{
	memset( & m_bi, 0, sizeof m_bi);
	m_bi.ulFlags = dwFlags;
	m_bi.pszDisplayName = szBuffer;
	szBuffer[0] = 0;
	m_bi.lpszTitle = lpszTitle;
	m_bi.lpfn = BrowseCallbackProc;
	m_bi.lParam = (LPARAM) this;
}

BEGIN_MESSAGE_MAP(CFolderDialog, CCommonDialog)
	//{{AFX_MSG_MAP(CFolderDialog)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

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
	//AfxUnhookWindowCreate();
	if (m_bi.hwndOwner != NULL && ::IsWindowEnabled(m_bi.hwndOwner))
	{
		bEnableParent = TRUE;
		::EnableWindow(m_bi.hwndOwner, FALSE);
	}

#if 0
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	ASSERT(pThreadState->m_pAlternateWndInit == NULL);

	if (m_bi.Flags & OFN_EXPLORER)
		pThreadState->m_pAlternateWndInit = this;
	else
		AfxHookWindowCreate(this);
#endif

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

#if 0
	if (lpResult)
		ASSERT(pThreadState->m_pAlternateWndInit == NULL);
	pThreadState->m_pAlternateWndInit = NULL;
#endif

	// WINBUG: Second part of special case for file open/save dialog.
	if (bEnableParent)
		::EnableWindow(m_bi.hwndOwner, TRUE);
	if (::IsWindow(hWndFocus))
		::SetFocus(hWndFocus);

	PostModal();
	return (lpResult != 0) ? IDOK : IDCANCEL;
}

int CALLBACK CFolderDialog::BrowseCallbackProc(HWND hwnd,
												UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	CFolderDialog * pDlg = (CFolderDialog *) lpData;
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		return pDlg->OnInitDone(hwnd);
		break;
	case BFFM_SELCHANGED:
		return pDlg->OnFolderChange(hwnd, (LPITEMIDLIST) lParam);
		break;
#if 0
	case BFFM_VALIDATEFAILED:
		return pDlg->OnValidateFailed(hwnd, (LPCTSTR)lParam);
		break;
#endif
	default:
		return 0;
		break;
	}
}

int CFolderDialog::OnInitDone(HWND hwnd)
{
	if (! szStartupDir.IsEmpty())
	{
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE,
					(LPARAM)(LPCTSTR)szStartupDir);
	}

	return 0;
}

int CFolderDialog::OnFolderChange(HWND hwnd, LPITEMIDLIST lpItem)
{
	return 0;
}

int CFolderDialog::OnValidateFailed(HWND /*hwnd*/, LPCTSTR /*ErrorName*/)
{
	return TRUE;
}

CString CFolderDialog::GetFolderDisplayName() const
{
	return szDisplayName;
}

CString CFolderDialog::GetFolderPath() const
{
	return szPath;
}

#ifdef _DEBUG
void CFolderDialog::Dump(CDumpContext& dc) const
{
}
#endif

