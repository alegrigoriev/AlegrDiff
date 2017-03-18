// DirectoryFingerprintCheckDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlegrDiff.h"
#include "DirectoryFingerprintCheckDlg.h"
#include "AlegrDiffDoc.h"
#include <io.h>
#include <fcntl.h>
#include <afxpriv.h>
#include "MessageBoxSynch.h"

// CDirectoryFingerprintCheckDlg dialog

CDirectoryFingerprintCheckDlg::CDirectoryFingerprintCheckDlg(
															CAlegrDiffDoc * pDoc,
															LPCTSTR DirectoryToCheck,
															LPCTSTR FingerprintFilename,
															CWnd* pParent /*=NULL*/)
	: BaseClass(IDD, pParent)
	, m_pDocument(pDoc)
	, m_bIncludeSubdirectories(FALSE)
	, m_bIncludeDirectoryStructure(FALSE)
	, m_sDirectory(DirectoryToCheck)
	, m_FingerprintFilename(FingerprintFilename)
{
}

CDirectoryFingerprintCheckDlg::~CDirectoryFingerprintCheckDlg()
{
}

void CDirectoryFingerprintCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	BaseClass::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDirectoryFingerprintCheckDlg, BaseClass)
END_MESSAGE_MAP()


// CDirectoryFingerprintCheckDlg message handlers

