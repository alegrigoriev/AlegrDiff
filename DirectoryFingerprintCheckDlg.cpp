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
	, m_bIncludeSubdirectories(false)
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

unsigned CDirectoryFingerprintCheckDlg::ThreadProc()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	// load the directory

	m_pDocument->m_sSecondDir = m_sDirectory;

	m_pDocument->m_bRecurseSubdirs = (m_bIncludeSubdirectories != 0);

	if (!m_pDocument->BuildFilePairList(m_FingerprintFilename, m_sDirectory))
	{
		SetNextItem(__T(""), 0, 0);

		SignalDialogEnd(IDABORT);
		return IDABORT;
	}

	m_TotalDataSize = 0;

	// calculate total size to read
	for (FilePair * pPair = m_pDocument->GetFirstFilePair(); m_pDocument->FilePairNotEnd(pPair); pPair = m_pDocument->GetNextFilePair(pPair))
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

	for (FilePair * pPair = m_pDocument->GetFirstFilePair(); m_pDocument->FilePairNotEnd(pPair); pPair = m_pDocument->GetNextFilePair(pPair))
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
