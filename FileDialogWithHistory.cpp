// FileDialogWithHistory.cpp : implementation file
//

#include "stdafx.h"
//#include "WaveSoapFront.h"
#include "FileDialogWithHistory.h"
#include <Dlgs.h>
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL AFXAPI AfxUnhookWindowCreate();
/////////////////////////////////////////////////////////////////////////////
// CFileDialogWithHistory

using CResizableFileDialog::CParentWnd;
IMPLEMENT_DYNAMIC(CResizableFileDialog, CFileDialog)
IMPLEMENT_DYNAMIC(CParentWnd, CWnd)

BEGIN_MESSAGE_MAP(CResizableFileDialog, CFileDialog)
	//{{AFX_MSG_MAP(CResizableFileDialog)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CParentWnd, CWnd)
	ON_MESSAGE(CDM_GETFILEPATH, OnGetFilePath)
	ON_MESSAGE(CDM_GETFOLDERPATH, OnGetFolderPath)
	ON_MESSAGE(CDM_GETSPEC, OnGetSpec)
	ON_MESSAGE(CDM_SETCONTROLTEXT, OnSetControlText)
	ON_MESSAGE(CDM_SETDEFEXT, OnSetDefExt)
END_MESSAGE_MAP()

LRESULT CParentWnd::OnGetFilePath(WPARAM wParam, LPARAM lParam)
{
	CStringA strA;
	LPSTR buf = strA.GetBuffer(wParam);
	// get in MBCS
	LRESULT lresult = DefWindowProc(CDM_GETFILEPATH, wParam, LPARAM(buf));
	// convert to unicode
	if (lresult > 0 && unsigned(lresult) < wParam)
	{
		CStringW strW(strA, lresult - 1);
		LPWSTR bufW = LPWSTR(lParam);
		wcsncpy(bufW, strW, wParam - 1);
		bufW[wParam - 1] = 0;
		return strW.GetLength() + 1;
	}
	else
	{
		return lresult;
	}
}

LRESULT CParentWnd::OnGetFolderPath(WPARAM wParam, LPARAM lParam)
{
	CStringA strA;
	LPSTR buf = strA.GetBuffer(wParam);
	// get in MBCS
	LRESULT lresult = DefWindowProc(CDM_GETFOLDERPATH, wParam, LPARAM(buf));
	// convert to unicode
	if (lresult > 0 && unsigned(lresult) < wParam)
	{
		CStringW strW(strA, lresult - 1);
		LPWSTR bufW = LPWSTR(lParam);
		wcsncpy(bufW, strW, wParam - 1);
		bufW[wParam - 1] = 0;
		return strW.GetLength() + 1;
	}
	else
	{
		return lresult;
	}
}

LRESULT CParentWnd::OnGetSpec(WPARAM wParam, LPARAM lParam)
{
	CStringA strA;
	LPSTR buf = strA.GetBuffer(wParam);
	// get in MBCS
	LRESULT lresult = DefWindowProc(CDM_GETSPEC, wParam, LPARAM(buf));
	// convert to unicode
	if (lresult > 0 && unsigned(lresult) < wParam)
	{
		CStringW strW(strA, lresult - 1);
		LPWSTR bufW = LPWSTR(lParam);
		wcsncpy(bufW, strW, wParam - 1);
		bufW[wParam - 1] = 0;
		return strW.GetLength() + 1;
	}
	else
	{
		return lresult;
	}
}
LRESULT CParentWnd::OnSetControlText(WPARAM wParam, LPARAM lParam)
{
	CStringA txt;
	txt = LPCWSTR(lParam);
	return DefWindowProc(CDM_SETCONTROLTEXT, wParam, LPARAM(LPCSTR(txt)));
}

LRESULT CParentWnd::OnSetDefExt(WPARAM wParam, LPARAM lParam)
{
	CStringA txt;
	txt = LPCWSTR(lParam);
	return DefWindowProc(CDM_SETDEFEXT, wParam, LPARAM(LPCSTR(txt)));
}

/////////////////////////////////////////////////////////////////////////////
// CFileDialogWithHistory

IMPLEMENT_DYNAMIC(CFileDialogWithHistory, CResizableFileDialog)

