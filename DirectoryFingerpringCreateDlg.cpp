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
																BOOL bSaveAsUnicode,
																CWnd* pParent /*=NULL*/)
	: BaseClass(IDD, pParent)
	, m_pFile(NULL)
	, m_bIncludeSubdirectories(bIncludeSubdirectories)
	, m_bIncludeDirectoryStructure(bIncludeDirectoryStructure)
	, m_bSaveAsUnicode(bSaveAsUnicode)
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
	_tfopen_s(&m_pFile, m_FingerprintFilename, _T("wt"));
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
	LPCTSTR crlf = _T("\n");
	if (m_bSaveAsUnicode)
	{
		crlf = _T("\r\n");
	}

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
			_T("; Fingerprint of \"%s\", created %s %s%s")
			_T("IncludeFiles=%s%s")
			_T("ExcludeFiles=%s%s")
			_T("ExcludeFolders=%s%s")
			_T("IncludeSubdirs=%d%s")
			_T("IncludeDirInfo=%d%s%s"),
			FullDirectoryName,
			date, time, crlf,
			LPCTSTR(m_sFilenameFilter), crlf,
			LPCTSTR(m_sIgnoreFiles), crlf,
			LPCTSTR(m_sIgnoreFolders), crlf,
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

		SetNextItem(pFile->GetFullName(), pFile->GetFileLength(), FILE_OPEN_OVERHEAD, true);

		if (pFile->CalculateHashes( & HashCalc, this))
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
