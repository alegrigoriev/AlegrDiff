// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#include <stdafx.h>
#include "ApplicationProfile.h"
#include <float.h>
#include <afxpriv.h>
#include <vector>

class CApplicationProfileItem : public ListItem<CApplicationProfileItem>
{
protected:
	virtual void WriteData(BOOL bForceWrite=FALSE) =0;
	virtual void ReadData() =0;
	virtual void ResetToDefault() = 0;
	virtual void ResetToInitial() = 0;
	virtual ~CApplicationProfileItem() {}

	CApplicationProfileItem(CApplicationProfile * pProfile, LPCTSTR szSection, LPCTSTR szName);
	CString Section;
	CString Name;
	CApplicationProfile * m_pProfile;
	BOOL Exists;

	friend class CApplicationProfile;
};

class CApplicationProfileItemStr: public CApplicationProfileItem
{
	CString & StrRef;
	CString InitialData;
	CString m_Default;
public:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemStr(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName,
								CString& StringReference, LPCTSTR Default);

	friend class CApplicationProfile;
};

class CApplicationProfileItemLong: public CApplicationProfileItem
{
	LONG & LongRef;
	LONG InitialData;
	LONG m_Default;
	LONG m_MinVal;
	LONG m_MaxVal;

protected:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemLong(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, LONG & RefValue,
								LONG Default, LONG MinVal, LONG MaxVal);

	friend class CApplicationProfile;
};

class CApplicationProfileItemInt: public CApplicationProfileItem
{
protected:
	int & Ref;
	int InitialData;
	int m_Default;
	int m_MinVal;
	int m_MaxVal;

	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();

	CApplicationProfileItemInt(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, int & RefValue,
								int Default, int MinVal, int MaxVal);

	friend class CApplicationProfile;
};

class CApplicationProfileItemUint: public CApplicationProfileItem
{
protected:
	unsigned int & Ref;
	unsigned int InitialData;
	unsigned int m_Default;
	unsigned int m_MinVal;
	unsigned int m_MaxVal;

	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemUint(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, unsigned int & RefValue,
								unsigned int Default, unsigned int MinVal, unsigned int MaxVal);

	friend class CApplicationProfile;
};

class CApplicationProfileItemShort : public CApplicationProfileItemInt
{
	short & ShortRef;
	int m_TempInt;

protected:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemShort(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, short & RefValue,
								short Default, short MinVal, short MaxVal);

	friend class CApplicationProfile;
};

class CApplicationProfileItemUshort : public CApplicationProfileItemUint
{
	unsigned short & ShortRef;
	unsigned int m_TempInt;

protected:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemUshort(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, unsigned short & RefValue,
								unsigned short Default, unsigned short MinVal, unsigned short MaxVal);

	friend class CApplicationProfile;
};

class CApplicationProfileItemBool: public CApplicationProfileItemInt
{
	bool & Ref;
	int m_TempVal;
	bool InitialData;
	bool m_bDefault;
protected:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemBool(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, bool & RefValue,
								bool Default = false);

	friend class CApplicationProfile;
};

class CApplicationProfileItemUlong: public CApplicationProfileItem
{
	ULONG & LongRef;
	ULONG InitialData;
	ULONG m_Default;
	ULONG m_MinVal;
	ULONG m_MaxVal;
protected:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemUlong(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, ULONG & RefValue,
								ULONG Default, ULONG MinVal, ULONG MaxVal);

	friend class CApplicationProfile;
};

class CApplicationProfileItemDouble: public CApplicationProfileItem
{
protected:
	double & DoubleRef;
	double InitialData;
	double m_Default;
	double m_MinVal;
	double m_MaxVal;
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemDouble(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, double & RefValue,
								double Default, double MinVal, double MaxVal);

	friend class CApplicationProfile;
};

class CApplicationProfileItemFloat: public CApplicationProfileItemDouble
{
protected:
	float & FloatRef;
	double InitialData;
	double IntermediateValue;

	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemFloat(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, float & RefValue,
								double Default, double MinVal, double MaxVal);

