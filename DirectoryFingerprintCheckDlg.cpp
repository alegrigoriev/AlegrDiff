// DirectoryFingerprintCheckDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DirectoryFingerprintCheckDlg.h"
#include "AlegrDiffDoc.h"
#include <io.h>
#include <fcntl.h>
#include <afxpriv.h>

// CDirectoryFingerprintCheckDlg dialog

IMPLEMENT_DYNAMIC(CDirectoryFingerprintCheckDlg, CDialog)
CDirectoryFingerprintCheckDlg::CDirectoryFingerprintCheckDlg(
															CAlegrDiffDoc * pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CDirectoryFingerprintCheckDlg::IDD, pParent)
	, m_pDocument(pDoc)
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

CDirectoryFingerprintCheckDlg::~CDirectoryFingerprintCheckDlg()
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

void CDirectoryFingerprintCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FILENAME, m_Filename);
	DDX_Control(pDX, IDC_STATIC_PERCENT, m_ProgressPercent);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
}


BEGIN_MESSAGE_MAP(CDirectoryFingerprintCheckDlg, CDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()


// CDirectoryFingerprintCheckDlg message handlers

INT_PTR CDirectoryFingerprintCheckDlg::DoModal()
{
	m_pFile = _tfopen(m_FingerprintFilename, _T("rb"));
	if (NULL == m_pFile)
	{
		CString s;
		s.Format(IDS_STRING_CANT_OPEN_FILE, LPCTSTR(m_FingerprintFilename));
		AfxMessageBox(s, MB_OK | MB_ICONSTOP);
		return -1;
	}

	wchar_t FirstChar = fgetwc(m_pFile);

	clearerr(m_pFile);

	if ((FirstChar & 0xFFFF) == 0xFEFF)
	{
		m_bSaveAsUnicode = true;
	}
	else
	{
		rewind(m_pFile);
		_setmode(_fileno(m_pFile), _O_TEXT);
	}

	return CDialog::DoModal();
}

BOOL CDirectoryFingerprintCheckDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	TRACE("CDirectoryFingerprintCheckDlg::OnInitDialog()\n");

	m_Progress.SetRange(0, 100);

	m_StopRunThread = FALSE;
	m_Thread.CreateThread(0, 0x10000);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CDirectoryFingerprintCheckDlg::OnKickIdle(WPARAM, LPARAM)
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

unsigned CDirectoryFingerprintCheckDlg::_ThreadProc()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	// load the directory
	FileList FileList1;
	FileList FileList2;
	int i;

	CString ExclusionPattern(PatternToMultiCString(m_sIgnoreFiles));
	CString InclusionPattern(PatternToMultiCString(m_sFilenameFilter));

	// make full names from the directories
	LPTSTR pFilePart;
	LPCTSTR crlf = _T("\n");
	if (m_bSaveAsUnicode)
	{
		crlf = _T("\r\n");
	}

	// load the fingerprint file

	TCHAR buf[1024];
	TCHAR FileName[512];
	while(NULL != _fgetts(buf, 1023, m_pFile))
	{
		buf[1023] = 0;
		FileName[511] = 0;

		LONGLONG FileLength;
		ULONG MD5[16];

		int NumScannedItems = _stscanf(buf,
										_T("\"%511[^\"]\" %I64u ")
										_T("%2x%2x%2x%2x")
										_T("%2x%2x%2x%2x")
										_T("%2x%2x%2x%2x")
										_T("%2x%2x%2x%2x")
										_T("\n")
										,
										FileName, & FileLength,
										& MD5[0], & MD5[1], & MD5[2], & MD5[3],
										& MD5[4], & MD5[5], & MD5[6], & MD5[7],
										& MD5[8], & MD5[9], & MD5[10], & MD5[11],
										& MD5[12], & MD5[13], & MD5[14], & MD5[15]);

		if (NumScannedItems < 1)
		{
			// error
			break;
		}

		// find the last '\'
		LPTSTR Dir = _tcsrchr(FileName, '\\');
		LPTSTR NamePart;
		CString SubDir;
		FileItem * pFile;

		WIN32_FIND_DATA wfd;
		memzero(wfd);

		if (NULL != Dir && 0 == Dir[1])
		{
			wfd.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			// find another backslash
			while (Dir != FileName)
			{
				Dir--;
				if ('\\' == *Dir)
				{
					break;
				}
			}
			if (Dir == FileName)
			{
				// another backslash not found
				Dir = NULL;
			}
			if (NumScannedItems != 1)
			{
				break;
			}
		}
		else
		{
			if (NumScannedItems != 18)
			{
				// error
				break;
			}
			wfd.nFileSizeLow = DWORD(FileLength);
			wfd.nFileSizeHigh = DWORD(FileLength >> 32);
		}

		if (NULL == Dir)
		{
			NamePart = FileName;
		}
		else
		{
			NamePart = Dir + 1;
			SubDir = CString(FileName, Dir - FileName + 1);
		}
		_tcsncpy(wfd.cFileName, NamePart, sizeof wfd.cFileName / sizeof wfd.cFileName[0] - 1);
		pFile = new FileItem(& wfd, CString(), SubDir);

		pFile->m_pNext = FileList1.m_pList;
		FileList1.m_pList = pFile;
		FileList1.m_NumFiles++;
	}

	GetFullPathName(m_sDirectory, MAX_PATH, buf, & pFilePart);

	if (InclusionPattern.IsEmpty())
	{
		InclusionPattern = '*';
	}

	if (! FileList2.LoadFolder(buf, m_bIncludeSubdirectories,
								InclusionPattern, ExclusionPattern, PatternToMultiCString(_T("*")),
								PatternToMultiCString(_T(""))))
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

	fclose(m_pFile);

	m_pFile = NULL;

	m_pDocument->BuildFilePairList(FileList1, FileList2);

	m_TotalDataSize = 0;

	// TODO: read files, verify MD5

	if (NULL != m_hWnd)
	{
		::PostMessage(m_hWnd, WM_COMMAND, IDOK, NULL);
	}
	return 0;
}