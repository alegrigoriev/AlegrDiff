#include <stdafx.h>
#include "ApplicationProfile.h"
#include <float.h>
#include <afxpriv.h>

CApplicationProfileItem::CApplicationProfileItem(CApplicationProfile * pProfile,
												LPCTSTR szSection, LPCTSTR szName)
	: Section(szSection), Name(szName), m_pProfile(pProfile)
{
}

CApplicationProfileItemStr::CApplicationProfileItemStr(CApplicationProfile * pProfile,
														LPCTSTR szSection, LPCTSTR szName,
														CString& StringReference, LPCTSTR Default)
	: CApplicationProfileItem(pProfile, szSection, szName), StrRef(StringReference), m_Default(Default)
{
	// read value
	ReadData();
	InitialData = StrRef;
}

void CApplicationProfileItemStr::ReadData()
{
	StrRef = m_pProfile->GetProfileString(Section, Name, m_Default);
}

void CApplicationProfileItemStr::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || StrRef != InitialData)
	{
		m_pProfile->WriteProfileString(Section, Name, StrRef);
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

CApplicationProfileItemLong::CApplicationProfileItemLong(CApplicationProfile * pProfile,
														LPCTSTR szSection, LPCTSTR szName,
														LONG& LongReference,
														LONG Default, LONG MinVal, LONG MaxVal)
	: CApplicationProfileItem(pProfile, szSection, szName),
	LongRef(LongReference), m_Default(Default), m_MinVal(MinVal), m_MaxVal(MaxVal)
{
	// read value
	ReadData();
	InitialData = LongRef;
}

void CApplicationProfileItemLong::ReadData()
{
	LongRef = m_pProfile->GetProfileInt(Section, Name, m_Default);
}

void CApplicationProfileItemLong::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || LongRef != InitialData)
	{
		m_pProfile->WriteProfileInt(Section, Name, LongRef);
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

CApplicationProfileItemInt::CApplicationProfileItemInt(CApplicationProfile * pProfile,
														LPCTSTR szSection, LPCTSTR szName,
														int & Reference,
														int Default, int MinVal, int MaxVal)
	: CApplicationProfileItem(pProfile, szSection, szName),
	Ref(Reference), m_Default(Default), m_MinVal(MinVal), m_MaxVal(MaxVal)
{
	// read value
	ReadData();
	InitialData = Ref;
}

void CApplicationProfileItemInt::ReadData()
{
	Ref = m_pProfile->GetProfileInt(Section, Name, m_Default);
}

void CApplicationProfileItemInt::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || Ref != InitialData)
	{
		m_pProfile->WriteProfileInt(Section, Name, Ref);
	}
}

void CApplicationProfileItemInt::ResetToInitial()
{
	Ref = InitialData;
}

void CApplicationProfileItemInt::ResetToDefault()
{
	Ref = m_Default;
}

CApplicationProfileItemShort::CApplicationProfileItemShort(
															CApplicationProfile * pProfile,
															LPCTSTR szSection, LPCTSTR szName,
															short & RefValue, short Default, short MinVal, short MaxVal)
	: CApplicationProfileItemInt(pProfile, szSection, szName, m_TempInt, Default, MinVal, MaxVal),
	ShortRef(RefValue)
{
	// data is read in parent class constructor
	ShortRef = m_TempInt;
}

void CApplicationProfileItemShort::ReadData()
{
	CApplicationProfileItemInt::ReadData();
	ShortRef = m_TempInt;
}

void CApplicationProfileItemShort::WriteData(BOOL bForceWrite)
{
	m_TempInt = ShortRef;
	CApplicationProfileItemInt::WriteData(bForceWrite);
}

void CApplicationProfileItemShort::ResetToInitial()
{
	ShortRef = InitialData;
}

void CApplicationProfileItemShort::ResetToDefault()
{
	ShortRef = m_Default;
}

CApplicationProfileItemUint::CApplicationProfileItemUint(CApplicationProfile * pProfile,
														LPCTSTR szSection, LPCTSTR szName,
														unsigned int & Reference,
														unsigned int Default, unsigned int MinVal, unsigned int MaxVal)
	: CApplicationProfileItem(pProfile, szSection, szName),
	Ref(Reference), m_Default(Default), m_MinVal(MinVal), m_MaxVal(MaxVal)
{
	// read value
	ReadData();
	InitialData = Ref;
}

void CApplicationProfileItemUint::ReadData()
{
	Ref = m_pProfile->GetProfileInt(Section, Name, m_Default);
}

