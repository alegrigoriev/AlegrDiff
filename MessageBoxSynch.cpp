// Copyright Alexander Grigoriev, 2003, All Rights Reserved
// MessageBoxSynch.cpp: message box which can be called from an arbitrary thread
// It then runs in the context of the main thread
// Make sure not to keep any critical sections entered when calling any of those
#include "stdafx.h"

#include "MessageBoxSynch.h"
#include "SimpleCriticalSection.h"

LRESULT MainThreadCall::Call(CSimpleCriticalSection * pLock)
{
	CWnd * pWnd = AfxGetApp()->m_pMainWnd;
	if (NULL == pWnd
		|| NULL == pWnd->m_hWnd)
	{
		return Exec();
	}

	if (NULL != pLock)
	{
		if (GetCurrentThreadId() == ::GetWindowThreadProcessId(pWnd->m_hWnd, NULL))
		{
			return Exec();
		}

		pLock->Lock();
	}

	LRESULT result = ::SendMessage(pWnd->m_hWnd, WM_USER_CALL_MAIN_THREAD, 0,
									reinterpret_cast<LPARAM>(this));

	if (NULL != pLock)
	{
		pLock->Unlock();
	}

	return result;
}

namespace
{
CSimpleCriticalSection DialogProxyCriticalSection;

class CDialogCall : public MainThreadCall
{
public:
	CDialogCall(CDialog & dlg)
		: m_dlg(dlg)
	{
	}
protected:
	virtual LRESULT Exec()
	{
		return m_dlg.DoModal();
	}
	CDialog & m_dlg;
};

class AfxDialogCall : public MainThreadCall
{
public:
	AfxDialogCall(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
		: m_Text(lpszText), m_Type(nType), m_IDHelp(nIDHelp)
	{
	}
protected:
	virtual LRESULT Exec()
	{
		return AfxMessageBox(m_Text, m_Type, m_IDHelp);
	}
	LPCTSTR const m_Text;
	UINT const m_Type;
	UINT const m_IDHelp;
};

class AfxDialogCallId : public MainThreadCall
{
public:
	AfxDialogCallId(UINT nIDPrompt, UINT nType, UINT nIDHelp)
		: m_IDPrompt(nIDPrompt), m_Type(nType), m_IDHelp(nIDHelp)
	{
	}

protected:
	virtual LRESULT Exec()
	{
		return AfxMessageBox(m_IDPrompt, m_Type, m_IDHelp);
	}
	UINT const m_IDPrompt;
	UINT const m_Type;
	UINT const m_IDHelp;
};
}

INT_PTR RunModalDialogSync(CDialog & dlg)
{
	return CDialogCall(dlg).Call( & DialogProxyCriticalSection);
}

INT_PTR MessageBoxSync(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
{
	return AfxDialogCall(lpszText, nType, nIDHelp).Call( & DialogProxyCriticalSection);
}

INT_PTR MessageBoxSync(UINT nIDPrompt, UINT nType, UINT nIDHelp)
{
	return AfxDialogCallId(nIDPrompt, nType, nIDHelp).Call( & DialogProxyCriticalSection);
}

BOOL WaitForSingleObjectAcceptSends(HANDLE handle, ULONG timeout)
{
	DWORD StartTime = GetTickCount();
	DWORD WaitTimeout = timeout;

	BOOL QuitMessageFetched = FALSE;
	int QuitCode = 0;

	BOOL result = FALSE;

	while (1)
	{
		DWORD WaitResult = MsgWaitForMultipleObjectsEx(1, & handle, WaitTimeout,
														QS_SENDMESSAGE, MWMO_INPUTAVAILABLE);

		if (WAIT_OBJECT_0 == WaitResult)
		{
			result = TRUE;
			break;
		}
		else if (//WAIT_TIMEOUT == WaitResult ||
				WAIT_OBJECT_0 + 1 != WaitResult)    // error or timeout occured
		{
			break;
		}
		// execute messages
		MSG msg;
		if (PeekMessage( & msg, NULL, 0, 0, PM_NOREMOVE | PM_QS_SENDMESSAGE)
			&& WM_QUIT == msg.message)
		{
			QuitMessageFetched = TRUE;
			QuitCode = int(msg.wParam);
		}

		if (timeout != INFINITE)
		{
			DWORD NewTime = GetTickCount();
			DWORD Elapsed = NewTime - StartTime;

			if (Elapsed < 0x8000000)
			{
				if (Elapsed >= WaitTimeout)
				{
					WaitTimeout = 0;
				}
				else
				{
					WaitTimeout -= Elapsed;
				}
			}
			else
			{
				// time stepped back
			}
			StartTime = NewTime;
		}
	}

	if (QuitMessageFetched)
	{
		// repost WM_QUIT back
		PostQuitMessage(QuitCode);
	}

	return result;
}
