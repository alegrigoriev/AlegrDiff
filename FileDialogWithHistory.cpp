// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
// FileDialogWithHistory.cpp : implementation file
//

#include "stdafx.h"
//#include "WaveSoapFront.h"
#include "FileDialogWithHistory.h"
#include <Dlgs.h>
#include "resource.h"
#include <shlwapi.h>
#include <cderr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL AFXAPI AfxUnhookWindowCreate();
/////////////////////////////////////////////////////////////////////////////
// CFileDialogWithHistory
bool SupportsV5FileDialog()
{
	OSVERSIONINFO VersionInfo;
	VersionInfo.dwOSVersionInfoSize = sizeof VersionInfo;

	GetVersionEx( & VersionInfo);

	TRACE("Major version=%d, minor version=%d, build=%X\n",
		VersionInfo.dwMajorVersion,
		VersionInfo.dwMinorVersion,
		VersionInfo.dwBuildNumber);
	switch (VersionInfo.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		return VersionInfo.dwMajorVersion >= 5;
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		return VersionInfo.dwMajorVersion > 4
				|| (VersionInfo.dwMajorVersion == 4
					&& VersionInfo.dwMinorVersion >= 90);
		break;
	default:
		return FALSE;

	}
}

IMPLEMENT_DYNAMIC(CResizableFileDialog, CFileDialog)

BEGIN_MESSAGE_MAP(CResizableFileDialog, BaseClass)
	//{{AFX_MSG_MAP(CResizableFileDialog)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

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
	: BaseClass(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
	, m_RecentFolders(& m_Profile, Section, KeyFormat, NumStrings)
{
	if (CThisApp::SupportsV5FileDialog())
	{
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_WITH_HISTORY_TEMPLATE_V5);
	}
	else
	{
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_WITH_HISTORY_TEMPLATE_V4);
	}

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
	: BaseClass(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
	, m_RecentFolders(pSourceHistory)
{
	if (CThisApp::SupportsV5FileDialog())
	{
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_WITH_HISTORY_TEMPLATE_V5);
	}
	else
	{
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_WITH_HISTORY_TEMPLATE_V4);
	}

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
	if (NULL == pCb)
	{
		return;
	}
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

INT_PTR CResizableFileDialog::DoModal()
{
	// proxy for MSLU fix under Windows ME
	while (1)
	{
		UINT result = BaseClass::DoModal();
		if (result != 0 || CommDlgExtendedError() != FNERR_INVALIDFILENAME)
		{
			return result;
		}
		m_ofn.lpstrFile[0] = 0;
		m_ofn.lpstrInitialDir = NULL;
	}
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
	return BaseClass::DoModal();
}

void CFileDialogWithHistory::OnInitDone()
{
	BaseClass::OnInitDone();

	CComboBox * pCb = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_RECENT));
	if (NULL != pCb)
	{
		pCb->SetExtendedUI();
		m_RecentFolders.LoadCombo(pCb);
	}
}

BOOL CFileDialogWithHistory::OnFileNameOK()
{
	GetParent()->SendMessage(CDM_GETFOLDERPATH, MAX_PATH, LPARAM(m_LastFolder.GetBuffer(MAX_PATH)));
	m_LastFolder.ReleaseBuffer();

	TRACE(_T("CFileDialogWithHistory::OnFileNameOK Folder Path=%s\n"), m_LastFolder);

	m_RecentFolders.AddString(m_LastFolder);

	m_RecentFolders.Flush();
	return 0;
}

void CFileDialogWithHistory::OnFolderChange()
{
	m_LastFolder = GetFolderPath();
	if (m_LastFolder.GetLength() > 1
		&& (m_LastFolder[m_LastFolder.GetLength() - 1] == '\\'
			|| m_LastFolder[m_LastFolder.GetLength() - 1] == '/'))
	{
		m_LastFolder.SetAt(m_LastFolder.GetLength() - 1, 0);
	}
	CComboBox * pCb = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_RECENT));
	if (NULL != pCb)
	{
		pCb->SelectString(-1, m_LastFolder);
	}
}

CString CResizableFileDialog::GetNextPathName(POSITION& pos) const
{
	if (0 == (m_ofn.Flags & OFN_EXPLORER)
		|| (m_ofn.Flags & OFN_ALLOWMULTISELECT) == 0)
	{
		return BaseClass::GetNextPathName(pos);
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
	if (PathIsUNC(lpszFileName))
	{
		return lpszFileName;
	}

	// TODO: if the current directory is network directory,
	// try not to change the directory
	// if the name doesn't start with \\ or //
	// set the path as current folder, then get fully qualified names
	TCHAR OldCurrentDir[MAX_PATH * 2];

	//TODO: avoid using SetCurrentDirectory, use CPath instead
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
	BaseClass::OnSize(nType, cx, cy);
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

