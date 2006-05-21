#pragma once


// CBinaryGoToDlg dialog

class CBinaryGoToDlg : public CDialog
{
	typedef CDialog BaseClass;
public:
	CBinaryGoToDlg(ULONGLONG FileOffset, ULONGLONG FileLength, CWnd* pParent = NULL);   // standard constructor
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
protected:
	ULONGLONG m_FileLength;
	ULONGLONG m_FileOffset;
	ULONGLONG m_InitialOffset;
	ULONGLONG GetNewOffset(LPCTSTR str);
};
