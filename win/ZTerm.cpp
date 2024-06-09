// ZTerm.cpp : main source file for ZTerm.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>

#include "XBitmap.h"
#include "resource.h"
#include "ZTerm.h"

#include "InputView.h"
#include "OutputView.h"
#include "EditView.h"
#include "View.h"
#include "XDlg.h"
#include "XView.h"
#include "MainFrm.h"

HWND	g_hWndTTY = nullptr;
HWND	g_hWndXPlayer = nullptr;

HINSTANCE g_hInstance = nullptr;
WCHAR     g_exePath[MAX_PATH + 1]    = { 0 };
WCHAR     g_appPath[MAX_PATH + 1]    = { 0 };
WCHAR     g_cachePath[MAX_PATH + 1]  = { 0 };
WCHAR     g_dbFilePath[MAX_PATH + 1] = { 0 };

volatile LONG  g_threadCount = 0;
volatile LONG  g_threadCountBKG = 0;
volatile LONG  g_Quit = 0;
volatile DWORD g_dwMainThreadID = 0;

/* the message queue from the remote server */
MessageTask* g_sendQueue = nullptr;
MessageTask* g_receQueue = nullptr;

/* used to sync different threads */
CRITICAL_SECTION     g_csSendMsg;
CRITICAL_SECTION     g_csReceMsg;

MemoryPoolContext g_sendMemPool = nullptr;
MemoryPoolContext g_receMemPool = nullptr;
MemoryPoolContext g_kbMemPool = nullptr;


/* for edit window */
U8* g_editBuf = nullptr;
U32 g_editLen = 0;
U32 g_editMax = EDITW_DEFAULT_BLOCK;

ZXConfig ZXCONFIGURATION = { 0 };
#if 0
ZXFileTree ZXFILETREE = { 0 };
#endif 

ZXKBTree* g_rootKBNode = nullptr;

/* Direct2D and DirectWrite factory */
ID2D1Factory* g_pD2DFactory = nullptr;
IDWriteFactory* g_pDWriteFactory = nullptr;
UINT g_nDPI = 96;

static D2D1_DRAW_TEXT_OPTIONS d2dDrawTextOptions = D2D1_DRAW_TEXT_OPTIONS_NONE;
static HMODULE hDLLD2D{};
static HMODULE hDLLDWrite{};

CAppModule _Module;

static void LoadD2DOnce()
{
	DWORD loadLibraryFlags = 0;
	HMODULE kernel32 = ::GetModuleHandleW(L"kernel32.dll");

	if (kernel32)
	{
		if (::GetProcAddress(kernel32, "SetDefaultDllDirectories"))
		{
			// Availability of SetDefaultDllDirectories implies Windows 8+ or
			// that KB2533623 has been installed so LoadLibraryEx can be called
			// with LOAD_LIBRARY_SEARCH_SYSTEM32.
			loadLibraryFlags = LOAD_LIBRARY_SEARCH_SYSTEM32;
		}
	}

	typedef HRESULT(WINAPI* D2D1CFSig)(D2D1_FACTORY_TYPE factoryType, REFIID riid, CONST D2D1_FACTORY_OPTIONS* pFactoryOptions, IUnknown** factory);
	typedef HRESULT(WINAPI* DWriteCFSig)(DWRITE_FACTORY_TYPE factoryType, REFIID iid, IUnknown** factory);

	hDLLD2D = ::LoadLibraryEx(TEXT("D2D1.DLL"), 0, loadLibraryFlags);
	D2D1CFSig fnD2DCF = DLLFunction<D2D1CFSig>(hDLLD2D, "D2D1CreateFactory");

	if (fnD2DCF)
	{
		// A single threaded factory as Scintilla always draw on the GUI thread
		fnD2DCF(D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory),
			nullptr,
			reinterpret_cast<IUnknown**>(&g_pD2DFactory));
	}

	hDLLDWrite = ::LoadLibraryEx(TEXT("DWRITE.DLL"), 0, loadLibraryFlags);
	DWriteCFSig fnDWCF = DLLFunction<DWriteCFSig>(hDLLDWrite, "DWriteCreateFactory");
	if (fnDWCF)
	{
		const GUID IID_IDWriteFactory2 = // 0439fc60-ca44-4994-8dee-3a9af7b732ec
		{ 0x0439fc60, 0xca44, 0x4994, { 0x8d, 0xee, 0x3a, 0x9a, 0xf7, 0xb7, 0x32, 0xec } };

		const HRESULT hr = fnDWCF(DWRITE_FACTORY_TYPE_SHARED,
			IID_IDWriteFactory2,
			reinterpret_cast<IUnknown**>(&g_pDWriteFactory));

		if (SUCCEEDED(hr))
		{
			// D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
			d2dDrawTextOptions = static_cast<D2D1_DRAW_TEXT_OPTIONS>(0x00000004);
		}
		else
		{
			fnDWCF(DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&g_pDWriteFactory));
		}
	}
}

