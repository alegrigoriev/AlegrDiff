// CApplicationProfile
#ifndef APPLICATIONPROFILE_H
#define APPLICATIONPROFILE_H
#pragma once

class CApplicationProfile;
class CApplicationProfileItem
{
public:
	CApplicationProfileItem * Next;
	virtual void WriteData(BOOL bForceWrite=FALSE) =0;
	virtual void ReadData() =0;
	virtual void ResetToDefault() = 0;
	virtual void ResetToInitial() = 0;
	CApplicationProfileItem(CApplicationProfile * pProfile, LPCTSTR szSection, LPCTSTR szName);
	virtual ~CApplicationProfileItem() {}
	CString Section;
	CString Name;
	CApplicationProfile * m_pProfile;
	BOOL Exists;
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
};

class CApplicationProfileItemLong: public CApplicationProfileItem
{
	LONG & LongRef;
	LONG InitialData;
	LONG m_Default;
	LONG m_MinVal;
	LONG m_MaxVal;
public:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemLong(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, LONG & RefValue,
								LONG Default, LONG MinVal, LONG MaxVal);
};

class CApplicationProfileItemInt: public CApplicationProfileItem
{
protected:
	int & Ref;
	int InitialData;
	int m_Default;
	int m_MinVal;
	int m_MaxVal;
public:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemInt(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, int & RefValue,
								int Default, int MinVal, int MaxVal);
};

class CApplicationProfileItemUint: public CApplicationProfileItem
{
protected:
	unsigned int & Ref;
	unsigned int InitialData;
	unsigned int m_Default;
	unsigned int m_MinVal;
	unsigned int m_MaxVal;
public:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemUint(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, unsigned int & RefValue,
								unsigned int Default, unsigned int MinVal, unsigned int MaxVal);
};

class CApplicationProfileItemShort : public CApplicationProfileItemInt
{
	short & ShortRef;
	int m_TempInt;
public:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemShort(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, short & RefValue,
								short Default, short MinVal, short MaxVal);
};

class CApplicationProfileItemUshort : public CApplicationProfileItemUint
{
	unsigned short & ShortRef;
	unsigned int m_TempInt;
public:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemUshort(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, unsigned short & RefValue,
								unsigned short Default, unsigned short MinVal, unsigned short MaxVal);
};

template<class T> class CApplicationProfileItemBinary: public CApplicationProfileItem
{
	T & m_Ref;
	T m_InitialData;
	T m_Default;
public:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault() { m_Ref = m_Default; }
	virtual void ResetToInitial() { m_Ref = m_InitialData; }
	CApplicationProfileItemBinary(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, T & RefValue,
								T const & Default);
	~CApplicationProfileItemBinary() {}
};

class CApplicationProfileItemBool: public CApplicationProfileItemInt
{
	bool & Ref;
	int m_TempVal;
	bool InitialData;
	bool m_bDefault;
public:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemBool(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, bool & RefValue,
								bool Default = false);
};

class CApplicationProfileItemUlong: public CApplicationProfileItem
{
	ULONG & LongRef;
	ULONG InitialData;
	ULONG m_Default;
	ULONG m_MinVal;
	ULONG m_MaxVal;
public:
	virtual void WriteData(BOOL bForceWrite);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemUlong(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, ULONG & RefValue,
								ULONG Default, ULONG MinVal, ULONG MaxVal);
};

class CApplicationProfileItemDouble: public CApplicationProfileItem
{
	double & DoubleRef;
	double InitialData;
	double m_Default;
	double m_MinVal;
	double m_MaxVal;
public:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemDouble(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, double & RefValue,
								double Default, double MinVal, double MaxVal);
	~CApplicationProfileItemDouble() {}
};

class CApplicationProfileItemFloat: public CApplicationProfileItemDouble
{
	float & FloatRef;
	double InitialData;
	double IntermediateValue;
public:
	virtual void WriteData(BOOL bForceWrite=FALSE);
	virtual void ReadData();
	virtual void ResetToDefault();
	virtual void ResetToInitial();
	CApplicationProfileItemFloat(CApplicationProfile * pProfile,
								LPCTSTR szSection, LPCTSTR szName, float & RefValue,
								double Default, double MinVal, double MaxVal);
	~CApplicationProfileItemFloat() {}
};

class CApplicationProfile
{
public:
	CApplicationProfile();
	~CApplicationProfile();
	// saves the section in INI file. If section name is empty,
	// saves all of them
	BOOL ExportSection(LPCTSTR Section, LPCTSTR szFilename);
	// restores the section from INI file. If section name is empty,
	// restores all of them.
	BOOL ImportSection(LPCTSTR Section, LPCTSTR szFilename);
	void AddItem(LPCTSTR Section, LPCTSTR szName, CString & str,
				LPCTSTR szDefault=_T(""),
				int MaxLen=256);
	void AddItem(LPCTSTR szSection, LPCTSTR szName, LONG & val,
				LONG nDefault = 0, LONG nMin = LONG_MIN, LONG nMax=LONG_MAX);
	void AddItem(LPCTSTR szSection, LPCTSTR szName, ULONG & val,
				ULONG nDefault = 0, ULONG nMin = 0, ULONG nMax=ULONG_MAX);