void CApplicationProfileItemUint::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || Ref != InitialData)
	{
		m_pProfile->WriteProfileInt(Section, Name, Ref);
	}
}

void CApplicationProfileItemUint::ResetToInitial()
{
	Ref = InitialData;
}

void CApplicationProfileItemUint::ResetToDefault()
{
	Ref = m_Default;
}

CApplicationProfileItemUshort::CApplicationProfileItemUshort(
															CApplicationProfile * pProfile,
															LPCTSTR szSection, LPCTSTR szName,
															unsigned short & RefValue, unsigned short Default,
															unsigned short MinVal, unsigned short MaxVal)
	: CApplicationProfileItemUint(pProfile, szSection, szName, m_TempInt, Default, MinVal, MaxVal),
	ShortRef(RefValue)
{
	// data is read in parent class constructor
	ShortRef = m_TempInt;
}

void CApplicationProfileItemUshort::ReadData()
{
	CApplicationProfileItemUint::ReadData();
	ShortRef = m_TempInt;
}

void CApplicationProfileItemUshort::WriteData(BOOL bForceWrite)
{
	m_TempInt = ShortRef;
	CApplicationProfileItemUint::WriteData(bForceWrite);
}

void CApplicationProfileItemUshort::ResetToInitial()
{
	ShortRef = InitialData;
}

void CApplicationProfileItemUshort::ResetToDefault()
{
	ShortRef = m_Default;
}

CApplicationProfileItemBool::CApplicationProfileItemBool(
														CApplicationProfile * pProfile,
														LPCTSTR szSection, LPCTSTR szName,
														bool & RefValue,
														bool Default)
	: CApplicationProfileItemInt(pProfile, szSection, szName, m_TempVal, Default, 0, 1),
	Ref(RefValue), m_bDefault(Default)
{
	// read value
	ReadData();
	InitialData = Ref;
}

void CApplicationProfileItemBool::ReadData()
{
	CApplicationProfileItemInt::ReadData();
	Ref = (m_TempVal != 0);
}

void CApplicationProfileItemBool::WriteData(BOOL bForceWrite)
{
	m_TempVal = Ref;
	CApplicationProfileItemInt::WriteData(bForceWrite);
}

void CApplicationProfileItemBool::ResetToInitial()
{
	Ref = InitialData;
}

void CApplicationProfileItemBool::ResetToDefault()
{
	Ref = m_bDefault;
}

CApplicationProfileItemUlong::CApplicationProfileItemUlong(CApplicationProfile * pProfile,
															LPCTSTR szSection, LPCTSTR szName,
															ULONG& LongReference,
															ULONG Default, ULONG MinVal, ULONG MaxVal)
	: CApplicationProfileItem(pProfile, szSection, szName),
	LongRef(LongReference), m_Default(Default), m_MinVal(MinVal), m_MaxVal(MaxVal)
{
	// read value
	ReadData();
	InitialData = LongRef;
}

void CApplicationProfileItemUlong::ReadData()
{
	LongRef = m_pProfile->GetProfileInt(Section, Name, m_Default);
}