static bool LoadD2D()
{
	static std::once_flag once;

	ReleaseUnknown(g_pD2DFactory);
	ReleaseUnknown(g_pDWriteFactory);

	std::call_once(once, LoadD2DOnce);

	return g_pDWriteFactory && g_pD2DFactory;
}

static int GetAppPath(HINSTANCE hInstance)
{
	int ret = 1;
	DWORD i, k, len;
	
	i = 0;
	len = GetModuleFileNameW(hInstance, g_exePath, MAX_PATH);

	if (len > 0)
	{
		for (i = len - 1; i > 0; i--)
		{
			if (g_exePath[i] == L'\\')
				break;
		}
	}

	if (i > 0 && i < (MAX_PATH - 8))
	{
		for (k = 0; k < i; k++)
		{
			g_dbFilePath[k] = g_exePath[k];
			g_cachePath[k]  = g_exePath[k];
			g_appPath[k]    = g_exePath[k];
		}

		g_appPath[i] = L'\0';

		g_dbFilePath[i + 0] = L'\\';
		g_dbFilePath[i + 1] = L'a';
		g_dbFilePath[i + 2] = L'i';
		g_dbFilePath[i + 3] = L'.';
		g_dbFilePath[i + 4] = L'd';
		g_dbFilePath[i + 5] = L'b';
		g_dbFilePath[i + 6] = L'\0';

		g_cachePath[i + 0] = L'\\';
		g_cachePath[i + 1] = L't';
		g_cachePath[i + 2] = L'\0';

		ret = 0;
	}
	return ret;
}

static int AppInit(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
	int ret = 0;
	
	g_sendQueue = nullptr;
	g_receQueue = nullptr;

	g_editBuf = nullptr;
	g_editLen = 0;
	g_editMax = 0;

	zx_InitZXConfig(&ZXCONFIGURATION);
	DUI_Init();

	ret = GetAppPath(hInstance);

	InitializeCriticalSection(&g_csSendMsg);  /* these two are Critial Sections to sync different threads */
	InitializeCriticalSection(&g_csReceMsg);

	if (Scintilla_RegisterClasses(hInstance) == 0)
		ret = 2;

	if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) /* initialize libCURL */
		ret = 3;

	if (!LoadD2D())
		ret = 4;

	g_sendMemPool = wt_mempool_create("SendMsgPool", ALLOCSET_DEFAULT_SIZES);
	if (g_sendMemPool == nullptr)
		ret = 5;

	g_receMemPool = wt_mempool_create("ReceMsgPool", ALLOCSET_DEFAULT_SIZES);
	if (g_receMemPool == nullptr)
		ret = 6;

	g_kbMemPool = wt_mempool_create("KBMsgPool", ALLOCSET_DEFAULT_SIZES);
	if (g_kbMemPool == nullptr)
		ret = 6;

	PTerm_Init(hInstance, hPrevInstance, "", nCmdShow);

	return ret;
}

