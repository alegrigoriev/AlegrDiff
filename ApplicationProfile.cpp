#include <stdafx.h>
#include "ApplicationProfile.h"
#include <float.h>
#include <afxpriv.h>

CApplicationProfileItem::CApplicationProfileItem(LPCTSTR szSection, LPCTSTR szName)
	: Section(szSection), Name(szName)
{
}

CApplicationProfileItemStr::CApplicationProfileItemStr(LPCTSTR szSection, LPCTSTR szName,
														CString& StringReference, LPCTSTR Default)
	: CApplicationProfileItem(szSection, szName), StrRef(StringReference), m_Default(Default)
{
	// read value
	ReadData();
	InitialData = StrRef;
}

void CApplicationProfileItemStr::ReadData()
{
	StrRef = AfxGetApp()->GetProfileString(Section, Name, m_Default);
}

void CApplicationProfileItemStr::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || StrRef != InitialData)
	{
		AfxGetApp()->WriteProfileString(Section, Name, StrRef);
	}
}

void CApplicationProfileItemStr::ResetToInitial()
{
	StrRef = InitialData;
}

void CApplicationProfileItemStr::ResetToDefault()
{
	StrRef = m_Default;
}

CApplicationProfileItemLong::CApplicationProfileItemLong(LPCTSTR szSection, LPCTSTR szName,
														LONG& LongReference,
														LONG Default, LONG MinVal, LONG MaxVal)
	: CApplicationProfileItem(szSection, szName),
	LongRef(LongReference), m_Default(Default), m_MinVal(MinVal), m_MaxVal(MaxVal)
{
	// read value
	ReadData();
	InitialData = LongRef;
}

void CApplicationProfileItemLong::ReadData()
{
	LongRef = AfxGetApp()->GetProfileInt(Section, Name, m_Default);
}

void CApplicationProfileItemLong::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || LongRef != InitialData)
	{
		AfxGetApp()->WriteProfileInt(Section, Name, LongRef);
	}
}

void CApplicationProfileItemLong::ResetToInitial()
{
	LongRef = InitialData;
}

void CApplicationProfileItemLong::ResetToDefault()
{
	LongRef = m_Default;
}

CApplicationProfileItemInt::CApplicationProfileItemInt(LPCTSTR szSection, LPCTSTR szName,
														int & Reference,
														int Default, int MinVal, int MaxVal)
	: CApplicationProfileItem(szSection, szName),
	Ref(Reference), m_Default(Default), m_MinVal(MinVal), m_MaxVal(MaxVal)
{
	// read value
	ReadData();
	InitialData = Ref;
}

CApplicationProfileItemBool::CApplicationProfileItemBool(LPCTSTR szSection, LPCTSTR szName,
														bool & RefValue,
														bool Default)
	: CApplicationProfileItemInt(szSection, szName, m_TempVal, Default, 0, 1),
	Ref(RefValue), m_bDefault(Default)
{
	// read value
	ReadData();
	InitialData = Ref;
}

void CApplicationProfileItemInt::ReadData()
{
	Ref = AfxGetApp()->GetProfileInt(Section, Name, m_Default);
}

void CApplicationProfileItemBool::ReadData()
{
	CApplicationProfileItemInt::ReadData();
	Ref = (m_TempVal != 0);
}

void CApplicationProfileItemInt::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || Ref != InitialData)
	{
		AfxGetApp()->WriteProfileInt(Section, Name, Ref);
	}
}

void CApplicationProfileItemBool::WriteData(BOOL bForceWrite)
{
	m_TempVal = Ref;
	CApplicationProfileItemInt::WriteData(bForceWrite);
}

void CApplicationProfileItemInt::ResetToInitial()
{
	Ref = InitialData;
}

void CApplicationProfileItemInt::ResetToDefault()
{
	Ref = m_Default;
}

void CApplicationProfileItemBool::ResetToInitial()
{
	Ref = InitialData;
}

void CApplicationProfileItemBool::ResetToDefault()
{
	Ref = m_bDefault;
}

