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
	, m_bFilenameChanged(TRUE)
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

	UINT_PTR result = CDialog::DoModal();
	if (m_Thread.m_hThread)
	{
		m_StopRunThread = TRUE;
		if (WAIT_TIMEOUT == WaitForSingleObject(m_Thread.m_hThread, 5000))
		{
			TerminateThread(m_Thread.m_hThread, -1);
		}
	}

	return result;
}

BOOL CDirectoryFingerpringCreateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	TRACE("CDirectoryFingerpringCreateDlg::OnInitDialog()\n");

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
	unsigned i;

	// make full names from the directories
	LPTSTR pFilePart;
	TCHAR FullDirectoryName[MAX_PATH];
	LPCTSTR crlf = _T("\n");
	if (m_bSaveAsUnicode)
	{
		crlf = _T("\r\n");
	}

	GetFullPathName(m_sDirectory, MAX_PATH, FullDirectoryName, & pFilePart);

	CString ExclusionPattern(PatternToMultiCString(m_sIgnoreFiles));
	CString InclusionPattern(PatternToMultiCString(m_sFilenameFilter));

	if (InclusionPattern.IsEmpty())
	{
		m_sFilenameFilter = '*';
		InclusionPattern = PatternToMultiCString(_T("*"));
	}

	if (! FileList1.LoadFolder(FullDirectoryName, m_bIncludeSubdirectories != 0,
								InclusionPattern, ExclusionPattern, PatternToMultiCString(_T("")),
								PatternToMultiCString(_T(""))))
	{
		DWORD error = GetLastError();
		CString s;
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, FullDirectoryName);
		AfxMessageBox(s);
		if (NULL != m_hWnd)
		{
			::PostMessage(m_hWnd, WM_COMMAND, IDCANCEL, NULL);
		}
		return 0;
	}

	vector<FileItem *> Files1;

	FileList1.GetSortedList(Files1, FileList::SortDirFirst);
	m_TotalDataSize = 0;
	for (i = 0; i < Files1.size(); i++)
	{
		FileItem * pFile = Files1[i];
		if ( ! pFile->IsFolder())
		{
			// a file open is taken as equivalent of 8K
			m_TotalDataSize += pFile->GetFileLength() + 0x2000;
		}
	}

	CMd5HashCalculator HashCalc;
	// save inclusion pattern, exclusion pattern,
	// "IncludeSubdirs", "IncludeDirs"
	int const TimeBufSize = 64;
	TCHAR time[TimeBufSize] = {0};
	TCHAR date[TimeBufSize] = {0};
	SYSTEMTIME SystemTime;
	SYSTEMTIME LocalTime;
	memzero(LocalTime);

	GetSystemTime( & SystemTime);
	SystemTimeToTzSpecificLocalTime(NULL, & SystemTime, & LocalTime);

	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, & LocalTime, NULL, date, TimeBufSize - 1);

	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, & LocalTime, NULL, time, TimeBufSize - 1);

	_ftprintf(m_pFile,
			_T("; Fingerprint of \"%s\", created %s %s%s")
			_T("IncludeFiles=%s%s")
			_T("ExcludeFiles=%s%s")
			_T("IncludeSubdirs=%d%s")
			_T("IncludeDirInfo=%d%s%s"),
			FullDirectoryName,
			date, time, crlf,
			LPCTSTR(m_sFilenameFilter), crlf,
			LPCTSTR(m_sIgnoreFiles), crlf,
			m_bIncludeSubdirectories, crlf,
			m_bIncludeDirectoryStructure, crlf, crlf);


	for (i = 0; i < Files1.size() && ! m_StopRunThread; i++)
	{
		FileItem * pFile = Files1[i];
		if (pFile->IsFolder())
		{
			if ( ! m_bIncludeDirectoryStructure)
			{
				continue;
			}
			_ftprintf(m_pFile, _T("\"%s%s\"%s"), pFile->GetSubdir(),
					pFile->GetName(), crlf);
			continue;
		}

		{
			CSimpleCriticalSectionLock lock(m_cs);

			m_CurrentFilename = pFile->GetFullName();
			m_bFilenameChanged = TRUE;
			m_CurrentFileDone = 0;
			m_ProcessedFiles += 0x2000;
		}

		if (NULL != m_hWnd)
		{
			::PostMessage(m_hWnd, WM_KICKIDLE, 0, 0);
		}
		if (pFile->CalculateHashes( & HashCalc,
									m_StopRunThread, m_CurrentFileDone, m_hWnd))
		{
			_ftprintf(m_pFile,
					_T("\"%s%s\" %I64d %016I64X ")
					_T("%02X%02X%02X%02X")
					_T("%02X%02X%02X%02X")
					_T("%02X%02X%02X%02X")
					_T("%02X%02X%02X%02X")
					_T("%s"),
					pFile->GetSubdir(),
					pFile->GetName(),
					pFile->GetFileLength(),
					pFile->GetLastWriteTime(),
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

	if (m_Filename.m_hWnd != NULL && m_bFilenameChanged)
	{
		m_Filename.SetWindowText(m_CurrentFilename);
		m_bFilenameChanged = FALSE;
	}
	if (m_Progress.m_hWnd != NULL && m_TotalDataSize != 0)
	{
		int Percent = int(100. * (m_ProcessedFiles + m_CurrentFileDone) / m_TotalDataSize);
		if (Percent != m_Progress.GetPos())
		{
			m_Progress.SetPos(Percent);
		}
	}
	return 0;
}