static void AppTerm()
{
	UINT tries;

	// tell all threads to quit
	InterlockedIncrement(&g_Quit);

	zx_SaveAllBeforeExit(); /* save the current configuration before exit */

	if (g_editBuf)
	{
		VirtualFree(g_editBuf, 0, MEM_RELEASE);
		g_editBuf = 0;
	}
	g_editLen = 0;
	g_editMax = 0;

	// wait for all threads to quit gracefully
	tries = 20;
	while (g_threadCount && tries > 0)
	{
		Sleep(1000);
		tries--;
	}

	ATLASSERT(g_threadCount == 0);
	ATLASSERT(g_threadCountBKG == 0);

	DUI_Term();

	g_sendQueue = nullptr;
	g_receQueue = nullptr;
	wt_mempool_destroy(g_sendMemPool);
	wt_mempool_destroy(g_receMemPool);
	wt_mempool_destroy(g_kbMemPool);

	PTerm_Exit();
	Scintilla_ReleaseResources();
	curl_global_cleanup();

	ReleaseUnknown(g_pDWriteFactory);
	ReleaseUnknown(g_pD2DFactory);

	if (hDLLDWrite)
	{
		FreeLibrary(hDLLDWrite);
		hDLLDWrite = {};
	}
	if (hDLLD2D)
	{
		FreeLibrary(hDLLD2D);
		hDLLD2D = {};
	}

	DeleteCriticalSection(&g_csSendMsg);
	DeleteCriticalSection(&g_csReceMsg);

}

///////////////////////////////////////////////////////////////////////////////
// TMessageLoop - message loop implementation to hook PTerm

#define WM_NETEVENT  (WM_APP + 5)

class XMessageLoop
{
public:
	ATL::CSimpleArray<CMessageFilter*> m_aMsgFilter;
	ATL::CSimpleArray<CIdleHandler*> m_aIdleHandler;
	MSG m_msg;

	XMessageLoop()
	{
		memset(&m_msg, 0, sizeof(m_msg));
	}

	virtual ~XMessageLoop()
	{ }

	// Message filter operations
	BOOL AddMessageFilter(CMessageFilter* pMessageFilter)
	{
		return m_aMsgFilter.Add(pMessageFilter);
	}

	BOOL RemoveMessageFilter(CMessageFilter* pMessageFilter)
	{
		return m_aMsgFilter.Remove(pMessageFilter);
	}

	// Idle handler operations
	BOOL AddIdleHandler(CIdleHandler* pIdleHandler)
	{
		return m_aIdleHandler.Add(pIdleHandler);
	}

	BOOL RemoveIdleHandler(CIdleHandler* pIdleHandler)
	{
		return m_aIdleHandler.Remove(pIdleHandler);
	}