	friend class CApplicationProfile;
};

class CApplicationProfileItemBinary: public CApplicationProfileItem
{
	void * const m_Pointer;
	std::vector<UCHAR> m_InitialData;
	std::vector<UCHAR> m_Default;
	size_t const m_Size;

protected:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();

	virtual void ResetToDefault();
	virtual void ResetToInitial();

	CApplicationProfileItemBinary(
								CApplicationProfile * pProfile, LPCTSTR szSection, LPCTSTR szName,
								void * Pointer, void const * pDefault, size_t size);

	friend class CApplicationProfile;
};

CApplicationProfileItemBinary::CApplicationProfileItemBinary(
															CApplicationProfile * pProfile, LPCTSTR szSection, LPCTSTR szName,
															void * Pointer, void const * pDefault, size_t size)
	: CApplicationProfileItem(pProfile, szSection, szName)
	, m_Size(size)
// save default data:
	, m_Default((UCHAR const *) pDefault, size + (UCHAR const *) pDefault)
// allocate initial data array:
	, m_InitialData(size)
	, m_Pointer(Pointer)
{
	ASSERT(0 != size);
	// read value
	ReadData();
	// save initial data:
	std::copy((UCHAR const *) m_Pointer, size + (UCHAR const *) m_Pointer, m_InitialData.begin());
}

void CApplicationProfileItemBinary::ReadData()
{
	BYTE * pData = NULL;
	UINT ReadBytes = 0;
	BOOL res = m_pProfile->GetProfileBinary(Section, Name, & pData, & ReadBytes);

	if (res && NULL != pData && ReadBytes == m_Size)
	{
		memcpy(m_Pointer, pData, m_Size);
	}
	else
	{
		memcpy(m_Pointer, & m_Default.front(), m_Size);
	}
	delete[] pData;
}

void CApplicationProfileItemBinary::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || memcmp(m_Pointer, & m_InitialData.front(), m_Size))
	{
		m_pProfile->WriteProfileBinary(Section, Name, LPBYTE(m_Pointer), m_Size);
	}
}

void CApplicationProfileItemBinary::ResetToDefault()
{
	memcpy(m_Pointer, & m_Default.front(), m_Size);
}

void CApplicationProfileItemBinary::ResetToInitial()
{
	memcpy(m_Pointer, & m_InitialData.front(), m_Size);
}

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
	ShortRef = short(m_TempInt);
}

void CApplicationProfileItemShort::ReadData()
{
	CApplicationProfileItemInt::ReadData();
	ShortRef = short(m_TempInt);
}

void CApplicationProfileItemShort::WriteData(BOOL bForceWrite)
{
	m_TempInt = ShortRef;
	CApplicationProfileItemInt::WriteData(bForceWrite);
}

void CApplicationProfileItemShort::ResetToInitial()
{
	ShortRef = short(InitialData);
}

void CApplicationProfileItemShort::ResetToDefault()
{
	ShortRef = short(m_Default);
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
	ShortRef = (unsigned short)(0xFFFF & m_TempInt);
}

void CApplicationProfileItemUshort::ReadData()
{
	CApplicationProfileItemUint::ReadData();
	ShortRef = (unsigned short)(0xFFFF & m_TempInt);
}

void CApplicationProfileItemUshort::WriteData(BOOL bForceWrite)
{
	m_TempInt = ShortRef;
	CApplicationProfileItemUint::WriteData(bForceWrite);
}

void CApplicationProfileItemUshort::ResetToInitial()
{
	ShortRef = (unsigned short)(0xFFFF & InitialData);
}

void CApplicationProfileItemUshort::ResetToDefault()
{
	ShortRef = (unsigned short)(0xFFFF & m_Default);
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
	CString s = m_pProfile->GetProfileString(Section, Name, _T(""));
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
		s.Format(_T("%g"), DoubleRef);
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
	FloatReference = float(IntermediateValue);
}

void CApplicationProfileItemFloat::ReadData()
{
	CApplicationProfileItemDouble::ReadData();
	FloatRef = float(IntermediateValue);
}