BEGIN_MESSAGE_MAP(CFileDialogWithHistory, CResizableFileDialog)
	//{{AFX_MSG_MAP(CFileDialogWithHistory)
	ON_CBN_SELENDOK(IDC_COMBO_RECENT, OnComboSelendOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CFileDialogWithHistory::CFileDialogWithHistory(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
												LPCTSTR lpszDefExt,
												LPCTSTR lpszFileName,
												DWORD dwFlags,
												LPCTSTR lpszFilter,
												CWnd* pParentWnd, LPCTSTR Section,
												LPCTSTR KeyFormat, int NumStrings)
	: CResizableFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
	, m_RecentFolders(& m_Profile, Section, KeyFormat, NumStrings)
{
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_OPEN_TEMPLATE);
	m_ofn.Flags |= OFN_ENABLETEMPLATE;

	static ResizableDlgItem const item = {IDC_COMBO_RECENT, ExpandRight};
	m_pResizeItems = & item;
	m_ResizeItemsCount = 1;
}

CFileDialogWithHistory::CFileDialogWithHistory(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
												CStringHistory * pSourceHistory, LPCTSTR lpszDefExt,
												LPCTSTR lpszFileName,
												DWORD dwFlags,
												LPCTSTR lpszFilter,
												CWnd* pParentWnd)
	: CResizableFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
	, m_RecentFolders(pSourceHistory)
{
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_OPEN_TEMPLATE);
	m_ofn.Flags |= OFN_ENABLETEMPLATE;

	static ResizableDlgItem const item = {IDC_COMBO_RECENT, ExpandRight};
	m_pResizeItems = & item;
	m_ResizeItemsCount = 1;
}

void CFileDialogWithHistory::OnComboSelendOK()
{
	TRACE("CFileDialogWithHistory::OnComboSelendOK()\n");
	CString str;
	CComboBox * pCb = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_RECENT));
	if (NULL != pCb)
	{
		int sel = pCb->GetCurSel();
		if (-1 == sel
			|| sel >= pCb->GetCount())
		{
			return;
		}
		pCb->GetLBText(sel, str);
		TRACE(_T("CFileDialogWithHistory::OnComboSelendOK: %s selected\n"), str);
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
			m_RecentFolders.DeleteString(str);

			pCb->DeleteString(sel);
			pCb->SetCurSel(-1); // no selection
			return;
		}
		else
		{
			TRACE("FindFirstFile success\n");
			FindClose(hFind);
			CWnd * pParent = GetParent();
			CWnd * pTmp = pParent->GetDlgItem(edt1);
			CString name;
			if (NULL == pTmp)
			{
				// new style dialog
				pTmp = pParent->GetDlgItem(cmb13);
			}
			if (NULL != pTmp
				&& ! m_bOpenFileDialog)
			{
				pTmp->GetWindowText(name);
			}
#ifdef _UNICODE
			OSVERSIONINFO vi;
			ZeroMemory(&vi, sizeof(OSVERSIONINFO));
			vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			::GetVersionEx(&vi);

			if (vi.dwPlatformId != VER_PLATFORM_WIN32_NT)
			{
				if (NULL != pTmp)
				{
					pTmp->SetWindowText(str);

					pParent->SendMessage(WM_COMMAND, IDOK, 0);

					pTmp->SetWindowText(name);
				}
			}
			else
#endif
			{
				pParent->SendMessage(CDM_SETCONTROLTEXT, edt1, LPARAM(LPCTSTR(str)));
				pParent->SendMessage(WM_COMMAND, IDOK, 0);

				pParent->SendMessage(CDM_SETCONTROLTEXT, edt1, LPARAM(LPCTSTR(name)));
			}
			if (NULL != pTmp)
			{
				pTmp->SetFocus();
			}
		}


	}
}

#ifdef _UNICODE
void CResizableFileDialog::UpdateOfn(OPENFILENAMEA & ofn)
{
	// move file name and the custom filter back
	CStringW s;

	if (ofn.Flags & OFN_ALLOWMULTISELECT)
	{
		AssignMultiSz(s, ofn.lpstrFile);
	}
	else
	{
		s = ofn.lpstrFile;
	}
	size_t length = s.GetLength() + 1;
	if (length <= m_pOFN->nMaxFile)
	{
		memcpy(m_pOFN->lpstrFile, LPCWSTR(s), length * sizeof (WCHAR));
	}

	if (m_pOFN->lpstrCustomFilter)
	{
		AssignMultiSz(s, ofn.lpstrCustomFilter);
		size_t length = s.GetLength() + 1;

		if (length <= m_pOFN->nMaxCustFilter)
		{
			memcpy(m_pOFN->lpstrCustomFilter, LPCWSTR(s), length * sizeof (WCHAR));
		}
	}

	// translate nFileOffset
	m_pOFN->nFileOffset = CStringW::StrTraits::GetBaseTypeLength(ofn.lpstrFile, ofn.nFileOffset);

	if (0 != ofn.nFileExtension)
	{
		m_pOFN->nFileExtension = CStringW::StrTraits::GetBaseTypeLength(ofn.lpstrFile, ofn.nFileExtension);
	}
	else
	{
		m_pOFN->nFileExtension = 0;
	}

	m_pOFN->nFilterIndex = ofn.nFilterIndex;
	m_pOFN->Flags = ofn.Flags;
}