	// message loop
	int Run()
	{
		BOOL bDoIdle = TRUE;
		int nIdleCount = 0;
		BOOL bRet;

		for (;;)
		{
			bRet = PTerm_before_loop(&m_msg);

			//while (bDoIdle && !::PeekMessage(&m_msg, nullptr, 0, 0, PM_NOREMOVE))
			while (bDoIdle && !bRet)
			{
				if (!OnIdle(nIdleCount++))
					bDoIdle = FALSE;
			}
#if 0
			bRet = ::GetMessage(&m_msg, nullptr, 0, 0);

			if (bRet == -1)
			{
				ATLTRACE2(atlTraceUI, 0, _T("::GetMessage returned -1 (error)\n"));
				continue;   // error, don't process
			}
			else if (!bRet)
			{
				ATLTRACE2(atlTraceUI, 0, _T("XMessageLoop::Run - exiting\n"));
				break;   // WM_QUIT, exit message loop
			}
#endif 
			while (::PeekMessage(&m_msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (m_msg.message == WM_QUIT)
					goto finished;

				if (g_hWndTTY != nullptr && m_msg.hwnd == g_hWndTTY)
				{
					::DispatchMessage(&m_msg);
					if (m_msg.message != WM_NETEVENT)
						break;
				}
				else
				{
					if (!PreTranslateMessage(&m_msg))
					{
						::TranslateMessage(&m_msg);
						::DispatchMessage(&m_msg);
					}
				}
			}

			if (IsIdleMessage(&m_msg))
			{
				bDoIdle = TRUE;
				nIdleCount = 0;
			}

			PTerm_run_toplevel_callbacks();
		}
	finished:
		return (int)m_msg.wParam;
	}

#if 0
	// message loop
	int Run()
	{
		BOOL bDoIdle = TRUE;
		int nIdleCount = 0;
		BOOL bRet = FALSE;

		for (;;)
		{
			while (bDoIdle && !::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
			{
				if (!OnIdle(nIdleCount++))
					bDoIdle = FALSE;
			}

			bRet = ::GetMessage(&m_msg, NULL, 0, 0);

			if (bRet == -1)
			{
				ATLTRACE2(atlTraceUI, 0, _T("::GetMessage returned -1 (error)\n"));
				continue;   // error, don't process
			}
			else if (!bRet)
			{
				ATLTRACE2(atlTraceUI, 0, _T("CMessageLoop::Run - exiting\n"));
				break;   // WM_QUIT, exit message loop
			}

			if (!PreTranslateMessage(&m_msg))
			{
				::TranslateMessage(&m_msg);
				::DispatchMessage(&m_msg);
			}

			if (IsIdleMessage(&m_msg))
			{
				bDoIdle = TRUE;
				nIdleCount = 0;
			}
		}

		return (int)m_msg.wParam;
	}
#endif 
	// Overrideables
		// Override to change message filtering
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		// loop backwards
		for (int i = m_aMsgFilter.GetSize() - 1; i >= 0; i--)
		{
			CMessageFilter* pMessageFilter = m_aMsgFilter[i];
			if ((pMessageFilter != NULL) && pMessageFilter->PreTranslateMessage(pMsg))
				return TRUE;
		}
		return FALSE;   // not translated
	}

	// override to change idle processing
	virtual BOOL OnIdle(int /*nIdleCount*/)
	{
		for (int i = 0; i < m_aIdleHandler.GetSize(); i++)
		{
			CIdleHandler* pIdleHandler = m_aIdleHandler[i];
			if (pIdleHandler != NULL)
				pIdleHandler->OnIdle();
		}
		return FALSE;   // don't continue
	}

	// override to change non-idle messages
	virtual BOOL IsIdleMessage(MSG* pMsg) const
	{
		// These messages should NOT cause idle processing
		switch (pMsg->message)
		{
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
		case WM_PAINT:
		case 0x0118:	// WM_SYSTIMER (caret blink)
			return FALSE;
		}

		return TRUE;
	}
};

class CAppThreadManager
{
public:
	// thread init param
	struct _RunData
	{
		LPWSTR lpstrCmdLine;
		int nCmdShow;
	};

	// thread proc
	static DWORD WINAPI RunThread(LPVOID lpData)
	{
		int nRet = 0;
		XMessageLoop theLoop;

		_Module.AddMessageLoop((CMessageLoop*)&theLoop);
		InterlockedIncrement(&g_threadCount);

		CMainFrame* wndMain = new CMainFrame;
		ATLASSERT(wndMain);
		if (wndMain)
		{
			RECT rw;
			int screenWidth = GetSystemMetrics(SM_CXSCREEN);
			int screenHeight = GetSystemMetrics(SM_CYSCREEN);

			rw.left = 16;
			rw.right = screenWidth - 32;
			rw.top = 16;
			rw.bottom = screenHeight - 32;

			if (wndMain->CreateEx(NULL, &rw) == NULL)
			{
				ATLTRACE(_T("Main window creation failed!\n"));
				delete wndMain;
				return 0;
			}
			wndMain->ShowWindow(SW_SHOW);

			nRet = theLoop.Run();

			_Module.RemoveMessageLoop();
			delete wndMain;
		}

		InterlockedDecrement(&g_threadCount);
		return nRet;
	}