CApplicationProfileItemUlong::CApplicationProfileItemUlong(LPCTSTR szSection, LPCTSTR szName,
															ULONG& LongReference,
															ULONG Default, ULONG MinVal, ULONG MaxVal)
	: CApplicationProfileItem(szSection, szName),
	LongRef(LongReference), m_Default(Default), m_MinVal(MinVal), m_MaxVal(MaxVal)
{
	// read value
	ReadData();
	InitialData = LongRef;
}

void CApplicationProfileItemUlong::ReadData()
{
	LongRef = AfxGetApp()->GetProfileInt(Section, Name, m_Default);
}

void CApplicationProfileItemUlong::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || LongRef != InitialData)
	{
		AfxGetApp()->WriteProfileInt(Section, Name, LongRef);
	}
}

void CApplicationProfileItemUlong::ResetToInitial()
{
	LongRef = InitialData;
}

void CApplicationProfileItemUlong::ResetToDefault()
{
	LongRef = m_Default;
}

CApplicationProfileItemDouble::CApplicationProfileItemDouble(LPCTSTR szSection, LPCTSTR szName,
															double& DoubleReference,
															double Default, double MinVal, double MaxVal)
	: CApplicationProfileItem(szSection, szName),
	DoubleRef(DoubleReference), m_Default(Default), m_MinVal(MinVal), m_MaxVal(MaxVal)
{
	// read value
	ReadData();
	InitialData = DoubleRef;
}

void CApplicationProfileItemDouble::ReadData()
{
	CString s = AfxGetApp()->GetProfileString(Section, Name, "");
	double val;
	TCHAR * endptr;

	if (s.IsEmpty()
		|| (val = _tcstod(s, & endptr), 0 != *endptr)
		|| _isnan(val)
		|| ! _finite(val))
	{
		val = m_Default;
	}

	if (m_MinVal != m_MaxVal)
	{
		if (val < m_MinVal) val = m_MinVal;
		if (val > m_MaxVal) val = m_MaxVal;
	}
	DoubleRef = val;
}

void CApplicationProfileItemDouble::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || DoubleRef != InitialData)
	{
		CString s;
		s.Format("%g", DoubleRef);
		AfxGetApp()->WriteProfileString(Section, Name, s);
	}
}

void CApplicationProfileItemDouble::ResetToInitial()
{
	DoubleRef = InitialData;
}

void CApplicationProfileItemDouble::ResetToDefault()
{
	DoubleRef = m_Default;
}

CApplicationProfileItemFloat::CApplicationProfileItemFloat(LPCTSTR szSection, LPCTSTR szName,
															float& FloatReference,
															double Default, double MinVal, double MaxVal)
	: CApplicationProfileItemDouble(szSection, szName, IntermediateValue,
									Default, MinVal, MaxVal),
	FloatRef(FloatReference)
{
	// read value
	ReadData();
	FloatReference = IntermediateValue;
}

void CApplicationProfileItemFloat::ReadData()
{
	CApplicationProfileItemDouble::ReadData();
	FloatRef = IntermediateValue;
}

void CApplicationProfileItemFloat::WriteData(BOOL bForceWrite)
{
	IntermediateValue = FloatRef;
	CApplicationProfileItemDouble::WriteData(bForceWrite);
}

void CApplicationProfileItemFloat::ResetToInitial()
{
	FloatRef = InitialData;
}

void CApplicationProfileItemFloat::ResetToDefault()
{
	CApplicationProfileItemDouble::ResetToDefault();
	FloatRef = IntermediateValue;
}

CApplicationProfile::CApplicationProfile()
	:pItems(NULL)
{
}