UINT_PTR CALLBACK CResizableFileDialog::ResizableFileDialogHook(
																HWND hdlg,      // handle to child dialog box
																UINT uiMsg,     // message identifier
																WPARAM wParam,  // message parameter
																LPARAM lParam   // message parameter
																)
{
	OPENFILENAMEA* pOfn;
	CResizableFileDialog * pDlg;

	if (uiMsg != WM_NOTIFY)
	{
		// handle WM_INITDIALOG
		if (WM_INITDIALOG == uiMsg)
		{
			// subclass dialog
			pOfn = reinterpret_cast<OPENFILENAMEA*>(lParam);
			pDlg = reinterpret_cast<CResizableFileDialog *>(pOfn->lCustData);

			pDlg->m_ParentWnd.SubclassWindow(::GetParent(hdlg));

			lParam = LPARAM( & pDlg->m_ofn);
			return pDlg->m_ofn.lpfnHook(hdlg, uiMsg, wParam, LPARAM( & pDlg->m_ofn));
		}

		pDlg = dynamic_cast<CResizableFileDialog *>(CWnd::FromHandle(hdlg));
		if (NULL == pDlg)
		{
			return 0;
		}
		return pDlg->m_ofn.lpfnHook(hdlg, uiMsg, wParam, lParam);
	}

	OFNOTIFYA * pOfnA = reinterpret_cast<OFNOTIFYA *>(lParam);
	OFNOTIFYW ofnW;
	ofnW.pszFile = NULL;
	CStringW szFile;

	pOfn = pOfnA->lpOFN;

	pDlg = reinterpret_cast<CResizableFileDialog *>(pOfn->lCustData);
	// translate OPENFILENAME back

	pDlg->UpdateOfn(*pOfn);

	switch (pOfnA->hdr.code)
	{
	case CDN_INCLUDEITEM:
	{
		OFNOTIFYEXW ofex;
		OFNOTIFYEXA * pOfnEx = reinterpret_cast<OFNOTIFYEXA *>(lParam);

		ofex.hdr = pOfnEx->hdr;
		ofex.lpOFN = & pDlg->m_ofn;

		ofex.pidl = pOfnEx->pidl;
		ofex.psf = pOfnEx->psf;
		return ofex.lpOFN->lpfnHook(hdlg, uiMsg, wParam, LPARAM( & ofex));
	}
		break;
	case CDN_SHAREVIOLATION:
		szFile = pOfnA->pszFile;
		ofnW.pszFile = LPWSTR(LPCWSTR(szFile));
		// fall through
	case CDN_FOLDERCHANGE:
	case CDN_HELP:
	case CDN_INITDONE:
	case CDN_SELCHANGE:
	case CDN_FILEOK:
	case CDN_TYPECHANGE:
		ofnW.hdr = pOfnA->hdr;
		ofnW.lpOFN = & pDlg->m_ofn;
		return ofnW.lpOFN->lpfnHook(hdlg, uiMsg, wParam, LPARAM( & ofnW));
		break;
	default:
		pDlg = dynamic_cast<CResizableFileDialog *>(CWnd::FromHandle(hdlg));
		if (NULL == pDlg)
		{
			return 0;
		}
		return pDlg->m_ofn.lpfnHook(hdlg, uiMsg, wParam, lParam);
		break;
	}
}
#endif
INT_PTR CResizableFileDialog::DoModal()
{
	// proxy for MSLU fix under Windows ME
#ifdef _UNICODE
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof vi;
	GetVersionEx( & vi);

	if (vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
		&& vi.dwMajorVersion == 4
		&& vi.dwMinorVersion >= 90
		&& m_ofn.lStructSize != OPENFILENAME_SIZE_VERSION_400
		&& m_pOFN->Flags & OFN_EXPLORER)
	{
		OPENFILENAMEA ofn;
		memzero(ofn);

		ofn.lStructSize = m_pOFN->lStructSize;
		ofn.hwndOwner = m_pOFN->hwndOwner;
		ofn.hInstance = m_pOFN->hInstance;
		ofn.nMaxCustFilter = m_pOFN->nMaxCustFilter;
		ofn.nFilterIndex = m_pOFN->nFilterIndex;
		ofn.Flags = m_pOFN->Flags;

		ofn.pvReserved = m_pOFN->pvReserved;
		ofn.dwReserved = m_pOFN->dwReserved;
		ofn.FlagsEx = m_pOFN->FlagsEx;

		CStringA lpstrFilter;
		if (m_pOFN->lpstrFilter)
		{
			// it is multisz string
			AssignMultiSz(lpstrFilter, m_pOFN->lpstrFilter);
			ofn.lpstrFilter = lpstrFilter;
		}

		CStringA lpstrDefExt;
		if (m_pOFN->lpstrDefExt)
		{
			lpstrDefExt = m_pOFN->lpstrDefExt;
			ofn.lpstrDefExt = lpstrDefExt;
		}

		CStringA lpstrInitialDir;
		if (m_pOFN->lpstrInitialDir)
		{
			lpstrInitialDir = m_pOFN->lpstrInitialDir;
			ofn.lpstrInitialDir = lpstrInitialDir;
		}

		CStringA lpstrFileTitle;
		if (m_pOFN->lpstrFileTitle && m_pOFN->nMaxFileTitle)
		{
			ofn.lpstrFileTitle = lpstrFileTitle.GetBuffer(m_pOFN->nMaxFileTitle);
			ofn.nMaxFileTitle = m_pOFN->nMaxFileTitle;
		}

		CStringA lpstrTitle;
		if (m_pOFN->lpstrTitle)
		{
			lpstrTitle = m_pOFN->lpstrTitle;
			ofn.lpstrTitle = lpstrTitle;
		}

		CStringA lpstrCustomFilter;
		if (m_pOFN->lpstrCustomFilter)
		{
			// it is multisz string
			AssignMultiSz(lpstrCustomFilter, m_pOFN->lpstrCustomFilter);
			ofn.lpstrCustomFilter = lpstrCustomFilter.GetBuffer(m_pOFN->nMaxCustFilter);
			ofn.nMaxCustFilter = m_pOFN->nMaxCustFilter;

			DWORD nOffset = lpstrCustomFilter.GetLength() + 1;

			ASSERT(nOffset <= ofn.nMaxCustFilter);
			if (nOffset < ofn.nMaxCustFilter)
			{
				memset(ofn.lpstrCustomFilter + nOffset, 0, (ofn.nMaxCustFilter-nOffset)*sizeof(*ofn.lpstrCustomFilter));
			}
		}

		ofn.nFilterIndex = m_pOFN->nFilterIndex;

		CStringA lpTemplateName;
		ofn.lpTemplateName = LPCSTR(m_pOFN->lpTemplateName);

		if (0 != (0xFFFF0000 & ULONG_PTR(m_pOFN->lpTemplateName)))
		{
			lpTemplateName = m_pOFN->lpTemplateName;
			ofn.lpTemplateName = lpTemplateName;
		}

		ofn.lCustData = LPARAM(this);
		ofn.lpfnHook = ResizableFileDialogHook;

		CStringA lpstrFile(m_ofn.lpstrFile);
		DWORD nOffset = lpstrFile.GetLength() + 1;

		ofn.nMaxFile = m_ofn.nMaxFile;
		ofn.lpstrFile = lpstrFile.GetBuffer(ofn.nMaxFile);

		ASSERT(nOffset <= ofn.nMaxFile);
		if (nOffset < ofn.nMaxFile)
		{
			memset(ofn.lpstrFile + nOffset, 0, (ofn.nMaxFile-nOffset)*sizeof(*ofn.lpstrFile));
		}

		// WINBUG: This is a special case for the file open/save dialog,
		//  which sometimes pumps while it is coming up but before it has
		//  disabled the main window.
		HWND hWndFocus = ::GetFocus();
		BOOL bEnableParent = FALSE;
		m_ofn.hwndOwner = PreModal();
		AfxUnhookWindowCreate();
		if (ofn.hwndOwner != NULL && ::IsWindowEnabled(ofn.hwndOwner))
		{
			bEnableParent = TRUE;
			::EnableWindow(ofn.hwndOwner, FALSE);
		}

		_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
		ASSERT(pThreadState->m_pAlternateWndInit == NULL);

		pThreadState->m_pAlternateWndInit = this;

		INT_PTR nResult;
		if (m_bOpenFileDialog)
			nResult = ::GetOpenFileNameA( & ofn);
		else
			nResult = ::GetSaveFileNameA( & ofn);

		if (nResult)
		{
			ASSERT(pThreadState->m_pAlternateWndInit == NULL);
			UpdateOfn(ofn);
		}
		pThreadState->m_pAlternateWndInit = NULL;

		// WINBUG: Second part of special case for file open/save dialog.
		if (bEnableParent)
			::EnableWindow(ofn.hwndOwner, TRUE);
		if (::IsWindow(hWndFocus))
			::SetFocus(hWndFocus);

		PostModal();
		return nResult ? nResult : IDCANCEL;

	}
#endif
	return CFileDialog::DoModal();
}