void CApplicationProfileItemUlong::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || LongRef != InitialData)
	{
		m_pProfile->WriteProfileInt(Section, Name, LongRef);
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

CApplicationProfileItemDouble::CApplicationProfileItemDouble(CApplicationProfile * pProfile,
															LPCTSTR szSection, LPCTSTR szName,
															double& DoubleReference,
															double Default, double MinVal, double MaxVal)
	: CApplicationProfileItem(pProfile, szSection, szName),
	DoubleRef(DoubleReference), m_Default(Default), m_MinVal(MinVal), m_MaxVal(MaxVal)
{
	// read value
	ReadData();
	InitialData = DoubleRef;
}

void CApplicationProfileItemDouble::ReadData()
{
	CString s = m_pProfile->GetProfileString(Section, Name, "");
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
		m_pProfile->WriteProfileString(Section, Name, s);
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

CApplicationProfileItemFloat::CApplicationProfileItemFloat(CApplicationProfile * pProfile,
															LPCTSTR szSection, LPCTSTR szName,
															float& FloatReference,
															double Default, double MinVal, double MaxVal)
	: CApplicationProfileItemDouble(pProfile, szSection, szName, IntermediateValue,
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
	:pItems(NULL),
	hCachedRegistryKey(NULL),
	hCachedSectionKey(NULL)
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
	CloseCachedKeys();
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, CString & str,
								LPCTSTR szDefault,int MaxLen)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemStr(this, szSection, szName, str, szDefault);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, LONG & val,
								LONG nDefault, LONG nMin, LONG nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemLong(this, szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, ULONG & val,
								ULONG nDefault, ULONG nMin, ULONG nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemUlong(this, szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, int & val,
								int nDefault, int nMin, int nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemInt(this, szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, unsigned int & val,
								unsigned int nDefault, unsigned int nMin, unsigned int nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemUint(this, szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, short & val,
								short nDefault, short nMin, short nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemShort(this, szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, unsigned short & val,
								unsigned short nDefault, unsigned short nMin, unsigned short nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemUshort(this, szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddBoolItem(LPCTSTR szSection, LPCTSTR szName, int & val,
									int nDefault)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemInt(this, szSection, szName, val, nDefault, 0, 1);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, bool & val,
								bool nDefault)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemBool(this, szSection, szName, val, nDefault);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, double & val,
								double nDefault, double nMin, double nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemDouble(this, szSection, szName, val, nDefault, nMin, nMax);
	pTmp->Next = pItems;
	pItems = pTmp;
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, float & val,
								double nDefault, double nMin, double nMax)
{
	CApplicationProfileItem * pTmp;
	RemoveItem(szSection, szName);
	pTmp = new CApplicationProfileItemFloat(this, szSection, szName, val, nDefault, nMin, nMax);
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

void CApplicationProfile::RemoveSection(LPCTSTR szSection)
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

void CApplicationProfile::FlushSection(LPCTSTR szSection)
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
}

BOOL CApplicationProfile::UnloadItem(LPCTSTR szSection, LPCTSTR szName)
{
	return FlushItem(szSection, szName) && RemoveItem(szSection, szName);
}

void CApplicationProfile::UnloadSection(LPCTSTR szSection)
{
	FlushSection(szSection);
	RemoveSection(szSection);
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

void CApplicationProfile::ResetSectionToDefault(LPCTSTR szSection)
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

void CApplicationProfile::RevertSectionToInitial(LPCTSTR szSection)
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
}
		// saves the section in INI file. If section name is empty,
		// saves all of them
BOOL CApplicationProfile::ExportSection(LPCTSTR szSection, LPCTSTR szFilename)
{
	if (NULL == szFilename || 0 == szFilename[0])
	{
		return FALSE;
	}
	CString OldRegistryKeyName = m_pszRegistryKey;
	CString OldProfileName = m_pszProfileName;
	m_pszRegistryKey.Empty();
	m_pszProfileName = szFilename;
	CloseCachedKeys();
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
		m_pszProfileName = OldProfileName;
		m_pszRegistryKey = OldRegistryKeyName;
		throw;
	}
	m_pszProfileName = OldProfileName;
	m_pszRegistryKey = OldRegistryKeyName;
	return TRUE;
}

// restores the section from INI file. If section name is empty,
// restores all of them.
BOOL CApplicationProfile::ImportSection(LPCTSTR szSection, LPCTSTR szFilename)
{
	if (NULL == szFilename || 0 == szFilename[0])
	{
		return FALSE;
	}
	CString OldRegistryKeyName = m_pszRegistryKey;
	CString OldProfileName = m_pszProfileName;
	m_pszRegistryKey.Empty();
	m_pszProfileName = szFilename;
	CloseCachedKeys();
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
		m_pszProfileName = OldProfileName;
		m_pszRegistryKey = OldRegistryKeyName;
		throw;
	}
	m_pszProfileName = OldProfileName;
	m_pszRegistryKey = OldRegistryKeyName;
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

void CApplicationProfile::SetRegistryKey(LPCTSTR lpszRegistryKey)
{
	ASSERT(lpszRegistryKey != NULL);

	m_pszRegistryKey = lpszRegistryKey;

	m_pszProfileName = AfxGetApp()->m_pszAppName;

	CloseCachedKeys();
}

void CApplicationProfile::SetRegistryKey(UINT nIDRegistryKey)
{
	CString szRegistryKey;
	VERIFY(szRegistryKey.LoadString(nIDRegistryKey));
	SetRegistryKey(szRegistryKey);
}

// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
// creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
LPCTSTR CApplicationProfile::GetProfileName() const
{
	if ( ! m_pszProfileName.IsEmpty())
	{
		return m_pszProfileName;
	}
	else
	{
		return AfxGetApp()->m_pszProfileName;
	}
}

HKEY CApplicationProfile::GetAppRegistryKey()
{
	LPCTSTR RegistryKey = NULL;
	if (m_pszRegistryKey.IsEmpty())
	{
		if ( ! m_pszProfileName.IsEmpty())
		{
			return NULL;
		}
		RegistryKey = AfxGetApp()->m_pszRegistryKey;
	}
	else
	{
		RegistryKey = m_pszRegistryKey;
	}
	if (NULL == RegistryKey)
	{
		// use .INI file
		return NULL;
	}

	LPCTSTR ProfileName = GetProfileName();
	ASSERT(ProfileName != NULL);

	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;

	if (NULL != hCachedRegistryKey)
	{
		return hCachedRegistryKey;
	}

	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("software"), 0, KEY_WRITE|KEY_READ,
					&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, RegistryKey, 0, REG_NONE,
							REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
							&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, ProfileName, 0, REG_NONE,
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

void CApplicationProfile::CloseCachedKeys()
{
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

HKEY CApplicationProfile::GetSectionKey(LPCTSTR lpszSection)
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
		return NULL;  // use .INI file

	DWORD dw;
	RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
					REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
					&hSectionKey, &dw);

	hCachedSectionKey = hSectionKey;
	sCachedSectionName = lpszSection;
	return hSectionKey;
}

UINT CApplicationProfile::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry,
										int nDefault)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (NULL != hSecKey) // use registry
	{
		DWORD dwValue;
		DWORD dwType;
		DWORD dwCount = sizeof(DWORD);
		LONG lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
										(LPBYTE)&dwValue, &dwCount);
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
		LPCTSTR ProfileName = GetProfileName();
		ASSERT(ProfileName != NULL);
		return ::GetPrivateProfileInt(lpszSection, lpszEntry, nDefault,
									ProfileName);
	}
}

CString CApplicationProfile::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
											LPCTSTR lpszDefault)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (NULL != hSecKey) // use registry
	{
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
		LPCTSTR ProfileName = GetProfileName();
		ASSERT(ProfileName != NULL);

		if (lpszDefault == NULL)
			lpszDefault = &afxChNil;    // don't pass in NULL
		TCHAR szT[4096];
		DWORD dw = ::GetPrivateProfileString(lpszSection, lpszEntry,
											lpszDefault, szT, 4096, ProfileName);
		ASSERT(dw < 4095);
		return szT;
	}
}

