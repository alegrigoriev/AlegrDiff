#pragma once
#include "afxwin.h"
#include "ApplicationProfile.h"
#include "ResizableDialog.h"

// CDirectoryFingerprintDlg dialog

class CDirectoryFingerprintDlg : public CResizableDialog
{

public:
	CDirectoryFingerprintDlg(
							LPCTSTR sIgnoreFilesFilter,
							BOOL bIncludeSubdirectories,
							CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirectoryFingerprintDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_DIR_FINGERPRINT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	LPCTSTR GetFilenameFilter() const
	{
		return m_sFilenameFilter;
	}
	LPCTSTR GetDirectory() const
	{
		return m_sDirectory;
	}
	LPCTSTR GetFingerprintName() const
	{
		return m_sSaveFilename;
	}
	LPCTSTR GetIgnoreFiles() const
	{
		return m_sIgnoreFiles;
	}
	LPCTSTR GetIgnoreFolders() const
	{
		return m_sIgnoreFolders;
	}
	bool DoSaveAsUnicode() const
	{
		return m_bSaveAsUnicode != 0;
	}
	bool DoIncludeSubdirectories() const
	{
		return m_bIncludeSubdirectories != 0;
	}
	bool DoIncludeDirectoryStructure() const
	{
		return m_bIncludeDirectoryStructure != 0;
	}
protected:
	BOOL m_bIncludeDirectoryStructure;
	BOOL m_bIncludeSubdirectories;
	BOOL m_bSaveAsUnicode;

	CComboBox m_DirCombo;
	CString m_sDirectory;

	CApplicationProfile m_Profile;
	CComboBox m_FilenameFilterCombo;
	CString m_sFilenameFilter;

	CComboBox m_cbIgnoreFiles;
	CString m_sIgnoreFiles;
	CStringHistory m_IgnoreFilterHistory;

	CComboBox m_cbIgnoreFolders;
	CString m_sIgnoreFolders;
	CStringHistory m_IgnoreFolderHistory;

	CComboBox m_SaveFilename;
	CString m_sSaveFilename;
	BOOL m_bOkToOverwriteFile;
	CStringHistory m_FingerprintFilenameHistory;

	virtual BOOL OnInitDialog();
	virtual void OnMetricsChange();

	afx_msg void OnBnClickedButtonBrowseDir();
	afx_msg void OnBnClickedIncludeSubdirs();
	afx_msg void OnBnClickedButtonBrowseSaveFilename();
	afx_msg void OnCbnEditchangeComboFirstDir();
	afx_msg void OnCbnSelchangeComboFirstDir();
	afx_msg void OnCbnEditchangeComboSaveFilename();
	afx_msg void OnCbnSelchangeComboSaveFilename();

	void OnUpdateOk(CCmdUI * pCmdUI);
	void OnUpdateIgnoreDirs(CCmdUI * pCmdUI);
	void OnUpdateIncludeDirectoryStructure(CCmdUI * pCmdUI);
	virtual void OnOK();
};