CApplicationProfile::~CApplicationProfile()
{
	CApplicationProfileItem * pTmp;
	while (pItems != NULL)
	{
		pTmp = pItems;
		pItems = pTmp->Next;
		delete pTmp;
	}
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, CString & str,
								LPCTSTR szDefault,int MaxLen)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemStr(szSection, szName, str, szDefault);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, LONG & val,
								LONG nDefault, LONG nMin, LONG nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemLong(szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, int & val,
								int nDefault, int nMin, int nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemInt(szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, ULONG & val,
								ULONG nDefault, ULONG nMin, ULONG nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemUlong(szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddBoolItem(LPCTSTR szSection, LPCTSTR szName, int & val,
									int nDefault)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemInt(szSection, szName, val, nDefault, 0, 1);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, bool & val,
								bool nDefault)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemBool(szSection, szName, val, nDefault);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, double & val,
								double nDefault, double nMin, double nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemDouble(szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, float & val,
								double nDefault, double nMin, double nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemFloat(szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

BOOL CApplicationProfile::RemoveItem(LPCTSTR szSection, LPCTSTR szName)
{
	CApplicationProfileItem * pTmp = pItems;
	CApplicationProfileItem * pPrev = NULL;
	while(pTmp != NULL)
	{
		if (0 == pTmp->Name.CompareNoCase(szName)
			&& 0 == pTmp->Section.CompareNoCase(szSection))
		{
			if (pPrev)
			{
				pPrev->Next = pTmp->Next;
			}
			else
			{
				pItems = pTmp->Next;
			}
			delete pTmp;
			return TRUE;
		}
		pPrev = pTmp;
		pTmp = pTmp->Next;
	}
	return FALSE;
}

BOOL CApplicationProfile::RemoveSection(LPCTSTR szSection)
{
	CApplicationProfileItem * pTmp = pItems;
	CApplicationProfileItem * pPrev = NULL;
	while(pTmp != NULL)
	{
		if (NULL == szSection
			|| 0 == pTmp->Section.CompareNoCase(szSection))
		{
			if (pPrev)
			{
				pPrev->Next = pTmp->Next;
				delete pTmp;
				pTmp = pPrev->Next;
			}
			else
			{
				pItems = pTmp->Next;
				delete pTmp;
				pTmp = pItems;
			}
			continue;
		}
		pPrev = pTmp;
		pTmp = pTmp->Next;
	}
	return TRUE;
}

BOOL CApplicationProfile::FlushItem(LPCTSTR szSection, LPCTSTR szName)
{
	CApplicationProfileItem * pTmp = pItems;
	while(pTmp != NULL)
	{
		if (0 == pTmp->Name.CompareNoCase(szName)
			&& 0 == pTmp->Section.CompareNoCase(szSection))
		{
			pTmp->WriteData();
			return TRUE;
		}
		pTmp = pTmp->Next;
	}
	return FALSE;
}

BOOL CApplicationProfile::FlushSection(LPCTSTR szSection)
{
	CApplicationProfileItem * pTmp = pItems;
	while(pTmp != NULL)
	{
		if (NULL == szSection
			|| 0 == pTmp->Section.CompareNoCase(szSection))
		{
			pTmp->WriteData();
		}
		pTmp = pTmp->Next;
	}
	return TRUE;
}

BOOL CApplicationProfile::UnloadItem(LPCTSTR szSection, LPCTSTR szName)
{
	return FlushItem(szSection, szName) && RemoveItem(szSection, szName);
}

BOOL CApplicationProfile::UnloadSection(LPCTSTR szSection)
{
	return FlushSection(szSection) && RemoveSection(szSection);
}

BOOL CApplicationProfile::ResetItemToDefault(LPCTSTR szSection, LPCTSTR szName)
{
	CApplicationProfileItem * pTmp = pItems;
	while(pTmp != NULL)
	{
		if (0 == pTmp->Name.CompareNoCase(szName)
			&& 0 == pTmp->Section.CompareNoCase(szSection))
		{
			pTmp->ResetToDefault();
			return TRUE;
		}
		pTmp = pTmp->Next;
	}
	return FALSE;
}

BOOL CApplicationProfile::ResetSectionToDefault(LPCTSTR szSection)
{
	CApplicationProfileItem * pTmp = pItems;
	while(pTmp != NULL)
	{
		if (0 == pTmp->Section.CompareNoCase(szSection))
		{
			pTmp->ResetToDefault();
		}
		pTmp = pTmp->Next;
	}
	return TRUE;
}

BOOL CApplicationProfile::RevertItemToInitial(LPCTSTR szSection, LPCTSTR szName)
{
	CApplicationProfileItem * pTmp = pItems;
	while(pTmp != NULL)
	{
		if (0 == pTmp->Name.CompareNoCase(szName)
			&& 0 == pTmp->Section.CompareNoCase(szSection))
		{
			pTmp->ResetToInitial();
			return TRUE;
		}
		pTmp = pTmp->Next;
	}
	return FALSE;
}

BOOL CApplicationProfile::RevertSectionToInitial(LPCTSTR szSection)
{
	CApplicationProfileItem * pTmp = pItems;
	while(pTmp != NULL)
	{
		if (0 == pTmp->Section.CompareNoCase(szSection))
		{
			pTmp->ResetToInitial();
		}
		pTmp = pTmp->Next;
	}
	return TRUE;
}
		// saves the section in INI file. If section name is empty,
		// saves all of them
BOOL CApplicationProfile::ExportSection(LPCTSTR szSection, LPCTSTR szFilename)
{
	CWinApp * pApp = AfxGetApp();
	if (NULL == szFilename || 0 == szFilename[0])
	{
		return FALSE;
	}
	LPCTSTR OldRegistryKeyName = pApp->m_pszRegistryKey;
	LPCTSTR OldProfileName = pApp->m_pszProfileName;
	pApp->m_pszRegistryKey = NULL;
	pApp->m_pszProfileName = szFilename;
	try
	{
		CApplicationProfileItem * pTmp = pItems;
		while(pTmp != NULL)
		{
			if (NULL == szSection
				|| 0 == szSection[0]
				|| 0 == pTmp->Section.CompareNoCase(szSection))
			{
				pTmp->WriteData(TRUE);  // force to write
			}
			pTmp = pTmp->Next;
		}
	}
	catch(...)
	{
		pApp->m_pszProfileName = OldProfileName;
		pApp->m_pszRegistryKey = OldRegistryKeyName;
		throw;
	}
	pApp->m_pszProfileName = OldProfileName;
	pApp->m_pszRegistryKey = OldRegistryKeyName;
	return TRUE;
}

// restores the section from INI file. If section name is empty,
// restores all of them.
BOOL CApplicationProfile::ImportSection(LPCTSTR szSection, LPCTSTR szFilename)
{
	CWinApp * pApp = AfxGetApp();
	if (NULL == szFilename || 0 == szFilename[0])
	{
		return FALSE;
	}
	LPCTSTR OldRegistryKeyName = pApp->m_pszRegistryKey;
	LPCTSTR OldProfileName = pApp->m_pszProfileName;
	pApp->m_pszRegistryKey = NULL;
	pApp->m_pszProfileName = szFilename;
	try
	{
		CApplicationProfileItem * pTmp = pItems;
		while(pTmp != NULL)
		{
			if (NULL == szSection
				|| 0 == szSection[0]
				|| 0 == pTmp->Section.CompareNoCase(szSection))
			{
				pTmp->ReadData();
			}
			pTmp = pTmp->Next;
		}
	}
	catch(...)
	{
		pApp->m_pszProfileName = OldProfileName;
		pApp->m_pszRegistryKey = OldRegistryKeyName;
		throw;
	}
	pApp->m_pszProfileName = OldProfileName;
	pApp->m_pszRegistryKey = OldRegistryKeyName;
	return TRUE;
}

// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

/////////////////////////////////////////////////////////////////////////////
// CWinApp Settings Helpers

// the original registry functions modified to get faster registry access.
HKEY hCachedRegistryKey = NULL;
HKEY hCachedSectionKey = NULL;
CString sCachedSectionName;

void CWinApp::SetRegistryKey(LPCTSTR lpszRegistryKey)
{
	ASSERT(m_pszRegistryKey == NULL);
	ASSERT(lpszRegistryKey != NULL);
	ASSERT(m_pszAppName != NULL);

	BOOL bEnable = AfxEnableMemoryTracking(FALSE);
	free((void*)m_pszRegistryKey);
	m_pszRegistryKey = _tcsdup(lpszRegistryKey);
	free((void*)m_pszProfileName);
	m_pszProfileName = _tcsdup(m_pszAppName);
	AfxEnableMemoryTracking(bEnable);

	// ++AG:
	if (NULL != hCachedRegistryKey)
	{
		RegCloseKey(hCachedRegistryKey);
		hCachedRegistryKey = NULL;
	}
	sCachedSectionName.Empty();
	if (NULL != hCachedSectionKey)
	{
		RegCloseKey(hCachedSectionKey);
		hCachedSectionKey = NULL;
	}
}

void CWinApp::SetRegistryKey(UINT nIDRegistryKey)
{
	ASSERT(m_pszRegistryKey == NULL);

	TCHAR szRegistryKey[256];
	VERIFY(AfxLoadString(nIDRegistryKey, szRegistryKey));
	SetRegistryKey(szRegistryKey);
}

// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
// creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CWinApp::GetAppRegistryKey()
{
	ASSERT(m_pszRegistryKey != NULL);
	ASSERT(m_pszProfileName != NULL);

	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	// +AG:
	if (NULL != hCachedRegistryKey)
	{
		return hCachedRegistryKey;
	}

	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("software"), 0, KEY_WRITE|KEY_READ,
					&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, m_pszRegistryKey, 0, REG_NONE,
							REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
							&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, m_pszProfileName, 0, REG_NONE,
							REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
							&hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);

	hCachedRegistryKey = hAppKey;
	return hAppKey;
}

// returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CWinApp::GetSectionKey(LPCTSTR lpszSection)
{
	ASSERT(lpszSection != NULL);
	//++AG:
	if (0 == sCachedSectionName.CompareNoCase(lpszSection)
		&& NULL != hCachedSectionKey)
	{
		return hCachedSectionKey;
	}
	if (NULL != hCachedSectionKey)
	{
		RegCloseKey(hCachedSectionKey);
		hCachedSectionKey = NULL;
	}

	HKEY hSectionKey = NULL;
	HKEY hAppKey = GetAppRegistryKey();
	if (hAppKey == NULL)
		return NULL;

	DWORD dw;
	RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
					REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
					&hSectionKey, &dw);
	//RegCloseKey(hAppKey);
	hCachedSectionKey = hSectionKey;
	sCachedSectionName = lpszSection;
	return hSectionKey;
}

UINT CWinApp::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry,
							int nDefault)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	if (m_pszRegistryKey != NULL) // use registry
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return nDefault;
		DWORD dwValue;
		DWORD dwType;
		DWORD dwCount = sizeof(DWORD);
		LONG lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
										(LPBYTE)&dwValue, &dwCount);
		//RegCloseKey(hSecKey);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_DWORD);
			ASSERT(dwCount == sizeof(dwValue));
			return (UINT)dwValue;
		}
		return nDefault;
	}
	else
	{
		ASSERT(m_pszProfileName != NULL);
		return ::GetPrivateProfileInt(lpszSection, lpszEntry, nDefault,
									m_pszProfileName);
	}
}