BOOL CApplicationProfile::GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
											BYTE** ppData, UINT* pBytes)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	ASSERT(ppData != NULL);
	ASSERT(pBytes != NULL);
	*ppData = NULL;
	*pBytes = 0;
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (NULL != hSecKey) // use registry
	{

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
		ASSERT(GetProfileName() != NULL);

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

BOOL CApplicationProfile::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry,
										int nValue)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (NULL != hSecKey) // use registry
	{
		LONG lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_DWORD,
									(LPBYTE)&nValue, sizeof(nValue));
		return lResult == ERROR_SUCCESS;
	}
	else
	{
		LPCTSTR ProfileName = GetProfileName();
		ASSERT(ProfileName != NULL);

		TCHAR szT[16];
		wsprintf(szT, _T("%d"), nValue);
		return ::WritePrivateProfileString(lpszSection, lpszEntry, szT,
											ProfileName);
	}
}

BOOL CApplicationProfile::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
											LPCTSTR lpszValue)
{
	ASSERT(lpszSection != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (NULL != hSecKey) // use registry
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
			// necessary to cast away const below
			lResult = ::RegDeleteValue(hSecKey, (LPTSTR)lpszEntry);
		}
		else
		{
			lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_SZ,
									(LPBYTE)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR));
		}
		return lResult == ERROR_SUCCESS;
	}
	else
	{
		LPCTSTR ProfileName = GetProfileName();
		ASSERT(ProfileName != NULL);

		return ::WritePrivateProfileString(lpszSection, lpszEntry, lpszValue,
											ProfileName);
	}
}

BOOL CApplicationProfile::WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
											LPBYTE pData, UINT nBytes)
{
	ASSERT(lpszSection != NULL);
	HKEY hSecKey = GetSectionKey(lpszSection);
	if (NULL != hSecKey) // use registry
	{
		LONG lResult;
		lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_BINARY,
								pData, nBytes);
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

	ASSERT(GetProfileName() != NULL);

	BOOL bResult = WriteProfileString(lpszSection, lpszEntry, lpsz);
	delete[] lpsz;
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