INT_PTR CFileDialogWithHistory::DoModal()
{
	m_RecentFolders.Load();
	// if there is no initial directory set, extract it from the name or get from the history
	if (NULL == m_ofn.lpstrInitialDir
		|| 0 == m_ofn.lpstrInitialDir[0])
	{
		// get the initial dir from the name, if any specified, or the first from the history
		TCHAR Buf[512];
		LPTSTR NamePart = NULL;
		if (GetFullPathName(m_ofn.lpstrFile, countof(Buf), Buf, & NamePart)
			&& NULL != NamePart)
		{
			m_SubstituteInitialFolder = CString(Buf, NamePart - Buf);
		}
		else
		{
			if (0 != m_ofn.lpstrFile[0])
			{
				m_SubstituteInitialFolder = m_ofn.lpstrFile;
			}
			else
			{
				m_SubstituteInitialFolder = m_RecentFolders[0];
			}
		}
		m_ofn.lpstrInitialDir = m_SubstituteInitialFolder;
	}
	return CResizableFileDialog::DoModal();
}

void CFileDialogWithHistory::OnInitDone()
{
	CResizableFileDialog::OnInitDone();

	CComboBox * pCb = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_RECENT));
	if (NULL != pCb)
	{
		pCb->SetExtendedUI();
		m_RecentFolders.LoadCombo(pCb);
	}
}

