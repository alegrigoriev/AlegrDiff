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

CDirectoryFingerprintCheckDlg::CDirectoryFingerprintCheckDlg(
															CAlegrDiffDoc * pDoc,
															LPCTSTR DirectoryToCheck,
															LPCTSTR FingerprintFilename,
															CWnd* pParent /*=NULL*/)
	: BaseClass(IDD, pParent)
	, m_pDocument(pDoc)
	, m_pFile(NULL)
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

	if ((FirstChar & 0xFFFF) != 0xFEFF)
	{
		rewind(m_pFile);
		_setmode(_fileno(m_pFile), _O_TEXT);
	}

	return BaseClass::DoModal();
}

BOOL CDirectoryFingerprintCheckDlg::OnInitDialog()
{
	BaseClass::OnInitDialog();

	TRACE("CDirectoryFingerprintCheckDlg::OnInitDialog()\n");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

unsigned CDirectoryFingerprintCheckDlg::ThreadProc()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	// load the directory
	FileList FileList1;
	FileList FileList2;

	// make full names from the directories
	LPTSTR pFilePart;

	// load the fingerprint file
	TCHAR buf[1024];
	while(NULL != _fgetts(buf, 1023, m_pFile))
	{
		if (';' == buf[0])
		{
			continue;
		}
		if (0 == _tcsnicmp(buf, _T("IncludeFiles="), 13))
		{
			m_sFilenameFilter = buf + 13;
			m_sFilenameFilter.Trim();
		}
		else if (0 == _tcsnicmp(buf, _T("ExcludeFiles="), 13))
		{
			m_sIgnoreFiles = buf + 13;
			m_sIgnoreFiles.Trim();
		}
		else if (0 == _tcsnicmp(buf, _T("ExcludeFolders="), 15))
		{
			m_sIgnoreFolders = buf + 13;
			m_sIgnoreFolders.Trim();
		}
		else if (0 == _tcsnicmp(buf, _T("IncludeSubdirs="), 15))
		{
			TCHAR * Endptr;
			m_bIncludeSubdirectories = _tcstol(buf + 15, & Endptr, 10);
		}
		else if (0 == _tcsnicmp(buf, _T("IncludeDirInfo="), 15))
		{
			TCHAR * Endptr;
			m_bIncludeDirectoryStructure = _tcstol(buf + 15, & Endptr, 10);
		}
		else
		{
			break;
		}
	}

	CString ExclusionPattern(PatternToMultiCString(m_sIgnoreFiles));
	CString InclusionPattern(PatternToMultiCString(m_sFilenameFilter));
	CString IgnoreDirsPattern(PatternToMultiCString(m_sIgnoreFolders));

	TCHAR FileName[512];
	while(NULL != _fgetts(buf, 1023, m_pFile))
	{
		if (';' == buf[0])
		{
			continue;
		}
		buf[1023] = 0;
		FileName[511] = 0;

		LONGLONG FileLength;

		WIN32_FIND_DATA wfd;
		memzero(wfd);
		ULONG MD5[16];
		BYTE md5[16];

		int NumScannedItems = _stscanf(buf,
										_T("\"%511[^\"]\" %I64u %I64x ")
										_T("%2x%2x%2x%2x")
										_T("%2x%2x%2x%2x")
										_T("%2x%2x%2x%2x")
										_T("%2x%2x%2x%2x")
										_T("\n")
										,
										FileName, & FileLength, & wfd.ftLastWriteTime,

										& MD5[0], & MD5[1], & MD5[2], & MD5[3],
										& MD5[4], & MD5[5], & MD5[6], & MD5[7],
										& MD5[8], & MD5[9], & MD5[10], & MD5[11],
										& MD5[12], & MD5[13], & MD5[14], & MD5[15]);

		if (NumScannedItems < 1)
		{
			// error
			continue;
		}

		for (int i = 0; i < 16; i++)
		{
			md5[i] = BYTE(MD5[i]);
		}

		// find the last '\'
		LPTSTR DirEnd = _tcsrchr(FileName, '\\');
		LPTSTR NamePart = FileName;
		CString SubDir;
		FileItem * pFile;

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
					NamePart = DirEnd + 1;
					break;
				}
			}
		}
		else
		{
			if (NumScannedItems != 19)
			{
				// error
				continue;
			}
			if (NULL != DirEnd)
			{
				NamePart = DirEnd + 1;
			}
			else
			{
				DirEnd = FileName;
			}

			wfd.nFileSizeLow = DWORD(FileLength);
			wfd.nFileSizeHigh = DWORD(FileLength >> 32);
		}

		if (DirEnd != FileName)
		{
			SubDir = CString(FileName, DirEnd - FileName + 1);
		}

		if (NamePart[0] != 0
			&& NamePart[0] != '\\'
			&& (SubDir.IsEmpty()
				|| SubDir[0] != '\\'))
		{
			_tcsncpy(wfd.cFileName, NamePart, countof(wfd.cFileName) - 1);
			wfd.cFileName[countof(wfd.cFileName) - 1] = 0;

			pFile = new FileItem(& wfd, CString(), SubDir, NULL);   // TODO: Parent dir

			pFile->SetMD5(md5);

			pFile->m_pNext = FileList1.m_pList;
			FileList1.m_pList = pFile;
			FileList1.m_NumFiles++;
		}
	}

	GetFullPathName(m_sDirectory, MAX_PATH, buf, & pFilePart);

	if (InclusionPattern.IsEmpty())
	{
		InclusionPattern = '*';
	}

	fclose(m_pFile);
	m_pFile = NULL;

	m_pDocument->m_bRecurseSubdirs = (m_bIncludeSubdirectories != 0);

	CString s;
	s.Format(IDS_STRING_LOADING_DIRECTORY, buf);
	SetNextItem(s, 0, 0);

	if (! FileList2.LoadFolder(buf, m_bIncludeSubdirectories != 0,
								InclusionPattern, ExclusionPattern, PatternToMultiCString(_T("")),
								PatternToMultiCString(_T("*")), IgnoreDirsPattern))
	{
		s.Format(IDS_STRING_DIRECTORY_LOAD_ERROR, buf);

		SetNextItem(s, 0, 0);

		SignalDialogEnd(IDABORT);
		return 0;
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

				AddDoneItem(pPair->pSecondFile->GetFileLength());
			}
		}

	}

	SignalDialogEnd(IDOK);
	return 0;
}
