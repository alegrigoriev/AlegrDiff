// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
// CApplicationProfile
#ifndef APPLICATIONPROFILE_H
#define APPLICATIONPROFILE_H
#pragma once
#include "KListEntry.h"

class CApplicationProfileItem;

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

	void AddItem(LPCTSTR szSection, LPCTSTR szName,
				void * pValue, void const * pDefault, size_t size);

	template <class T> void AddItem(LPCTSTR szSection, LPCTSTR szName,
									T & value, T const & Default)
	{
		AddItem(szSection, szName, & value, & Default, sizeof T);
	}

	void AddBoolItem(LPCTSTR szSection, LPCTSTR szName, int & val,
					int nDefault = 0);
	void AddItem(LPCTSTR szSection, LPCTSTR szName, float & val,
				double nDefault = 0., double nMin = 0., double nMax=0.);
	void AddItem(LPCTSTR szSection, LPCTSTR szName, double & val,
				double nDefault = 0., double nMin = 0., double nMax=0.);

	// delete the value or key
	void RemoveFromRegistry(LPCTSTR szSection, LPCTSTR szName);

	// remove from list
	void RemoveSection(LPCTSTR szSection);
	BOOL RemoveItem(LPCTSTR szSection, LPCTSTR szName);
	void RemoveAllItems()
	{
		RemoveSection(NULL);
	}


	BOOL ResetItemToDefault(LPCTSTR szSection, LPCTSTR szName);
	void ResetSectionToDefault(LPCTSTR szSection);
	void ResetAllToDefault()
	{
		ResetSectionToDefault(NULL);
	}


	BOOL RevertItemToInitial(LPCTSTR szSection, LPCTSTR szName);
	void RevertSectionToInitial(LPCTSTR szSection);
	void RevertAllToInitial()
	{
		RevertSectionToInitial(NULL);
	}

	// Flush means write all changed data back to profile
	BOOL FlushItem(LPCTSTR szSection, LPCTSTR szName);
	void FlushSection(LPCTSTR szSection);
	void FlushAll()
	{
		FlushSection(NULL);
	}

	// Unload is Flush item and Remove() it from the list
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

	ListHead<CApplicationProfileItem> Items;

	HKEY hCachedRegistryKey;
	HKEY hCachedSectionKey;
	CString sCachedSectionName;
	CString m_pszProfileName;
	CString m_pszRegistryKey;
	CString m_pszAppName;
};

class CStringHistory
{
public:
	enum { Trim = 1, CaseSensitive = 2 };
	CStringHistory(CApplicationProfile * pProfile,
					LPCTSTR Section, LPCTSTR KeyFormat, int NumStrings, ULONG Flags = Trim);
	CStringHistory(CStringHistory * pSourceHistory);
	~CStringHistory();
	void Load(LPCTSTR DefaultFirstString = _T(""));
	void Flush();
	void LoadCombo(CComboBox * pCb);
	void AddString(CString const & str, int AtIndex = 0);
	void DeleteString(CString const & str, int StartFromIndex = 0);

	int Size() const { return m_NumStrings; }
	CString const & operator[](int index) const { return m_Strings[index]; }
	CString & operator[](int index) { return m_Strings[index]; }

protected:
	enum { AttachedHistory = 4 };
	CString * m_Strings;
	int m_NumStrings;
	CApplicationProfile * m_pProfile;
	CString m_ProfileSection;
	CString m_KeyFormat;
	ULONG m_Flags;
private:
};

#endif
