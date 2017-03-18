// DirectoryFingerpringCreateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DirectoryFingerpringCreateDlg.h"
#include <io.h>
#include <fcntl.h>
#include "MessageBoxSynch.h"

// CDirectoryFingerpringCreateDlg dialog

CDirectoryFingerpringCreateDlg::CDirectoryFingerpringCreateDlg(
																LPCTSTR sDirectory,
																LPCTSTR sFingerprintFilename,
																LPCTSTR sFilenameFilter,
																LPCTSTR sIgnoreFiles,
																LPCTSTR sIgnoreFolders,
																BOOL bIncludeSubdirectories,
																BOOL bIncludeDirectoryStructure,
																CWnd* pParent /*=NULL*/)
	: BaseClass(IDD, pParent)
	, m_pFile(NULL)
	, m_bIncludeSubdirectories(bIncludeSubdirectories)
	, m_bIncludeDirectoryStructure(bIncludeDirectoryStructure)
	, m_sDirectory(sDirectory)
	, m_FingerprintFilename(sFingerprintFilename)
	, m_sFilenameFilter(sFilenameFilter)
	, m_sIgnoreFiles(sIgnoreFiles)
	, m_sIgnoreFolders(sIgnoreFolders)
{
}

CDirectoryFingerpringCreateDlg::~CDirectoryFingerpringCreateDlg()
{
}

void CDirectoryFingerpringCreateDlg::DoDataExchange(CDataExchange* pDX)
{
	BaseClass::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDirectoryFingerpringCreateDlg, BaseClass)
END_MESSAGE_MAP()


// CDirectoryFingerpringCreateDlg message handlers

INT_PTR CDirectoryFingerpringCreateDlg::DoModal()
{
	m_pFile = NULL;
	_tfopen_s(&m_pFile, m_FingerprintFilename, _T("wt,ccs=UTF-8"));
	if (NULL == m_pFile)
	{
		CString s;
		s.Format(IDS_STRING_UNABLE_TO_CREATE_FILE, LPCTSTR(m_FingerprintFilename));
		AfxMessageBox(s, MB_OK | MB_ICONSTOP);
		return -1;
	}

	return CProgressDialog::DoModal();
}

unsigned CDirectoryFingerpringCreateDlg::ThreadProc()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	// load the directory
	FileList FileList1;
	unsigned i;

	// make full names from the directories
	LPTSTR pFilePart;
	TCHAR FullDirectoryName[MAX_PATH];

	GetFullPathName(m_sDirectory, MAX_PATH, FullDirectoryName, & pFilePart);

	CString ExclusionPattern(PatternToMultiCString(m_sIgnoreFiles));
	CString InclusionPattern(PatternToMultiCString(m_sFilenameFilter));
	CString IgnoreDirsPattern(PatternToMultiCString(m_sIgnoreFolders));

	if (InclusionPattern.IsEmpty())
	{
		m_sFilenameFilter = '*';
		InclusionPattern = PatternToMultiCString(_T("*"));
	}

	CString s;
	s.Format(IDS_STRING_LOADING_DIRECTORY, FullDirectoryName);
	SetNextItem(s, 0, 0);

	if (! FileList1.LoadFolder(FullDirectoryName, m_bIncludeSubdirectories != 0,
								InclusionPattern, ExclusionPattern, PatternToMultiCString(_T("")),
								PatternToMultiCString(_T("")), IgnoreDirsPattern))
	{
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, FullDirectoryName);

		MessageBoxSync(s);

		SignalDialogEnd(IDOK);
		return 0;
	}

	vector<FileItem *> Files1;

	FileList1.GetSortedList(Files1, FileList::SortDirFirst);

	ULONGLONG TotalDataSize = 0;
	for (i = 0; i < Files1.size(); i++)
	{
		FileItem * pFile = Files1[i];
		if ( ! pFile->IsFolder())
		{
			// a file open is taken as equivalent of 8K
			TotalDataSize += pFile->GetFileLength() + FILE_OPEN_OVERHEAD;
		}
	}
	SetTotalDataSize(TotalDataSize);

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
			_T("; Fingerprint of \"%s\", created %s %s\n")
			_T("IncludeFiles=%s\n")
			_T("ExcludeFiles=%s\n")
			_T("ExcludeFolders=%s\n")
			_T("IncludeSubdirs=%d\n")
			_T("IncludeDirInfo=%d\n\n"),
			FullDirectoryName,
			date, time,
			LPCTSTR(m_sFilenameFilter),
			LPCTSTR(m_sIgnoreFiles),
			LPCTSTR(m_sIgnoreFolders),
			m_bIncludeSubdirectories,
			m_bIncludeDirectoryStructure);


	for (i = 0; i < Files1.size() && ! m_StopRunThread; i++)
	{
		FileItem * pFile = Files1[i];
		if (pFile->IsFolder())
		{
			if ( ! m_bIncludeDirectoryStructure)
			{
				continue;
			}
			_ftprintf(m_pFile, _T("\"%s%s\\\"\n"), pFile->GetSubdir(), pFile->GetName());
			continue;
		}

		SetNextItem(pFile->GetFullName(), pFile->GetFileLength(), FILE_OPEN_OVERHEAD, true);

		if (pFile->CalculateHashes( & HashCalc, this))
		{
			FILETIME LastWriteTime = pFile->GetLastWriteTime();
			LARGE_INTEGER LastWriteTime64;
			LastWriteTime64.LowPart = LastWriteTime.dwLowDateTime;
			LastWriteTime64.HighPart = LastWriteTime.dwHighDateTime;

			_ftprintf(m_pFile,
					_T("\"%s%s\" %I64d %016I64X ")
					_T("%02X%02X%02X%02X")
					_T("%02X%02X%02X%02X")
					_T("%02X%02X%02X%02X")
					_T("%02X%02X%02X%02X")
					_T("\n"),
					pFile->GetSubdir(),
					pFile->GetName(),
					pFile->GetFileLength(),
					LastWriteTime64.QuadPart,
					pFile->GetDigest(0), pFile->GetDigest(1), pFile->GetDigest(2), pFile->GetDigest(3),
					pFile->GetDigest(4), pFile->GetDigest(5), pFile->GetDigest(6), pFile->GetDigest(7),
					pFile->GetDigest(8), pFile->GetDigest(9), pFile->GetDigest(10), pFile->GetDigest(11),
					pFile->GetDigest(12), pFile->GetDigest(13), pFile->GetDigest(14), pFile->GetDigest(15));
		}

		AddDoneItem(pFile->GetFileLength());
	}

	fclose(m_pFile);

	m_pFile = NULL;
	if ( ! m_StopRunThread)
	{
		s.Format(IDS_STRING_FINGERPRINT_CREATED, LPCTSTR(m_sDirectory), LPCTSTR(m_FingerprintFilename));
		MessageBoxSync(s);

		SignalDialogEnd(IDOK);
	}
	return 0;
}

void CDirectoryFingerpringCreateDlg::OnCancel()
{
	if (IDYES == AfxMessageBox(IDS_CAN_CANCEL_FINGERPRINT_PROMPT, MB_YESNO))
	{
		EndDialog(IDCANCEL);
	}
}
