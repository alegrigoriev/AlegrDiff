#pragma once


// CBinaryGoToDlg dialog

class CBinaryGoToDlg : public CDialog
{

public:
	CBinaryGoToDlg(ULONGLONG FileOffset, CWnd* pParent = NULL);   // standard constructor
	virtual ~CBinaryGoToDlg();
	ULONGLONG GetOffset() const
	{
		return m_FileOffset;
	}
// Overrides

// Dialog Data
	enum { IDD = IDD_DIALOG_GOTO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	// edit result
	CString m_OffsetStr;
	ULONGLONG m_FileOffset;
};