	DWORD  m_dwCount = 0;
	HANDLE m_arrThreadHandles[1];

	CAppThreadManager()
	{
		m_dwCount = 0;
		m_arrThreadHandles[0] = nullptr;
	}

	// Operations
	DWORD AddThread(LPWSTR lpstrCmdLine, int nCmdShow)
	{
		DWORD dwThreadID;
		HANDLE hThread;
		if (m_dwCount == 1)
		{
			::MessageBox(NULL, _T("ERROR: Cannot create ANY MORE threads!!!"), _T("ZTerm@AI"), MB_OK);
			return 0;
		}

		hThread = ::CreateThread(NULL, 0, RunThread, NULL, 0, &dwThreadID);
		if (hThread == NULL)
		{
			::MessageBox(NULL, _T("ERROR: Cannot create thread!!!"), _T("ZTerm@AI"), MB_OK);
			return 0;
		}

		m_arrThreadHandles[m_dwCount] = hThread;
		m_dwCount++;
		return dwThreadID;
	}

	void RemoveThread(DWORD dwIndex)
	{
		::CloseHandle(m_arrThreadHandles[dwIndex]);
		if (dwIndex != (m_dwCount - 1))
			m_arrThreadHandles[dwIndex] = m_arrThreadHandles[m_dwCount - 1];
		m_dwCount--;
	}

	int Run(LPWSTR lpstrCmdLine, int nCmdShow)
	{
		DWORD dwRet;
		MSG msg;
		// force message queue to be created
		::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

		AddThread(lpstrCmdLine, nCmdShow);

		int nRet = m_dwCount;
		while (m_dwCount > 0)
		{
			dwRet = ::MsgWaitForMultipleObjects(m_dwCount, m_arrThreadHandles, FALSE, INFINITE, QS_ALLINPUT);

			if (dwRet == 0xFFFFFFFF)
			{
				::MessageBox(NULL, _T("ERROR: Wait for multiple objects failed!!!"), _T("ZTerm@AI"), MB_OK);
			}
			else if (dwRet >= WAIT_OBJECT_0 && dwRet <= (WAIT_OBJECT_0 + m_dwCount - 1))
			{
				RemoveThread(dwRet - WAIT_OBJECT_0);
			}
#if 0
			else if (dwRet == (WAIT_OBJECT_0 + m_dwCount))
			{
				if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
#if 0
					if (msg.message == WM_LAUNCH_XPLAYER)
					{
						if (::IsWindow(g_hWndXPlayer))
						{
							//::PostThreadMessage(g_dwXPlayerThreadID, WM_BRING_TO_FRONT, 0, 0L);
							::PostMessage(g_hWndXPlayer, WM_BRING_TO_FRONT, 0, 0);
						}
						else
						{
							DWORD dwThreadID;
							HANDLE hThread;
							WPARAM wp = msg.wParam;
							LPARAM lp = msg.lParam;

							hThread = ::CreateThread(NULL, 0, XPlayerThread, (LPVOID)lp, 0, &dwThreadID);
							if (hThread == NULL)
							{
								::MessageBox(NULL, _T("ERROR: Cannot create thread!!!"), _T("ZTerm@AI"), MB_OK);
								return 0;
							}
						}
					}
#endif 
				}
			}
			else
			{
				::MessageBeep((UINT)-1);
			}
#endif 
		}
#if 0
		if (::IsWindow(g_hWndXPlayer))
		{
			::PostMessage(g_hWndXPlayer, WM_CLOSE, 0, 0);
		}
#endif 
		return nRet;
	}
};

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpstrCmdLine, _In_ int nCmdShow)
{
	int nRet = 0;
	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	nRet = AppInit(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);

	if (nRet == 0)
	{
		g_dwMainThreadID = GetCurrentThreadId();

		CAppThreadManager mgr;
		nRet = mgr.Run(lpstrCmdLine, nCmdShow);
	}
	
	AppTerm();
	_Module.Term();
	::CoUninitialize();

	return nRet;
}