void CApplicationProfileItemFloat::WriteData(BOOL bForceWrite)
{
	IntermediateValue = FloatRef;
	CApplicationProfileItemDouble::WriteData(bForceWrite);
}

void CApplicationProfileItemFloat::ResetToInitial()
{
	FloatRef = float(InitialData);
}

void CApplicationProfileItemFloat::ResetToDefault()
{
	CApplicationProfileItemDouble::ResetToDefault();
	FloatRef = float(IntermediateValue);
}

CApplicationProfile::CApplicationProfile()
	:hCachedRegistryKey(NULL),
	hCachedSectionKey(NULL)
{
}

CApplicationProfile::~CApplicationProfile()
{
	while (! Items.IsEmpty())
	{
		delete Items.RemoveHead();
	}

	CloseCachedKeys();
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, CString & str,
								LPCTSTR szDefault,int /*MaxLen*/)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemStr(this, szSection, szName, str, szDefault));
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, LONG & val,
								LONG nDefault, LONG nMin, LONG nMax)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemLong(this, szSection, szName, val, nDefault, nMin, nMax));
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, ULONG & val,
								ULONG nDefault, ULONG nMin, ULONG nMax)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemUlong(this, szSection, szName, val, nDefault, nMin, nMax));
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, int & val,
								int nDefault, int nMin, int nMax)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemInt(this, szSection, szName, val, nDefault, nMin, nMax));
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, unsigned int & val,
								unsigned int nDefault, unsigned int nMin, unsigned int nMax)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemUint(this, szSection, szName, val, nDefault, nMin, nMax));
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, short & val,
								short nDefault, short nMin, short nMax)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemShort(this, szSection, szName, val, nDefault, nMin, nMax));
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, unsigned short & val,
								unsigned short nDefault, unsigned short nMin, unsigned short nMax)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemUshort(this, szSection, szName, val, nDefault, nMin, nMax));
}

void CApplicationProfile::AddBoolItem(LPCTSTR szSection, LPCTSTR szName, int & val,
									int nDefault)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemInt(this, szSection, szName, val, nDefault, 0, 1));
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, bool & val,
								bool nDefault)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemBool(this, szSection, szName, val, nDefault));
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, double & val,
								double nDefault, double nMin, double nMax)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemDouble(this, szSection, szName, val, nDefault, nMin, nMax));
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName, float & val,
								double nDefault, double nMin, double nMax)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemFloat(this, szSection, szName, val, nDefault, nMin, nMax));
}

void CApplicationProfile::AddItem(LPCTSTR szSection, LPCTSTR szName,
								void * pValue, void const * pDefault, size_t size)
{
	RemoveItem(szSection, szName);

	Items.InsertTail(new CApplicationProfileItemBinary(this, szSection, szName, pValue, pDefault, size));
}

void CApplicationProfile::RemoveFromRegistry(LPCTSTR szSection, LPCTSTR szName)
{
	WriteProfileString(szSection, szName, NULL);
}

BOOL CApplicationProfile::RemoveItem(LPCTSTR szSection, LPCTSTR szName)
{
	for (CApplicationProfileItem * p = Items.First();
		Items.NotEnd(p); p = Items.Next(p))
	{
		if (0 == p->Name.CompareNoCase(szName)
			&& 0 == p->Section.CompareNoCase(szSection))
		{
			p->RemoveFromList();

			delete p;

			return TRUE;
		}
	}
	return FALSE;
}

void CApplicationProfile::RemoveSection(LPCTSTR szSection)
{
	for (CApplicationProfileItem * p = Items.First();
		Items.NotEnd(p); )
	{
		CApplicationProfileItem * next = Items.Next(p);

		if (NULL == szSection
			|| 0 == p->Section.CompareNoCase(szSection))
		{
			p->RemoveFromList();

			delete p;
		}

		p = next;
	}
}

BOOL CApplicationProfile::FlushItem(LPCTSTR szSection, LPCTSTR szName)
{
	for (CApplicationProfileItem * p = Items.First();
		Items.NotEnd(p); p = Items.Next(p))
	{
		if (0 == p->Name.CompareNoCase(szName)
			&& 0 == p->Section.CompareNoCase(szSection))
		{
			p->WriteData();
			return TRUE;
		}
	}
	return FALSE;
}

