#pragma once
#include <afxpriv.h>
#include "MessageMapT.h"

// MainFrameExT template
enum MainFrameFeatures
{
	MainFrameRememberMaximized = 1,
	MainFrameNeatCtrlTab = 2,
	MainFrameRecalcLayoutOnDisplayChange = 4,
	MainFrameRecalcLayoutOnSettingChange = 8,
	MainFrameHandlePaletteChange = 0x10,
};
template<int _Feature = MainFrameRememberMaximized
						| MainFrameNeatCtrlTab,
		class Base = CMDIFrameWnd>
class FrameExParameters
{
public:
	typedef Base BaseClass;
	enum { Feature = _Feature};
};

typedef FrameExParameters<> DefaultFrameExParameters;

template<class Parameters = DefaultFrameExParameters>
class CMainFrameExT : public Parameters::BaseClass
{
	typedef typename Parameters::BaseClass BaseClass;
protected:
	CMainFrameExT()
		: m_nRotateChildIndex(0)
	{
		if (Feature & MainFrameRememberMaximized)
		{
			GetApp()->Profile.AddItem(_T("Settings"), _T("OpenMaximized"), m_bOpenMaximized, true);
		}
	}
	~CMainFrameExT()
	{
		if (Feature & MainFrameRememberMaximized)
		{
			GetApp()->Profile.RemoveItem(_T("Settings"), _T("OpenMaximized"));
		}
	}
// Overrides
	// ClassWizard generated virtual function overrides
	////{{AFX_VIRTUAL(CMainFrameExT)
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (Feature & MainFrameNeatCtrlTab)
		{
			// catch Ctrl key down and up
			if (WM_KEYDOWN == pMsg->message)
			{
				if (VK_CONTROL == pMsg->wParam
					&& 0 == (0x40000000 & pMsg->lParam))
				{
					TRACE("Ctrl key was just pressed\n");
					m_nRotateChildIndex = 0;
				}
				else
				{
					if ((VK_TAB == pMsg->wParam || VK_F6 == pMsg->wParam)
						&& (0x8000 & GetKeyState(VK_CONTROL)))
					{
						CMDIChildWnd * pActive = MDIGetActive();
						if (NULL == pActive)
						{
							return TRUE;
						}
						CWnd * pBottom = pActive->GetWindow(GW_HWNDLAST);

						if (pBottom != pActive)
						{
							CWnd * pPlaceWnd = pActive;
							CWnd * pFrameToActivate;
							if (0x8000 & GetKeyState(VK_SHIFT))
							{
								if (m_nRotateChildIndex > 0)
								{
									for (int i = 0; i < m_nRotateChildIndex - 1; i++)
									{
										pPlaceWnd = pPlaceWnd->GetWindow(GW_HWNDNEXT);
										if (pPlaceWnd == pBottom)
										{
											break;
										}
									}
									m_nRotateChildIndex = i;
									if (pPlaceWnd == pBottom)
									{
										pFrameToActivate = pBottom;
										pPlaceWnd = pBottom->GetWindow(GW_HWNDPREV);
									}
									else
									{
										pFrameToActivate = pPlaceWnd->GetWindow(GW_HWNDNEXT);
									}
								}
								else
								{
									pFrameToActivate = pBottom;
									pPlaceWnd = pFrameToActivate;
									m_nRotateChildIndex = 1000;  // arbitrary big
								}
							}
							else
							{
								for (int i = 0; i < m_nRotateChildIndex; i++)
								{
									pPlaceWnd = pPlaceWnd->GetWindow(GW_HWNDNEXT);
									if (pPlaceWnd == pBottom)
									{
										break;
									}
								}
								m_nRotateChildIndex = i + 1;

								if (pPlaceWnd == pBottom)
								{
									pFrameToActivate = pActive->GetWindow(GW_HWNDNEXT);
									m_nRotateChildIndex = 0;
								}
								else
								{
									pFrameToActivate = pPlaceWnd->GetWindow(GW_HWNDNEXT);
								}
							}

							if (0) TRACE("m_nRotateChildIndex=%d, prev active=%X, pFrameToActivate=%X, pPlaceWnd=%X\n",
										m_nRotateChildIndex, pActive, pFrameToActivate, pPlaceWnd);

							// first activate new frame
							((CMDIChildWnd *) pFrameToActivate)->MDIActivate();
							// then move previously active window under pPlaceWnd
							pActive->SetWindowPos(pPlaceWnd, 0, 0, 0, 0,
												SWP_NOACTIVATE
												| SWP_NOMOVE
												| SWP_NOOWNERZORDER
												| SWP_NOSIZE);
						}
						return TRUE;  // message eaten
					}
				}
			}
			else if (WM_KEYUP == pMsg->message
					&& VK_CONTROL == pMsg->wParam)
			{
				m_nRotateChildIndex = 0;
			}
		}
		return BaseClass::PreTranslateMessage(pMsg);
	}

	////}}AFX_VIRTUAL
private:
	int m_nRotateChildIndex;  // used for Ctrl+Tab handling
	bool m_bOpenMaximized;

	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = BaseClass::OnDisplayChange(wParam, lParam);
		if (Feature & MainFrameRecalcLayoutOnDisplayChange)
		{
			RecalcLayout();
		}
		return result;
	}

	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
	{
		BaseClass::OnSettingChange(uFlags, lpszSection);
		if (Feature & MainFrameRecalcLayoutOnSettingChange)
		{
			RecalcLayout();
		}
	}
// Generated message map functions
protected:
	template<bool enable>
	BOOL OnQueryNewPaletteT();

	template<>
	BOOL OnQueryNewPaletteT<true>()
	{
		CDC * dc = GetDC();
		{
			CPushDcPalette hOldPal(dc, GetApp()->GetPalette(), FALSE);
			int redraw = dc->RealizePalette();
			if (redraw)
			{
				GetApp()->BroadcastUpdate();
			}
		}
		ReleaseDC(dc);
		//BaseClass::OnQueryNewPalette();
		return TRUE;
	}
	template<>
	BOOL OnQueryNewPaletteT<false>()
	{
		return BaseClass::OnQueryNewPalette();
	}
	////{{AFX_MSG(CMainFrameExT)
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd)
	{
		if (pFocusWnd != this)
		{
			OnQueryNewPalette();
		}
	}
	afx_msg BOOL OnQueryNewPalette()
	{
		return OnQueryNewPaletteT<0 != (Feature & MainFrameHandlePaletteChange)>();
	}
	afx_msg void OnDestroy()
	{
		if (Feature & MainFrameRememberMaximized)
		{
			WINDOWPLACEMENT wp;
			wp.length = sizeof wp;

			GetWindowPlacement( & wp);
			m_bOpenMaximized = 0 != (wp.flags & WPF_RESTORETOMAXIMIZED);
			GetApp()->Profile.FlushItem(_T("Settings"), _T("OpenMaximized"));
		}
		BaseClass::OnDestroy();
	}
	////}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP_T(CMainFrameExT, BaseClass)
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

typedef CMainFrameExT<> CMainFrameEx;
