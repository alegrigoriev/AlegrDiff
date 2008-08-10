// Copyright Alexander Grigoriev, 2003, All Rights Reserved
// MessageBoxSynch.h: message box which can be called from an arbitrary thread
// It then runs in the context of the main thread
// Make sure not to keep any critical sections entered when calling any of those
#pragma once
//#include "MessageMapT.h"

enum
{
	WM_USER_CALL_MAIN_THREAD = WM_USER+0xAA,
};

// BaseClass is usually CFrameWnd.
// This class is used as a base for CMainFrm class
class MainThreadCall
{
public:
	virtual LRESULT Exec() = 0;
	LRESULT Call(class CSimpleCriticalSection * pLock = NULL);
};

template<class B>
class DialogProxyWnd : public B
{
	typedef B BaseClass;
public:
	template<typename W>
	DialogProxyWnd(W * pParent)
		: BaseClass(pParent)
	{
	}

	template<typename W1, typename W2>
	DialogProxyWnd(W1 * pParent, W2 Context)
		: BaseClass(pParent, Context)
	{
	}

	DialogProxyWnd()
		: BaseClass()
	{
	}

protected:
	DECLARE_MESSAGE_MAP();
//    DECLARE_DYNAMIC(DialogProxyWnd);

private:
	LRESULT OnCallMainThread(WPARAM, LPARAM lparam)
	{
		MainThreadCall * pCall = reinterpret_cast<MainThreadCall *>(lparam);
		return pCall->Exec();
	}

};

BEGIN_TEMPLATE_MESSAGE_MAP(DialogProxyWnd, B, BaseClass)
	ON_MESSAGE(WM_USER_CALL_MAIN_THREAD, OnCallMainThread)
END_MESSAGE_MAP()

// synchronously runs the dialog from a worker thread in the main thread context
INT_PTR RunModalDialogSync(CDialog & dlg);

INT_PTR MessageBoxSync(LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = -1);

INT_PTR MessageBoxSync(UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = 0);

// use this function to wait for completion of a thread
// which can call SendMessage to the main thread.
// To avoid deadlock, this function will schedule messages while waiting
DWORD WaitForSingleObjectAcceptSends(HANDLE handle, ULONG timeout);

