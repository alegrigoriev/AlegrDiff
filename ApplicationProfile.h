// CApplicationProfile
#ifndef APPLICATIONPROFILE_H
#define APPLICATIONPROFILE_H

class CApplicationProfileItem
{
public:
	CApplicationProfileItem * Next;
	virtual void WriteData(BOOL bForceWrite=FALSE) =0;
	virtual void ReadData() =0;
	virtual void ResetToDefault() = 0;
	virtual void ResetToInitial() = 0;
	CApplicationProfileItem(LPCTSTR szSection, LPCTSTR szName);
	virtual ~CApplicationProfileItem() {}
	CString Section;
	CString Name;
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
	CApplicationProfileItemStr(LPCTSTR szSection, LPCTSTR szName,
								CString& StringReference, LPCTSTR Default);
	~CApplicationProfileItemStr() {}
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
	CApplicationProfileItemLong(LPCTSTR szSection, LPCTSTR szName, LONG & RefValue,
								LONG Default, LONG MinVal, LONG MaxVal);
	~CApplicationProfileItemLong() {}
};

class CApplicationProfileItemInt: public CApplicationProfileItem
{
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
	CApplicationProfileItemInt(LPCTSTR szSection, LPCTSTR szName, int & RefValue,
								int Default, int MinVal, int MaxVal);
	~CApplicationProfileItemInt() {}
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
	CApplicationProfileItemUlong(LPCTSTR szSection, LPCTSTR szName, ULONG & RefValue,
								ULONG Default, ULONG MinVal, ULONG MaxVal);
	~CApplicationProfileItemUlong() {}
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
	CApplicationProfileItemDouble(LPCTSTR szSection, LPCTSTR szName, double & RefValue,
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
	CApplicationProfileItemFloat(LPCTSTR szSection, LPCTSTR szName, float & RefValue,
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
	BOOL AddItem(LPCTSTR Section, LPCTSTR szName, CString & str,
				LPCTSTR szDefault=_T(""),
				int MaxLen=256);
	BOOL AddItem(LPCTSTR szSection, LPCTSTR szName, LONG & val,
				LONG nDefault = 0, LONG nMin = LONG_MIN, LONG nMax=LONG_MAX);
	BOOL AddItem(LPCTSTR szSection, LPCTSTR szName, int & val,
				int nDefault = 0, int nMin = LONG_MIN, int nMax=LONG_MAX);
	BOOL AddItem(LPCTSTR szSection, LPCTSTR szName, ULONG & val,
				ULONG nDefault = 0, ULONG nMin = 0, ULONG nMax=ULONG_MAX);
	BOOL AddBoolItem(LPCTSTR szSection, LPCTSTR szName, int & val,
					int nDefault = 0);
	BOOL AddItem(LPCTSTR szSection, LPCTSTR szName, float & val,
				double nDefault = 0., double nMin = 0., double nMax=0.);
	BOOL AddItem(LPCTSTR szSection, LPCTSTR szName, double & val,
				double nDefault = 0., double nMin = 0., double nMax=0.);
	BOOL RemoveItem(LPCTSTR szSection, LPCTSTR szName);
	BOOL RemoveSection(LPCTSTR szSection);
	BOOL ResetItemToDefault(LPCTSTR szSection, LPCTSTR szName);
	BOOL ResetSectionToDefault(LPCTSTR szSection);
	BOOL RevertItemToInitial(LPCTSTR szSection, LPCTSTR szName);
	BOOL RevertSectionToInitial(LPCTSTR szSection);
	BOOL FlushItem(LPCTSTR szSection, LPCTSTR szName);
	BOOL FlushSection(LPCTSTR szSection);
	BOOL UnloadItem(LPCTSTR szSection, LPCTSTR szName);
	BOOL UnloadSection(LPCTSTR szSection);

	CApplicationProfileItem * pItems;
};


#endif
