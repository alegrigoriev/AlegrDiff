// DirectoryFingerpringCreateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DirectoryFingerpringCreateDlg.h"
#include <io.h>
#include <fcntl.h>
#include <afxpriv.h>


// CDirectoryFingerpringCreateDlg dialog

IMPLEMENT_DYNAMIC(CDirectoryFingerpringCreateDlg, CDialog)
CDirectoryFingerpringCreateDlg::CDirectoryFingerpringCreateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDirectoryFingerpringCreateDlg::IDD, pParent)
	, m_pFile(NULL)
	, m_bIncludeSubdirectories(FALSE)
	, m_bIncludeDirectoryStructure(FALSE)
	, m_bSaveAsUnicode(FALSE)
	, m_StopRunThread(FALSE)
	, m_Thread(ThreadProc, this)
	, m_TotalDataSize(0)
	, m_ProcessedFiles(0)
	, m_CurrentFileDone(0)
{
	m_Thread.m_bAutoDelete = FALSE;
	m_hThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CDirectoryFingerpringCreateDlg::~CDirectoryFingerpringCreateDlg()
{
	if (m_Thread.m_hThread)
	{
		m_StopRunThread = TRUE;
		if (WAIT_TIMEOUT == WaitForSingleObject(m_Thread.m_hThread, 5000))
		{
			TerminateThread(m_Thread.m_hThread, -1);
		}
	}

	if (m_hThreadEvent)
	{
		CloseHandle(m_hThreadEvent);
	}
}

void CDirectoryFingerpringCreateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FILENAME, m_Filename);
	DDX_Control(pDX, IDC_STATIC_PERCENT, m_ProgressPercent);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
}


BEGIN_MESSAGE_MAP(CDirectoryFingerpringCreateDlg, CDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()


// CDirectoryFingerpringCreateDlg message handlers

INT_PTR CDirectoryFingerpringCreateDlg::DoModal()
{
	m_pFile = _tfopen(m_FingerprintFilename, _T("wt"));
	if (NULL == m_pFile)
	{
		CString s;
		s.Format(IDS_STRING_UNABLE_TO_CREATE_FILE, LPCTSTR(m_FingerprintFilename));
		AfxMessageBox(s, MB_OK | MB_ICONSTOP);
		return -1;
	}

	if (m_bSaveAsUnicode)
	{
		_setmode(_fileno(m_pFile), _O_BINARY);
		fputwc(0xFEFF, m_pFile);
	}

	return CDialog::DoModal();
}

BOOL CDirectoryFingerpringCreateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	TRACE("CDirectoryFingerpringCreateDlg::CWaveSoapFrontDoc()\n");

	m_Progress.SetRange(0, 100);

	m_StopRunThread = FALSE;
	m_Thread.CreateThread(0, 0x10000);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

unsigned CDirectoryFingerpringCreateDlg::_ThreadProc()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	// load the directory
	FileList FileList1;
	int i;

	// make full names from the directories
	LPTSTR pFilePart;
	TCHAR buf[MAX_PATH];
	LPCTSTR crlf = _T("\n");
	if (m_bSaveAsUnicode)
	{
		crlf = _T("\r\n");
	}

	GetFullPathName(m_sDirectory, MAX_PATH, buf, & pFilePart);

	CString ExclusionPattern(PatternToMultiCString(m_sIgnoreFiles));
	CString InclusionPattern(PatternToMultiCString(m_sFilenameFilter));

	if (InclusionPattern.IsEmpty())
	{
		InclusionPattern = '*';
	}

	if (! FileList1.LoadFolder(buf, m_bIncludeSubdirectories,
								InclusionPattern, ExclusionPattern, _T(""),
								_T("")))
	{
		DWORD error = GetLastError();
		CString s;
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, buf);
		AfxMessageBox(s);
		if (NULL != m_hWnd)
		{
			::PostMessage(m_hWnd, WM_COMMAND, IDCANCEL, NULL);
		}
		return 0;
	}

	CArray<FileItem *, FileItem *> Files1;

	FileList1.GetSortedList(Files1, FileList::SortDirFirst | FileList::SortBackwards);
	m_TotalDataSize = 0;
	for (i = 0; i < Files1.GetSize(); i++)
	{
		FileItem * pFile = Files1[i];
		if ( ! pFile->IsFolder())
		{
			// a file open is taken as equivalent of 8K
			m_TotalDataSize += pFile->GetFileLength() + 0x2000;
		}
	}

	CMd5HashCalculator HashCalc;

	for (i = 0; i < Files1.GetSize() && ! m_StopRunThread; i++)
	{
		FileItem * pFile = Files1[i];
		if (pFile->IsFolder())
		{
			if ( ! m_bIncludeDirectoryStructure)
			{
				continue;
			}
			_ftprintf(m_pFile, _T("\"%s%s\\\"%s"), pFile->GetSubdir(),
					pFile->GetName(), crlf);
			continue;
		}

		{
			CSimpleCriticalSectionLock lock(m_cs);
			m_CurrentFilename = pFile->GetFullName();
			m_ProcessedFiles += 0x2000;
		}

		if (NULL != m_hWnd)
		{
			::PostMessage(m_hWnd, WM_KICKIDLE, 0, 0);
		}
		if (pFile->CalculateHashes( & HashCalc, m_StopRunThread))
		{
			_ftprintf(m_pFile,
					_T("\"%s%s\" %I64d ")
					_T("%02x%02x%02x%02x")
					_T("%02x%02x%02x%02x")
					_T("%02x%02x%02x%02x")
					_T("%02x%02x%02x%02x")
					_T("%s"),
					pFile->GetSubdir(),
					pFile->GetName(),
					pFile->GetFileLength(),
					pFile->GetDigest(0), pFile->GetDigest(1), pFile->GetDigest(2), pFile->GetDigest(3),
					pFile->GetDigest(4), pFile->GetDigest(5), pFile->GetDigest(6), pFile->GetDigest(7),
					pFile->GetDigest(8), pFile->GetDigest(9), pFile->GetDigest(10), pFile->GetDigest(11),
					pFile->GetDigest(12), pFile->GetDigest(13), pFile->GetDigest(14), pFile->GetDigest(15),
					crlf);
		}
		else
		{
		}

		m_ProcessedFiles += pFile->GetFileLength();
	}

	fclose(m_pFile);

	m_pFile = NULL;
	if (NULL != m_hWnd)
	{
		::PostMessage(m_hWnd, WM_COMMAND, IDOK, NULL);
	}
	return 0;
}


void CDirectoryFingerpringCreateDlg::OnOK()
{
	CString s;
	s.Format(IDS_STRING_FINGERPRINT_CREATED, LPCTSTR(m_sDirectory), LPCTSTR(m_FingerprintFilename));
	AfxMessageBox(s, MB_OK);

	CDialog::OnOK();
}

LRESULT CDirectoryFingerpringCreateDlg::OnKickIdle(WPARAM, LPARAM)
{

	CSimpleCriticalSectionLock lock(m_cs);

	if (m_Filename.m_hWnd != NULL)
	{
		m_Filename.SetWindowText(m_CurrentFilename);
	}
	if (m_Progress.m_hWnd != NULL && m_TotalDataSize != 0)
	{
		m_Progress.SetPos(int(100. * (m_ProcessedFiles + m_CurrentFileDone) / m_TotalDataSize));
	}
	return 0;
}