	void AddItem(LPCTSTR szSection, LPCTSTR szName, int & val,
				int nDefault = 0, int nMin = INT_MIN, int nMax = INT_MAX);
	void AddItem(LPCTSTR szSection, LPCTSTR szName, unsigned int & val,
				unsigned int nDefault = 0,
				unsigned int nMin = 0, unsigned int nMax = UINT_MAX);

	void AddItem(LPCTSTR szSection, LPCTSTR szName, short & val,
				short nDefault = 0, short nMin = SHRT_MIN, short nMax = SHRT_MAX);
	void AddItem(LPCTSTR szSection, LPCTSTR szName, unsigned short & val,
				unsigned short nDefault = 0,
				unsigned short nMin = 0, unsigned short nMax = USHRT_MAX);

	void AddItem(LPCTSTR szSection, LPCTSTR szName, bool & val,
				bool nDefault = false);
	// Visual C 6 doesn't like out of class member template definition
	template <class T> void AddItem(LPCTSTR szSection, LPCTSTR szName,
									T & value, T const & Default)
	{
		CApplicationProfileItem * pTmp;
		RemoveItem(szSection, szName);
		pTmp = new CApplicationProfileItemBinary<T>(this, szSection, szName, value, Default);
		pTmp->Next = pItems;
		pItems = pTmp;
	}
	void AddBoolItem(LPCTSTR szSection, LPCTSTR szName, int & val,
					int nDefault = 0);
	void AddItem(LPCTSTR szSection, LPCTSTR szName, float & val,
				double nDefault = 0., double nMin = 0., double nMax=0.);
	void AddItem(LPCTSTR szSection, LPCTSTR szName, double & val,
				double nDefault = 0., double nMin = 0., double nMax=0.);
	BOOL RemoveItem(LPCTSTR szSection, LPCTSTR szName);
	void RemoveSection(LPCTSTR szSection);
	BOOL ResetItemToDefault(LPCTSTR szSection, LPCTSTR szName);
	void ResetSectionToDefault(LPCTSTR szSection);
	BOOL RevertItemToInitial(LPCTSTR szSection, LPCTSTR szName);
	void RevertSectionToInitial(LPCTSTR szSection);
	BOOL FlushItem(LPCTSTR szSection, LPCTSTR szName);
	void FlushSection(LPCTSTR szSection);
	void FlushAll()
	{
		FlushSection(NULL);
	}
	BOOL UnloadItem(LPCTSTR szSection, LPCTSTR szName);
	void UnloadSection(LPCTSTR szSection);
	void UnloadAll()
	{
		UnloadSection(NULL);
	}

	void SetRegistryKey(LPCTSTR lpszRegistryKey);
	void SetRegistryKey(UINT nIDRegistryKey);
	HKEY GetAppRegistryKey();
	HKEY GetSectionKey(LPCTSTR lpszSection);
	UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry,
						int nDefault);
	CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
							LPCTSTR lpszDefault);
	BOOL GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
						BYTE** ppData, UINT* pBytes);
	BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry,
						int nValue);
	BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
							LPCTSTR lpszValue);
	BOOL WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
							LPBYTE pData, UINT nBytes);
	void CloseCachedKeys();
	LPCTSTR GetProfileName() const;

	CApplicationProfileItem * pItems;
	HKEY hCachedRegistryKey;
	HKEY hCachedSectionKey;
	CString sCachedSectionName;
	CString m_pszProfileName;
	CString m_pszRegistryKey;
	CString m_pszAppName;
};

template<class T>
CApplicationProfileItemBinary<T>::CApplicationProfileItemBinary(
	CApplicationProfile * pProfile, LPCTSTR szSection, LPCTSTR szName,
	T & Reference, T const & Default)
	: CApplicationProfileItem(pProfile, szSection, szName),
	m_Ref(Reference), m_Default(Default)
{
	// read value
	ReadData();
	m_InitialData = m_Ref;
}

template<class T> void CApplicationProfileItemBinary<T>::ReadData()
{
	BYTE * pData = NULL;
	UINT ReadBytes = 0;
	BOOL res = m_pProfile->GetProfileBinary(Section, Name, & pData, & ReadBytes);
	if (res && NULL != pData && ReadBytes == sizeof (T))
	{
		memcpy( & m_Ref, pData, sizeof (T));
	}
	else
	{
		memcpy( & m_Ref, & m_Default, sizeof (T));
	}
	delete[] pData;
}

template<class T> void CApplicationProfileItemBinary<T>::WriteData(BOOL bForceWrite)
{
	if (bForceWrite || memcmp(& m_Ref, & m_InitialData, sizeof (T)))
	{
		m_pProfile->WriteProfileBinary(Section, Name, LPBYTE( & m_Ref), sizeof (T));
	}
}

template<int NumStrings = 10>
class CStringHistory
{
public:
	CStringHistory(CApplicationProfile * pProfile,
					LPCTSTR Section, LPCTSTR KeyFormat);
	~CStringHistory();
	void Load();
	void Flush();
	void LoadCombo(CComboBox & Combo);
	void AddString(CString const & str);
private:
	CString m_Strings[NumStrings];
	CApplicationProfile * m_pProfile;
};


#endif