CString CWinApp::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
								LPCTSTR lpszDefault)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	if (m_pszRegistryKey != NULL)
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return lpszDefault;
		CString strValue;
		DWORD dwType, dwCount;
		LONG lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
										NULL, &dwCount);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
									(LPBYTE)strValue.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
			strValue.ReleaseBuffer();
		}
		//RegCloseKey(hSecKey);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			return strValue;
		}
		return lpszDefault;
	}
	else
	{
		ASSERT(m_pszProfileName != NULL);

		if (lpszDefault == NULL)
			lpszDefault = &afxChNil;    // don't pass in NULL
		TCHAR szT[4096];
		DWORD dw = ::GetPrivateProfileString(lpszSection, lpszEntry,
											lpszDefault, szT, 4096, m_pszProfileName);
		ASSERT(dw < 4095);
		return szT;
	}
}

BOOL CWinApp::GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
								BYTE** ppData, UINT* pBytes)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	ASSERT(ppData != NULL);
	ASSERT(pBytes != NULL);
	*ppData = NULL;
	*pBytes = 0;
	if (m_pszRegistryKey != NULL)
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return FALSE;

		DWORD dwType, dwCount;
		LONG lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
										NULL, &dwCount);
		*pBytes = dwCount;
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_BINARY);
			*ppData = new BYTE[*pBytes];
			lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
									*ppData, &dwCount);
		}
		//RegCloseKey(hSecKey);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_BINARY);
			return TRUE;
		}
		else
		{
			delete [] *ppData;
			*ppData = NULL;
		}
		return FALSE;
	}
	else
	{
		ASSERT(m_pszProfileName != NULL);

		CString str = GetProfileString(lpszSection, lpszEntry, NULL);
		if (str.IsEmpty())
			return FALSE;
		ASSERT(str.GetLength()%2 == 0);
		int nLen = str.GetLength();
		*pBytes = nLen/2;
		*ppData = new BYTE[*pBytes];
		for (int i=0;i<nLen;i+=2)
		{
			(*ppData)[i/2] = (BYTE)
							(((str[i+1] - 'A') << 4) + (str[i] - 'A'));
		}
		return TRUE;
	}
}