BOOL CFileDialogWithHistory::OnFileNameOK()
{
	CString sCurrDir;
	GetParent()->SendMessage(CDM_GETFOLDERPATH, MAX_PATH, LPARAM(sCurrDir.GetBuffer(MAX_PATH)));
	sCurrDir.ReleaseBuffer();
	TRACE("CFileDialogWithHistory::OnFileNameOK Folder Path=%s\n", sCurrDir);

	m_RecentFolders.AddString(sCurrDir);

	m_RecentFolders.Flush();
	return 0;
}

void CFileDialogWithHistory::OnFolderChange()
{
	CString dir = GetFolderPath();
	if (dir.GetLength() > 1
		&& (dir[dir.GetLength() - 1] == '\\'
			|| dir[dir.GetLength() - 1] == '/'))
	{
		dir.SetAt(dir.GetLength() - 1, 0);
	}
	CComboBox * pCb = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_RECENT));
	if (NULL != pCb)
	{
		pCb->SelectString(-1, dir);
	}
}

CString CResizableFileDialog::GetNextPathName(POSITION& pos) const
{
	if (0 == (m_ofn.Flags & OFN_EXPLORER)
		|| (m_ofn.Flags & OFN_ALLOWMULTISELECT) == 0)
	{
		return CFileDialog::GetNextPathName(pos);
	}

	LPTSTR lpsz = (LPTSTR)pos;
	if (lpsz == m_ofn.lpstrFile) // first time
	{
		// find char pos after first Delimiter
		while( *lpsz != '\0')
			lpsz = _tcsinc(lpsz);
		lpsz = _tcsinc(lpsz);

		// if single selection then return only selection
		if (*lpsz == 0)
		{
			pos = NULL;
			return m_ofn.lpstrFile;
		}
	}

	ASSERT(lpsz > m_ofn.lpstrFile
			|| lpsz < m_ofn.lpstrFile + m_ofn.nMaxFile);

	if (lpsz < m_ofn.lpstrFile
		|| lpsz >= m_ofn.lpstrFile + m_ofn.nMaxFile)
	{
		pos = NULL;
		return CString();
	}

	LPCTSTR NewCurrentDir = m_ofn.lpstrFile;

	LPCTSTR lpszFileName = lpsz;

	// find char pos at next Delimiter
	while( *lpsz != '\0')
	{
		lpsz = _tcsinc(lpsz);
	}

	lpsz = _tcsinc(lpsz);
	if (*lpsz == '\0') // if double terminated then done
	{
		pos = NULL;
	}
	else
	{
		pos = (POSITION)lpsz;
	}

	// check if the filename is network path
	if ((lpszFileName[0] == '/' && lpszFileName[1] == '/')
		|| (lpszFileName[0] == '\\' && lpszFileName[1] == '\\'))
	{
		return lpszFileName;
	}

	// TODO: if the current directory is network directory,
	// try not to change the directory
	// if the name doesn't start with \\ or //
	// set the path as current folder, then get fully qualified names
	TCHAR OldCurrentDir[MAX_PATH * 2];

	// save old directory
	GetCurrentDirectory(countof(OldCurrentDir), OldCurrentDir);
	// set new current directory
	if (0 != _tcsicmp(OldCurrentDir, NewCurrentDir))
	{
		SetCurrentDirectory(NewCurrentDir);
	}

	TCHAR * pTitle;
	TCHAR FullPath[MAX_PATH * 2];

	GetFullPathName(lpszFileName, countof(FullPath), FullPath, & pTitle);

	if (0 != _tcsicmp(OldCurrentDir, NewCurrentDir))
	{
		SetCurrentDirectory(OldCurrentDir);
	}

	return FullPath;
}