void CApplicationProfile::FlushSection(LPCTSTR szSection)
{
	for (CApplicationProfileItem * p = Items.First();
		Items.NotEnd(p); p = Items.Next(p))
	{
		if (NULL == szSection
			|| 0 == p->Section.CompareNoCase(szSection))
		{
			p->WriteData();
		}
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
	for (CApplicationProfileItem * p = Items.First();
		Items.NotEnd(p); p = Items.Next(p))
	{
		if (0 == p->Name.CompareNoCase(szName)
			&& 0 == p->Section.CompareNoCase(szSection))
		{
			p->ResetToDefault();
			return TRUE;
		}
	}
	return FALSE;
}

void CApplicationProfile::ResetSectionToDefault(LPCTSTR szSection)
{
	for (CApplicationProfileItem * p = Items.First();
		Items.NotEnd(p); p = Items.Next(p))
	{
		if (NULL == szSection
			|| 0 == p->Section.CompareNoCase(szSection))
		{
			p->ResetToDefault();
		}
	}
}

BOOL CApplicationProfile::RevertItemToInitial(LPCTSTR szSection, LPCTSTR szName)
{
	for (CApplicationProfileItem * p = Items.First();
		Items.NotEnd(p); p = Items.Next(p))
	{
		if (0 == p->Name.CompareNoCase(szName)
			&& 0 == p->Section.CompareNoCase(szSection))
		{
			p->ResetToInitial();
			return TRUE;
		}
	}
	return FALSE;
}

void CApplicationProfile::RevertSectionToInitial(LPCTSTR szSection)
{
	for (CApplicationProfileItem * p = Items.First();
		Items.NotEnd(p); p = Items.Next(p))
	{
		if (NULL == szSection
			|| 0 == p->Section.CompareNoCase(szSection))
		{
			p->ResetToInitial();
		}
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
		for (CApplicationProfileItem * p = Items.First();
			Items.NotEnd(p); p = Items.Next(p))
		{
			if (NULL == szSection
				|| 0 == szSection[0]
				|| 0 == p->Section.CompareNoCase(szSection))
			{
				p->WriteData(TRUE);  // force to write
			}
		}
	}
	catch(...)
	{
		m_pszProfileName = OldProfileName;
		m_pszRegistryKey = OldRegistryKeyName;
		throw;
	}

	// flush the file just written
	WritePrivateProfileStringA(NULL, NULL, NULL, NULL);

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

	// flush the cached files
	WritePrivateProfileStringA(NULL, NULL, NULL, NULL);

	try
	{
		for (CApplicationProfileItem * p = Items.First();
			Items.NotEnd(p); p = Items.Next(p))
		{
			if (NULL == szSection
				|| 0 == szSection[0]
				|| 0 == p->Section.CompareNoCase(szSection))
			{
				p->ReadData();
			}
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
		DWORD dwType = REG_NONE;
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
		DWORD dwType = REG_NONE, dwCount = 0;
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
			lpszDefault = _T("");    // don't pass in NULL
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

		DWORD dwType = REG_NONE, dwCount = 0;
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
	lpsz[nBytes*2] = 0;

	ASSERT(GetProfileName() != NULL);

	BOOL bResult = WriteProfileString(lpszSection, lpszEntry, lpsz);
	delete[] lpsz;
	return bResult;
}


/////////////////////////////////////////////////////////////////////////////
CStringHistory::CStringHistory(CApplicationProfile * pProfile,
								LPCTSTR Section, LPCTSTR KeyFormat, int NumStrings, ULONG Flags)
	: m_NumStrings(NumStrings)
	, m_KeyFormat(KeyFormat)
	, m_ProfileSection(Section)
	, m_pProfile(pProfile)
	, m_Strings(new CString[NumStrings])
	, m_Flags(Flags & (Trim | CaseSensitive))
{
}

CStringHistory::CStringHistory(CStringHistory * pSourceHistory)
	: m_NumStrings(pSourceHistory->m_NumStrings)
	, m_KeyFormat(pSourceHistory->m_KeyFormat)
	, m_ProfileSection(pSourceHistory->m_ProfileSection)
	, m_pProfile(pSourceHistory->m_pProfile)
	, m_Strings(pSourceHistory->m_Strings)
	, m_Flags(AttachedHistory | (pSourceHistory->m_Flags & (Trim | CaseSensitive)))
{
}

CStringHistory::~CStringHistory()
{
	if (0 == (m_Flags & AttachedHistory))
	{
		for (int i = 0; i < m_NumStrings; i++)
		{
			CString s;
			s.Format(m_KeyFormat, i);
			m_pProfile->RemoveItem(m_ProfileSection, s);
		}
		delete[] m_Strings;
	}
}

void CStringHistory::Load(LPCTSTR DefaultFirstString)
{
	// if it is attached, the history must already be loaded
	TRACE(_T("CStringHistory::Load: [%s]\n"), LPCTSTR(m_ProfileSection));

	if (m_Flags & AttachedHistory)
	{
		return;
	}

	for (int i = 0; i < m_NumStrings; i++)
	{
		CString s;
		s.Format(m_KeyFormat, i);
		m_pProfile->AddItem(m_ProfileSection, s, m_Strings[i], DefaultFirstString);
		TRACE(_T("Loaded value %s=\"%s\"\n"), LPCTSTR(s), LPCTSTR(m_Strings[i]));
		// use default for the first string only
		DefaultFirstString = _T("");

		if (m_Flags & Trim)
		{
			m_Strings[i].Trim();
		}
	}
}

void CStringHistory::Flush()
{
	TRACE(_T("CStringHistory::Flush: [%s] %s \n"), m_ProfileSection, m_KeyFormat);
	for (int i = 0; i < m_NumStrings; i++)
	{
		CString s;
		s.Format(m_KeyFormat, i);
		m_pProfile->FlushItem(m_ProfileSection, s);
		TRACE(_T("Saved value %s=\"%s\"\n"), LPCTSTR(s), LPCTSTR(m_Strings[i]));
	}
}

void CStringHistory::LoadCombo(CComboBox * pCb)
{
	TRACE(_T("CStringHistory::LoadCombo: [%s]\n"), m_ProfileSection);
	for (int i = 0; i < m_NumStrings; i++)
	{
		if (! m_Strings[i].IsEmpty())
		{
			TRACE(_T("Loaded \"%s\"\n"), m_Strings[i]);

			pCb->AddString(m_Strings[i]);
		}
	}
}

void CStringHistory::DeleteString(CString const & str, int StartFromIndex)
{
	int i, j;
	for (i = StartFromIndex, j = StartFromIndex; i < m_NumStrings; i++)
	{
		if (m_Flags & CaseSensitive)
		{
			if (0 == str.Compare(m_Strings[i])
				// check if previous string is the same
				|| j > StartFromIndex
				&& 0 == m_Strings[j - 1].Compare(m_Strings[i]))
			{
				continue;
			}
		}
		else
		{
			if (0 == str.CompareNoCase(m_Strings[i])
				// check if previous string is the same
				|| j > StartFromIndex
				&& 0 == m_Strings[j - 1].CompareNoCase(m_Strings[i]))
			{
				continue;
			}
		}
		if (i != j)
		{
			m_Strings[j] = m_Strings[i];
		}
		j++;
	}

	for (; j < m_NumStrings; j++)
	{
		m_Strings[j].Empty();
	}
}
void CStringHistory::AddString(CString const & str, int AtIndex)
{
	// remove those that match the currently selected dirs
	if (AtIndex >= m_NumStrings)
	{
		return;
	}
	DeleteString(str, AtIndex);
	// remove last dir from the list
	for (int i = m_NumStrings - 1; i > AtIndex; i--)
	{
		m_Strings[i] = m_Strings[i - 1];
	}
	m_Strings[AtIndex] = str;
}