BOOL CWinApp::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry,
							int nValue)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	if (m_pszRegistryKey != NULL)
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return FALSE;
		LONG lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_DWORD,
									(LPBYTE)&nValue, sizeof(nValue));
		return lResult == ERROR_SUCCESS;
	}
	else
	{
		ASSERT(m_pszProfileName != NULL);

		TCHAR szT[16];
		wsprintf(szT, _T("%d"), nValue);
		return ::WritePrivateProfileString(lpszSection, lpszEntry, szT,
											m_pszProfileName);
	}
}

BOOL CWinApp::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
								LPCTSTR lpszValue)
{
	ASSERT(lpszSection != NULL);
	if (m_pszRegistryKey != NULL)
	{
		LONG lResult;
		if (lpszEntry == NULL) //delete whole section
		{
			HKEY hAppKey = GetAppRegistryKey();
			if (hAppKey == NULL)
				return FALSE;
			if (NULL != hCachedSectionKey)
			{
				RegCloseKey(hCachedSectionKey);
				hCachedSectionKey = NULL;
			}
			lResult = ::RegDeleteKey(hAppKey, lpszSection);
		}
		else if (lpszValue == NULL)
		{
			HKEY hSecKey = GetSectionKey(lpszSection);
			if (hSecKey == NULL)
				return FALSE;
			// necessary to cast away const below
			lResult = ::RegDeleteValue(hSecKey, (LPTSTR)lpszEntry);
		}
		else
		{
			HKEY hSecKey = GetSectionKey(lpszSection);
			if (hSecKey == NULL)
				return FALSE;
			lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_SZ,
									(LPBYTE)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR));
		}
		return lResult == ERROR_SUCCESS;
	}
	else
	{
		ASSERT(m_pszProfileName != NULL);
		ASSERT(lstrlen(m_pszProfileName) < 4095); // can't read in bigger
		return ::WritePrivateProfileString(lpszSection, lpszEntry, lpszValue,
											m_pszProfileName);
	}
}

BOOL CWinApp::WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
								LPBYTE pData, UINT nBytes)
{
	ASSERT(lpszSection != NULL);
	if (m_pszRegistryKey != NULL)
	{
		LONG lResult;
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return FALSE;
		lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_BINARY,
								pData, nBytes);
		//RegCloseKey(hSecKey);
		return lResult == ERROR_SUCCESS;
	}

	// convert to string and write out
	LPTSTR lpsz = new TCHAR[nBytes*2+1];
	for (UINT i = 0; i < nBytes; i++)
	{
		lpsz[i*2] = (TCHAR)((pData[i] & 0x0F) + 'A'); //low nibble
		lpsz[i*2+1] = (TCHAR)(((pData[i] >> 4) & 0x0F) + 'A'); //high nibble
	}
	lpsz[i*2] = 0;

	ASSERT(m_pszProfileName != NULL);

	BOOL bResult = WriteProfileString(lpszSection, lpszEntry, lpsz);
	delete[] lpsz;
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
