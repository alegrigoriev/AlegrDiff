#pragma once
//#include "MessageMapT.h"

// MainFrameExT template
enum ChildFrameFeatures
{
	ChildFrameRememberMaximized = 1,
};
template<int _Feature = ChildFrameRememberMaximized,
		class Base = CMDIChildWnd>
class ChildFrameExParameters
{
public:
	typedef Base BaseClass;
	enum { Feature = _Feature};
};

typedef ChildFrameExParameters<> DefaultChildFrameExParameters;

template<class Parameters = DefaultChildFrameExParameters>
class CChildFrameExT : public Parameters::BaseClass
{
	typedef typename Parameters::BaseClass BaseClass;
	enum { Feature = Parameters::Feature };

public:
protected:
	CChildFrameExT()
	{
		if (Feature & ChildFrameRememberMaximized)
		{
			static bool IsAddedToProfile = false;
			if ( ! IsAddedToProfile)
			{
				GetApp()->Profile.AddItem(_T("Settings"), _T("OpenChildMaximized"), m_bOpenChildMaximized, true);
				IsAddedToProfile = true;
			}
		}
	}

	~CChildFrameExT()
	{
	}
// Overrides
	// ClassWizard generated virtual function overrides
	////{{AFX_VIRTUAL(CChildFrameExT)
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs)
	{

		if( ! BaseClass::PreCreateWindow(cs) )
		{
			return FALSE;
		}

		if (Feature & ChildFrameRememberMaximized)
		{
			CMDIChildWnd * pActive = ((CMDIFrameWnd *)AfxGetMainWnd())->MDIGetActive();

			if ((pActive == NULL && m_bOpenChildMaximized)
				|| (pActive != NULL && (WS_MAXIMIZE & pActive->GetStyle())))
			{
				cs.style |= WS_VISIBLE | WS_MAXIMIZE;
			}
		}
		return TRUE;
	}

	////}}AFX_VIRTUAL
private:
	static bool m_bOpenChildMaximized;

// Generated message map functions
protected:
	afx_msg void OnDestroy()
	{
		if (Feature & ChildFrameRememberMaximized)
		{
			WINDOWPLACEMENT wp;
			wp.length = sizeof wp;

			GetWindowPlacement( & wp);
			m_bOpenChildMaximized = 0 != (wp.flags & WPF_RESTORETOMAXIMIZED);
		}
		BaseClass::OnDestroy();
	}
	////}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_TEMPLATE_MESSAGE_MAP(CChildFrameExT, Parameters, BaseClass)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

template<class Parameters> bool CChildFrameExT<Parameters>::m_bOpenChildMaximized = true;

typedef CChildFrameExT<> CChildFrameEx;