size_t CResizableFileDialog::OpenfilenameSize()
{
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof vi;
	GetVersionEx( & vi);

	if ((vi.dwPlatformId == VER_PLATFORM_WIN32_NT
			&& vi.dwMajorVersion >= 5)
		|| (vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
			&& vi.dwMajorVersion == 4
			&& vi.dwMinorVersion >= 90))
	{
		return sizeof (OPENFILENAME);
	}
	else
	{
		return OPENFILENAME_SIZE_VERSION_400;
	}
}

void CResizableFileDialog::OnSize(UINT nType, int cx, int cy)
{
	TRACE("CResizableFileDialog::OnSize %d %d, hwdn=%x\n", cx, cy, m_hWnd);
	CFileDialog::OnSize(nType, cx, cy);
	// move dialog items
	// use _parent_ dialog size
	CRect r;
	GetParent()->GetClientRect( & r);
	if (-1 == m_PrevSize.cx)
	{
		m_PrevSize.cx = r.Width();
		m_PrevSize.cy = r.Height();
		return;
	}
	// only X coordinate or width needs change
	int dx = r.Width() - m_PrevSize.cx;
	m_PrevSize.cx = r.Width();
	m_PrevSize.cy = r.Height();
	if (0 == dx)
	{
		return;
	}

	if (0 != m_ResizeItemsCount)
	{
		HDWP hdwp = ::BeginDeferWindowPos(m_ResizeItemsCount);
		for (int i = 0; i < m_ResizeItemsCount && NULL != hdwp; i++)
		{
			HWND hWnd = ::GetDlgItem(GetSafeHwnd(), m_pResizeItems[i].Id);
			if (NULL == hWnd) continue;

			CRect cr;
			::GetWindowRect(hWnd, cr);
			ScreenToClient(cr);

			if (m_pResizeItems[i].flags & CenterHorizontally)
			{
				cr.right += (dx + (cx & 1)) >> 1;
				cr.left += (dx + (cx & 1)) >> 1;
			}
			else
			{
				if (m_pResizeItems[i].flags & (ExpandRight | MoveRight))
				{
					cr.right += dx;
				}
				if (m_pResizeItems[i].flags & MoveRight)
				{
					cr.left += dx;
				}
			}


			hdwp = ::DeferWindowPos(hdwp, hWnd, NULL, cr.left, cr.top,
									cr.Width(), cr.Height(),
									SWP_NOZORDER | SWP_NOOWNERZORDER// | SWP_NOACTIVATE | SWP_NOSENDCHANGING
									);
			if (1) TRACE("DeferWindowPos hwnd=%x dw=%d x=%d, y=%d returned %X\n",
						hWnd, dx, cr.left, cr.top, hdwp);
		}

		if (NULL != hdwp)
		{
			::EndDeferWindowPos(hdwp);
		}

	}

}