BOOL CDirectoryFingerprintCheckDlg::OnInitDialog()
{
	BaseClass::OnInitDialog();

	TRACE("CDirectoryFingerprintCheckDlg::OnInitDialog()\n");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

static const TCHAR sIncludeFiles[] = _T("IncludeFiles=");
static const TCHAR sExcludeFiles[] = _T("ExcludeFiles=");
static const TCHAR sExcludeFolders[] = _T("ExcludeFolders=");
static const TCHAR sIncludeSubdirs[] = _T("IncludeSubdirs=");
static const TCHAR sIncludeDirInfo[] = _T("IncludeDirInfo=");

BOOL CDirectoryFingerprintCheckDlg::LoadFingerprintFile(LPCTSTR Filename, FileList & Files)
{
	FILE * pFile = NULL;
	_tfopen_s(&pFile, Filename, _T("rt,ccs=UNICODE"));
	if (NULL == pFile)
	{
		CString s;
		s.Format(IDS_STRING_CANT_OPEN_FILE, Filename);
		MessageBoxSync(s, MB_OK | MB_ICONSTOP);
		SignalDialogEnd(IDABORT);
		return FALSE;
	}

	// load the fingerprint file
	TCHAR buf[1024];
	while (NULL != _fgetts(buf, 1023, pFile))
	{
		if (';' == buf[0])
		{
			continue;
		}
		if (0 == _tcsnicmp(buf, sIncludeFiles, countof(sIncludeFiles) - 1))
		{
			m_sFilenameFilter = buf + countof(sIncludeFiles) - 1;
			m_sFilenameFilter.Trim();
		}
		else if (0 == _tcsnicmp(buf, sExcludeFiles, countof(sExcludeFiles) - 1))
		{
			m_sIgnoreFiles = buf + countof(sExcludeFiles) - 1;
			m_sIgnoreFiles.Trim();
		}
		else if (0 == _tcsnicmp(buf, sExcludeFolders, countof(sExcludeFolders) - 1))
		{
			m_sIgnoreFolders = buf + countof(sExcludeFolders) - 1;
			m_sIgnoreFolders.Trim();
		}
		else if (0 == _tcsnicmp(buf, sIncludeSubdirs, countof(sIncludeSubdirs) - 1))
		{
			TCHAR * Endptr;
			m_bIncludeSubdirectories = (0 != _tcstol(buf + countof(sIncludeSubdirs) - 1, &Endptr, 10));
		}
		else if (0 == _tcsnicmp(buf, sIncludeDirInfo, countof(sIncludeDirInfo) - 1))
		{
			TCHAR * Endptr;
			m_bIncludeDirectoryStructure = (0 != _tcstol(buf + countof(sIncludeDirInfo) - 1, &Endptr, 10));
		}
		else
		{
			break;
		}
	}

	TCHAR FileName[4096];
	while (NULL != _fgetts(buf, 1023, pFile))
	{
		if (';' == buf[0])
		{
			continue;
		}
		buf[1023] = 0;
		FileName[511] = 0;

		LONGLONG FileLength;

		WIN32_FIND_DATA wfd = { 0 };
		BYTE md5[16];

		ULARGE_INTEGER uli;

		int NumScannedItems = _stscanf_s(buf,
										_T("\"%[^\"]\" %I64u %I64x ")
										_T("%2hhx%2hhx%2hhx%2hhx")
										_T("%2hhx%2hhx%2hhx%2hhx")
										_T("%2hhx%2hhx%2hhx%2hhx")
										_T("%2hhx%2hhx%2hhx%2hhx")
										_T("\n")
										,
										FileName, (unsigned)_countof(FileName), &FileLength, &uli.QuadPart,

										&md5[0], &md5[1], &md5[2], &md5[3],
										&md5[4], &md5[5], &md5[6], &md5[7],
										&md5[8], &md5[9], &md5[10], &md5[11],
										&md5[12], &md5[13], &md5[14], &md5[15]);

		if (NumScannedItems < 1)
		{
			// error
			continue;
		}

		wfd.ftLastWriteTime.dwHighDateTime = uli.HighPart;
		wfd.ftLastWriteTime.dwLowDateTime = uli.LowPart;

		// find the last '\'
		LPTSTR DirEnd = _tcsrchr(FileName, '\\');
		// NamePart is the last component of the path
		LPTSTR NamePart = FileName;
		CString SubDir;

		if (NULL != DirEnd && 0 == DirEnd[1])
		{
			// last char is backslash
			if (NumScannedItems != 1)
			{
				continue;
			}

			wfd.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			// find another backslash
			while (DirEnd != FileName)
			{
				DirEnd--;
				if ('\\' == *DirEnd)
				{
					// NamePart is the last directory component of the path
					NamePart = DirEnd + 1;
					break;
				}
			}
		}
		else
		{
			if (NumScannedItems != 19)
			{
				// error, skip this line
				continue;
			}
			if (NULL != DirEnd)
			{
				// NamePart is the filename component of the path
				NamePart = DirEnd + 1;
			}
			else
			{
				// there was no subdirectory component
				DirEnd = FileName;
			}

			wfd.nFileSizeLow = DWORD(FileLength);
			wfd.nFileSizeHigh = DWORD(FileLength >> 32);
		}

		if (DirEnd != FileName)
		{
			SubDir = CString(FileName, (int)(DirEnd - FileName + 1));
		}

		if (NamePart[0] != 0
			&& NamePart[0] != '\\'
			&& (SubDir.IsEmpty()
				|| SubDir[0] != '\\'))
		{
			_tcsncpy_s(wfd.cFileName, countof(wfd.cFileName), NamePart, countof(wfd.cFileName) - 1);
			wfd.cFileName[countof(wfd.cFileName) - 1] = 0;

			FileItem * pFileItem;
			pFileItem = new FileItem(&wfd, CString(), SubDir, NULL);   // FIXME: Base directory and Parent dir

			pFileItem->SetMD5(md5);

			pFileItem->m_pNext = Files.m_pList;
			Files.m_pList = pFileItem;
			Files.m_NumFiles++;
		}
	}
	fclose(pFile);

	return TRUE;
}

unsigned CDirectoryFingerprintCheckDlg::ThreadProc()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	// load the directory
	TCHAR buf[1024];
	FileList FileList1;
	FileList FileList2;

	if (!LoadFingerprintFile(m_FingerprintFilename, FileList1))
	{
		return 1;
	}

	// make full names from the directories
	LPTSTR pFilePart;

	CString ExclusionPattern(PatternToMultiCString(m_sIgnoreFiles));
	CString InclusionPattern(PatternToMultiCString(m_sFilenameFilter));
	CString IgnoreDirsPattern(PatternToMultiCString(m_sIgnoreFolders));

	GetFullPathName(m_sDirectory, MAX_PATH, buf, & pFilePart);

	if (InclusionPattern.IsEmpty())
	{
		InclusionPattern = '*';
	}

	m_pDocument->m_bRecurseSubdirs = (m_bIncludeSubdirectories != 0);

	CString s;
	s.Format(IDS_STRING_LOADING_DIRECTORY, buf);
	SetNextItem(s, 0, 0);

	if (! FileList2.LoadFolder(buf, m_bIncludeSubdirectories != 0,
								InclusionPattern, ExclusionPattern, PatternToMultiCString(_T("")),
								PatternToMultiCString(_T("*")), IgnoreDirsPattern))
	{
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, buf);
		MessageBoxSync(s, MB_OK | MB_ICONSTOP);

		SetNextItem(__T(""), 0, 0);

		SignalDialogEnd(IDABORT);
		return 1;
	}

	m_pDocument->BuildFilePairList(FileList1, FileList2, this);

	m_TotalDataSize = 0;

	// calculate total size to read
	FilePair * pPair;
	ListHead<FilePair> * pList;

	for (pList = m_pDocument->GetFilePairList(), pPair = pList->First();
		pList->NotEnd(pPair); pPair = pPair->Next())
	{
		if (pPair->pFirstFile != NULL
			&& NULL != pPair->pSecondFile
			&& ! pPair->pFirstFile->IsFolder()
			&& ! pPair->pSecondFile->IsFolder()
			)
		{
			ULONGLONG Length1 = pPair->pFirstFile->GetFileLength();
			ULONGLONG Length2 = pPair->pSecondFile->GetFileLength();
			if (Length1 < Length2)
			{
				pPair->SetComparisonResult(FilePair::SecondFileLonger);
			}
			else if (Length1 > Length2)
			{
				pPair->SetComparisonResult(FilePair::FirstFileLonger);
			}
			else
			{
				m_TotalDataSize += FILE_OPEN_OVERHEAD + pPair->pFirstFile->GetFileLength();
			}
		}
	}

	// read files, verify MD5
	CMd5HashCalculator HashCalc;

	for (pList = m_pDocument->GetFilePairList(), pPair = pList->First();
		pList->NotEnd(pPair); pPair = pPair->Next())
	{
		if (pPair->pFirstFile != NULL
			&& NULL != pPair->pSecondFile
			&& ! pPair->pFirstFile->IsFolder()
			&& ! pPair->pSecondFile->IsFolder())
		{
			if (pPair->pFirstFile->GetFileLength() == pPair->pSecondFile->GetFileLength())
			{
				SetNextItem(pPair->pSecondFile->GetFullName(), pPair->pSecondFile->GetFileLength(), FILE_OPEN_OVERHEAD, true);

				if (pPair->pSecondFile->CalculateHashes( & HashCalc, this))
				{
					if (0 == memcmp(pPair->pFirstFile->GetDigest(),
									pPair->pSecondFile->GetDigest(), pPair->pFirstFile->GetDigestLength()))
					{
						pPair->SetComparisonResult(FilePair::FilesIdentical);
					}
					else
					{
						pPair->SetComparisonResult(FilePair::FilesDifferent);
					}
				}
				else
				{
					if (GetLastError() == ERROR_ACCESS_DENIED)
					{
						pPair->SetComparisonResult(FilePair::FileUnaccessible);
					}
					else if (GetLastError() != ERROR_CANCELLED)
					{
						pPair->SetComparisonResult(FilePair::ErrorReadingSecondFile);
					}
					else
					{
						break;
					}
				}

				AddDoneItem(pPair->pSecondFile->GetFileLength());
			}
		}

	}

	SignalDialogEnd(IDOK);
	return 0;
}
