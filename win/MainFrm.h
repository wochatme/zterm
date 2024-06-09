// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "XViewStatusBar.h"
#include "XViewChatGap.h"
#include "XViewChatTab.h"
#include "XViewEditGap.h"

#define AI_TIMER_ASKROB			999

#define	IDX_TREEVIEW			100
#define	IDX_TREEVIEW_ADDNODE	101

static DWORD WINAPI openweb_threadfunc(void* param);
static DWORD WINAPI openkb_threadfunc(void* param);

// Splitter panes constants
#define SPLIT_PANE_LEFT			 0
#define SPLIT_PANE_RIGHT		 1
#define SPLIT_PANE_NONE			-1
#define SPLIT_PANE_VERT			 2

// Splitter extended styles
#define SPLIT_PROPORTIONAL		0x00000001
#define SPLIT_NONINTERACTIVE	0x00000002
#define SPLIT_RIGHTALIGNED		0x00000004
#define SPLIT_BOTTOMALIGNED		SPLIT_RIGHTALIGNED
#define SPLIT_GRADIENTBAR		0x00000008
#define SPLIT_FLATBAR			0x00000020
#define SPLIT_FIXEDBARSIZE		0x00000010

// Note: SPLIT_PROPORTIONAL and SPLIT_RIGHTALIGNED/SPLIT_BOTTOMALIGNED are 
// mutually exclusive. If both are set, splitter defaults to SPLIT_PROPORTIONAL.
// Also, SPLIT_FLATBAR overrides SPLIT_GRADIENTBAR if both are set.

#define XWIN_TTY	0x01
#define XWIN_CHAT	0x02
#define XWIN_INPUT	0x04
#define XWIN_EDIT	0x08
#define XWIN_TREE	0x10

class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler
{
	enum 
	{ 
		TOOLBAR_HEIGHT = 0, 
		STATUSBAR_HEIGHT = 20, 
		AICHAT_GAP = 36, 
		NOTEWIN_GAP = 4, 
		WINTAB_HEIGHT = 36
	};
	enum { m_nPanesCount = 2, m_nPropMax = INT_MAX, m_cxyStep = 1 };

	int m_heightToolbar   = 0;
	int m_heightStatusbar = 0;

	U8 m_winStatus = 0;

	HCURSOR m_hCursorNS = NULL;
	ID2D1HwndRenderTarget* m_pD2DRenderTarget = nullptr;
	ID2D1Bitmap* m_pixelBitmap0 = nullptr;
	ID2D1Bitmap* m_pixelBitmap1 = nullptr;
	//ID2D1SolidColorBrush* m_pBrush = nullptr;

	float m_deviceScaleFactor = 1.f;

	int  m_tabPane[m_nPanesCount] = { 0 };
	HWND m_hWndPane1[m_nPanesCount] = { nullptr };
	int  m_heightPane1[m_nPanesCount] = { 0 };
	int  m_gapPane[m_nPanesCount] = { 0 };

	int  m_xySplitterPosLR[m_nPanesCount] = { 0 };
	int  m_xySplitterPosLRNew[m_nPanesCount] = { 0 };
	int  m_cxyDragOffsetLR[m_nPanesCount] = { 0 };
	int  m_LRX = -1;

	U32* m_screenFixed = nullptr;
	U32* m_screenBuff = nullptr;
	U32  m_screenSize = 0;

	U8 m_loadingPercent = 0;
	U8 m_pecentage = 0;
	int m_nAIPane = SPLIT_PANE_RIGHT;

	XViewStatusBar m_duiViewSB;
	XViewChatGap   m_duiViewCG;
	XViewChatTab   m_duiViewCT;
	XViewEditGap   m_duiViewEG;

	CView m_viewWork;
	COutputView m_viewOutput;
	CInputView m_viewInput;
	CEditView m_viewEdit;
	CTreeViewCtrl m_viewTree;
	CImageList m_imageList;

public:
	HWND m_hWndPane[m_nPanesCount] = { nullptr };
	RECT m_rcSplitter;
	int m_xySplitterPos;            // splitter bar position
	int m_xySplitterPosNew;         // internal - new position while moving
	HWND m_hWndFocusSave;
	int m_nDefActivePane;
	int m_cxySplitBar;              // splitter bar width/height
	HCURSOR m_hCursor;
	int m_cxyMin;                   // minimum pane size
	int m_cxyBarEdge;              	// splitter bar edge
	bool m_bFullDrag;
	int m_cxyDragOffset;		// internal
	int m_nProportionalPos;
	bool m_bUpdateProportionalPos;
	DWORD m_dwExtendedStyle;        // splitter specific extended styles
	int m_nSinglePane;              // single pane mode
	int m_xySplitterDefPos;         // default position
	bool m_bProportionalDefPos;     // porportinal def pos

	// Constructor
	CMainFrame() :
		m_xySplitterPos(-1), m_xySplitterPosNew(-1), m_hWndFocusSave(NULL),
		m_nDefActivePane(SPLIT_PANE_NONE), m_cxySplitBar(4), m_hCursor(NULL), m_cxyMin(0), m_cxyBarEdge(0),
		m_bFullDrag(true), m_cxyDragOffset(0), m_nProportionalPos(0), m_bUpdateProportionalPos(true),
		m_dwExtendedStyle(SPLIT_PROPORTIONAL), m_nSinglePane(SPLIT_PANE_NONE),
		m_xySplitterDefPos(-1), m_bProportionalDefPos(false)
	{
		m_hWndPane[SPLIT_PANE_LEFT] = NULL;
		m_hWndPane[SPLIT_PANE_RIGHT] = NULL;

		m_hWndPane1[SPLIT_PANE_LEFT] = NULL;
		m_hWndPane1[SPLIT_PANE_RIGHT] = NULL;

		::SetRectEmpty(&m_rcSplitter);

		m_xySplitterPosLRNew[SPLIT_PANE_LEFT] = -1;
		m_xySplitterPosLRNew[SPLIT_PANE_RIGHT] = -1;

		m_screenFixed = nullptr;
		m_screenBuff  = nullptr;
		m_screenSize  = 0;
		m_nAIPane = SPLIT_PANE_RIGHT;
		m_loadingPercent = 0;

		m_duiViewSB.SetWindowId((const U8*)"DUIWSB", 6);
		m_duiViewCG.SetWindowId((const U8*)"DUIWCG", 6);
		m_duiViewCT.SetWindowId((const U8*)"DUIWCT", 6);
		m_duiViewEG.SetWindowId((const U8*)"DUIWEG", 6);
	}

	virtual ~CMainFrame()
	{
		if (nullptr != m_screenBuff)
		{
			VirtualFree(m_screenBuff, 0, MEM_RELEASE);
		}
		m_screenBuff = nullptr;
		m_screenFixed = nullptr;
		m_screenSize = 0;
	}

public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return FALSE; // m_view.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateToolBar();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER_DUIWINDOW(DUI_ALLMESSAGE, OnDUIWindowMessage)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
		if (IsInteractive())
		{
			MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
			MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
			MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		}
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)

		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(WM_LOADPERCENTMSG, OnLoading)
		MESSAGE_HANDLER(WM_LOADNOTEPADMSG, OnLoadNotes)
		MESSAGE_HANDLER(WM_SYNCKBDATA_MSG, OnSyncKnowledgeBase)
		MESSAGE_HANDLER(WM_STATUSBAR_MSG, OnStatusbar)
		MESSAGE_HANDLER(WM_CHATGAP_MSG, OnChatGap)
		MESSAGE_HANDLER(WM_CHATTAB_MSG, OnChatTab)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_NCCREATE, OnNCCreate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_NET_STATUS_MSG, OnNetworkStatus)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_APP_AI_SETTINGS, OnFileAISettings)
		COMMAND_ID_HANDLER(ID_APP_TTY_SETTINGS, OnFileTTYSetting)
		COMMAND_ID_HANDLER(ID_EDIT_COPYPASTE, OnEditCopyPaste)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnEditPaste)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnEditCut)
		COMMAND_ID_HANDLER(ID_EDIT_UNDO, OnEditUndo)
		COMMAND_ID_HANDLER(ID_VIEW_NOTE_WINDOW, OnViewNoteWindow)
		COMMAND_ID_HANDLER(ID_VIEW_AI_WINDOW, OnViewChatWindow)
		COMMAND_ID_HANDLER(ID_VIEW_TTY_WINDOW, OnViewTTYWindow)
		COMMAND_ID_HANDLER(ID_VIEW_LAYOUT_SWITCH, OnViewSwitchLayOut)
#if 0
		COMMAND_ID_HANDLER(ID_VIEW_INPUT_WIN, OnViewInputWindow)
		COMMAND_ID_HANDLER(ID_VIEW_XPLAYER, OnViewXPlayer)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
#endif 
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	// this is the routine to process all window messages for XView
	LRESULT OnDUIWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		WPARAM wp = wParam;
		LPARAM lp = lParam;

		ClearDUIWindowReDraw(); // reset the bit to indicate the redraw before we handle the messages

		if (uMsg == WM_CREATE)
		{
			wp = (WPARAM)m_hWnd;
		}

		m_duiViewSB.HandleOSMessage((U32)uMsg, (U64)wp, (U64)lp, this);
		m_duiViewCG.HandleOSMessage((U32)uMsg, (U64)wp, (U64)lp, this);
		m_duiViewCT.HandleOSMessage((U32)uMsg, (U64)wp, (U64)lp, this);
		m_duiViewEG.HandleOSMessage((U32)uMsg, (U64)wp, (U64)lp, this);

		// after all virtual windows procced the window message, the whole window may need to fresh
		if (DUIWindowNeedReDraw() && m_loadingPercent > 100) 
		{
			Invalidate(); // send out the WM_PAINT messsage to the window to redraw
		}
		// to allow the host window to continue to handle the windows message
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = 1024;
		lpMMI->ptMinTrackSize.y = 768;
		return 0;
	}

	LRESULT OnLoadNotes(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		U32 length = (U32)wParam;
		if (length)
		{
			U8* data = (U8*)lParam;
			if (data)
			{
				m_viewEdit.SetText((const char*)data, length);
			}
		}
		return 0;
	}

	LRESULT OnSyncKnowledgeBase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PopulateKBTree();
		return 0;
	}

	void UpdateUILayout()
	{
		int nAIPane, nOtherPane;
		ZXConfig* cf = &ZXCONFIGURATION;
		// at this point, the configuration information is updated by the progress thread.
		nAIPane    = (cf->property & AI_PROP_IS_LEFT) ? AI_PROP_IS_LEFT : SPLIT_PANE_RIGHT;
		nOtherPane = (nAIPane == SPLIT_PANE_RIGHT) ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;

		m_heightPane1[nOtherPane] = AI_DEFAULT_EDITWIN_HEIGHT;
		m_heightPane1[nAIPane] = AI_DEFAULT_TYPEWIN_HEIGHT;

		if (nAIPane != m_nAIPane)
		{
			DoSwitchLayout();
		}

		// update the font of the 3 windows
		m_viewOutput.DoFontChange();
		m_viewInput.DoFontChange();
		m_viewEdit.DoFontChange();

		//SetSplitterRect();
		//ReleaseUnknown(m_pD2DRenderTarget);
		GetClientRect(&m_rcSplitter);
		DoSize();
		//Invalidate();
	}

	LRESULT OnLoading(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		U8 cnt = (U8)lParam;

		if (cnt == 5) // at this step the configuratoin information had been loaded.
		{
			zx_StartupNetworkThread(m_hWnd);
			UpdateUILayout();
		}
		else if (cnt == 6) // at this step the configuratoin information had been loaded.
		{
			SetTimer(AI_TIMER_ASKROB, AI_TIMER_ASKROB);
			PopulateKBTree();
		}

		m_loadingPercent = (U8)wParam;
		if (m_loadingPercent > 100)
		{
			::SendMessage(m_viewOutput, SCI_SETVSCROLLBAR, 1, 0);
			::SendMessage(m_viewInput, SCI_SETVSCROLLBAR, 1, 0);
			::SendMessage(m_viewEdit, SCI_SETVSCROLLBAR, 1, 0);

			m_winStatus |= (XWIN_TTY | XWIN_CHAT | XWIN_INPUT);

			if (XWIN_TTY & m_winStatus)
			{
				m_viewWork.ShowWindow(SW_SHOW);
				m_viewWork.SetFocus();
			}
			else
			{
				m_viewWork.ShowWindow(SW_HIDE);
			}

			if (XWIN_CHAT & m_winStatus)
			{
				m_viewOutput.ShowWindow(SW_SHOW);
			}
			else
			{
				m_viewOutput.ShowWindow(SW_HIDE);
			}

			if (XWIN_INPUT & m_winStatus)
			{
				m_viewInput.ShowWindow(SW_SHOW);
			}
			else
			{
				m_viewInput.ShowWindow(SW_HIDE);
			}

			if (XWIN_EDIT & m_winStatus)
			{
				m_viewEdit.ShowWindow(SW_SHOW);
			}
			else
			{
				m_viewEdit.ShowWindow(SW_HIDE);
			}

			SetSplitterPosPct(m_pecentage);
		}

		Invalidate();  // redraw the screen

		return 0;
	}

	void AddKBNode(ZXKBTree* node, HTREEITEM htiParent)
	{
		if (node && htiParent)
		{
			HTREEITEM hti;
			if (wcslen(node->title) > 0)
			{
				TVITEM tvItem = { 0 };
				tvItem.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				hti = m_viewTree.InsertItem(TVIF_TEXT | TVIF_STATE, node->title, 0, 0, 0, 0, 0, htiParent, nullptr);
				if (hti)
				{
					int idx = 0;
					if (node->docId)
					{
						if ((node->status & 0xFF) == ZX_KBNODE_WEB)
							idx = 3;
						else
						{
							if ((node->status & 0xFF) == ZX_KBNODE_TXT)
								idx = 1;
							if ((node->status & 0xFF) == ZX_KBNODE_RDO)
								idx = 2;
						}
					}

					tvItem.hItem = hti;
					tvItem.iImage = idx; 
					tvItem.iSelectedImage = idx; 
					m_viewTree.SetItem(&tvItem);
					m_viewTree.SetItemData(hti, node->docId);
					if (node->child)
					{
						AddKBNode(node->child, hti);
					}
				}
			}
			if (node->next)
			{
				AddKBNode(node->next, htiParent);
			}
		}
	}

	void PopulateKBTree()
	{
		if (g_rootKBNode)
		{
			ZXKBTree* node = g_rootKBNode;
			if (wcslen(node->title) > 0)
			{
				HTREEITEM htiRoot;
				TVITEM tvItem = { 0 };
				tvItem.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;

				m_viewTree.DeleteAllItems();
				htiRoot = m_viewTree.InsertItem(TVIF_TEXT | TVIF_STATE, node->title, 0, 0, 0, 0, 0, nullptr, nullptr);
				if (htiRoot)
				{
					tvItem.hItem = htiRoot;
					tvItem.iImage = 0; // Set image index to 0 for root node (image 1)
					tvItem.iSelectedImage = 0; // Same image for selected state
					m_viewTree.SetItem(&tvItem);
					AddKBNode(node->child, htiRoot);
					m_viewTree.Expand(htiRoot);
				}
			}
		}
		wt_mempool_reset(g_kbMemPool);
		g_rootKBNode = nullptr;
	}

	void Initilize()
	{
		ZXConfig* cf = &ZXCONFIGURATION;
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_TRACKSELECT;

		m_winStatus = 0; // at first all windows are hidden 
		m_heightStatusbar = STATUSBAR_HEIGHT;
		m_tabPane[SPLIT_PANE_LEFT] = 0;
		m_tabPane[SPLIT_PANE_RIGHT] = 0;

		m_gapPane[SPLIT_PANE_LEFT] = NOTEWIN_GAP;
		m_gapPane[SPLIT_PANE_RIGHT] = AICHAT_GAP;
		m_heightPane1[SPLIT_PANE_LEFT] = AI_DEFAULT_EDITWIN_HEIGHT;
		m_heightPane1[SPLIT_PANE_RIGHT] = AI_DEFAULT_TYPEWIN_HEIGHT;

		m_viewWork.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_viewOutput.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		ATLASSERT(m_viewWork.IsWindow());
		ATLASSERT(m_viewOutput.IsWindow());
		m_viewWork.ShowWindow(SW_HIDE);
		m_viewOutput.ShowWindow(SW_HIDE);
		g_hWndTTY = m_viewWork;

		m_viewInput.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_viewEdit.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		ATLASSERT(m_viewInput.IsWindow());
		ATLASSERT(m_viewEdit.IsWindow());
		m_viewInput.ShowWindow(SW_HIDE);
		m_viewEdit.ShowWindow(SW_HIDE);

		m_pecentage = cf->layoutPercent;

		m_nAIPane = SPLIT_PANE_RIGHT;
		if (cf->property & AI_PROP_IS_LEFT)
		{
			m_nAIPane = SPLIT_PANE_LEFT;
			SetSplitterPanes(m_viewOutput, m_viewWork);
			m_tabPane[SPLIT_PANE_LEFT] = WINTAB_HEIGHT;
			m_hWndPane1[SPLIT_PANE_RIGHT] = m_viewEdit;
			m_hWndPane1[SPLIT_PANE_LEFT] = m_viewInput;
		}
		else
		{
			m_pecentage = 100 - cf->layoutPercent;
			SetSplitterPanes(m_viewWork, m_viewOutput);
			m_tabPane[SPLIT_PANE_RIGHT] = WINTAB_HEIGHT;
			m_hWndPane1[SPLIT_PANE_LEFT] = m_viewEdit;
			m_hWndPane1[SPLIT_PANE_RIGHT] = m_viewInput;
		}
		SetSplitterPosPct(m_pecentage);
		SetDefaultActivePane(m_viewWork);

#if 10
		dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
			| TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;
#endif 
		m_viewTree.Create(m_hWnd, rcDefault, NULL, dwStyle, 0, IDX_TREEVIEW);
		ATLASSERT(m_viewTree.IsWindow());
		m_imageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 4);
		HICON hIcon1 = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDX_TREE_FOLDER));
		HICON hIcon2 = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDX_TREETXTFILE));
		HICON hIcon3 = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDX_TREERDOFILE));
		HICON hIcon4 = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDX_TREEWEBFILE));
		m_imageList.AddIcon(hIcon1);
		m_imageList.AddIcon(hIcon2);
		m_imageList.AddIcon(hIcon3);
		m_imageList.AddIcon(hIcon4);
		DestroyIcon(hIcon1);
		DestroyIcon(hIcon2);
		DestroyIcon(hIcon3);
		DestroyIcon(hIcon4);
		m_viewTree.SetImageList(m_imageList);
		m_viewTree.ShowWindow(SW_HIDE);
	}

	LRESULT OnNCCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		Initilize();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DWORD dwThreadID;
		HANDLE hThread = nullptr;

		// remove old menu
		SetMenu(NULL);
		Init();

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		m_loadingPercent = 0;
		hThread = ::CreateThread(NULL, 0, LoadingDataThread, m_hWnd, 0, &dwThreadID);
		if (nullptr == hThread)
		{
			MessageBox(TEXT("Loading thread creation is failed!"), TEXT("ZTerm Error"), MB_OK);
			PostMessage(WM_CLOSE, 0, 0);
		}
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		ZXConfig* cf = &ZXCONFIGURATION;
		RECT windowRect;

		KillTimer(AI_TIMER_ASKROB);

		/* save the window's position */
		GetWindowRect(&windowRect);
		cf->winLeft = windowRect.left;
		cf->winTop = windowRect.top;
		cf->winRight = windowRect.right;
		cf->winBottom = windowRect.bottom;

		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		ReleaseUnknown(m_pixelBitmap0);
		ReleaseUnknown(m_pixelBitmap1);
		ReleaseUnknown(m_pD2DRenderTarget);

		m_viewEdit.SaveText();
		/* save the UI layout for the next startup */
		if (m_heightToolbar > 0)
			cf->property |= AI_PROP_TOOLBAR;
		else
			cf->property &= ~AI_PROP_TOOLBAR;

		if (m_heightStatusbar > 0)
			cf->property |= AI_PROP_STATBAR;
		else
			cf->property &= ~AI_PROP_STATBAR;

		if (m_winStatus & XWIN_INPUT)
			cf->property |= AI_PROP_TYPEWIN;
		else
			cf->property &= ~AI_PROP_TYPEWIN;

		if (m_nAIPane == SPLIT_PANE_LEFT)
		{
			if (m_winStatus & XWIN_EDIT)
				cf->property |= AI_PROP_EDITWIN;
			else
				cf->property &= ~AI_PROP_EDITWIN;
		}
		else
		{
			ATLASSERT(m_nAIPane == SPLIT_PANE_RIGHT);
			if (m_winStatus & XWIN_EDIT)
				cf->property |= AI_PROP_EDITWIN;
			else
				cf->property &= ~AI_PROP_EDITWIN;
		}

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnNetworkStatus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bool networkIsGood = (bool)lParam;
		m_duiViewSB.UpdateNetworkStatus(networkIsGood);
		Invalidate();
		return 0;
	}

	void DoInputNotification(U32 code)
	{
		if (code == SCN_CHARADDED && (XWIN_CHAT & m_winStatus))
		{
			bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0; /* does the user hold Ctrl key? */
			char ch = m_viewInput.GetLastChar();
			if (ch == '\n' && heldControl == false) /* the user hit the ENTER key */
			{
				bool share_screen = false;
				U8 offset = 0;
				U32 length = 0;
				U8* p = m_viewInput.GetInputData(length, offset, share_screen);
				m_viewInput.SetText("");
				if (p)
				{
					U8* screen_data;
					U32 mt_len, screen_len;
					MessageTask* mt = nullptr;
					m_viewOutput.AppendText((const char*)p, length);

					ATLASSERT(length > offset);
					U8* q = p + offset;
					length -= offset;

					// the TTY window is not visible, so we cannot get the screen data
					if ((XWIN_TTY & m_winStatus) == 0) 
						share_screen = false;

					screen_len = 0;
					screen_data = nullptr;
					if (share_screen)
					{
						screen_data = m_viewWork.GetScreenData(screen_len);
					}
					if (screen_len)
						screen_len += 8;

					mt_len = sizeof(MessageTask) + length + screen_len + 1;

					mt = (MessageTask*)wt_palloc(g_sendMemPool, mt_len);
					if (mt)
					{
						U8* s = (U8*)mt;
						mt->next = NULL;
						mt->msg_state = 0;
						mt->msg_length = length + screen_len;
						mt->msg_body = s + sizeof(MessageTask);
						s = mt->msg_body;
						memcpy_s(s, length, q, length);
						if (screen_len)
						{
							ATLASSERT(screen_len > 8);
							s += length;
							*s++ = '"'; *s++ = '"'; *s++ = '"'; *s++ = '\n';
							memcpy_s(s, screen_len - 8, screen_data, screen_len - 8);
							s += screen_len - 8;
							*s++ = '\n'; *s++ = '"'; *s++ = '"'; *s++ = '"';
							*s = '\0';
						}
						zx_PushIntoSendQueue(mt);
					}
				}
			}
		}
	}

	void DoOutputNotification(U32 code)
	{
	}

	void DoEditNotification(U32 code)
	{
	}

	void DoKBTreeNotification(U32 code)
	{
		if (NM_DBLCLK == code || NM_RETURN == code)
		{
			HTREEITEM hItem = m_viewTree.GetSelectedItem();
			if (hItem)
			{
				int nImage = 0;
				int nSelectedImage = 0;
				m_viewTree.GetItemImage(hItem, nImage, nSelectedImage);

				U64 dId = (U64)m_viewTree.GetItemData(hItem);
				if (dId)
				{
					if (nImage == 3) // this is a link, open the browser directly
					{
						DWORD in_threadid = 0; /* required for Win9x */
						HANDLE hThread = CreateThread(NULL, 0, openweb_threadfunc, (void*)dId, 0, &in_threadid);
						if (hThread) /* we don't need the thread handle */
							CloseHandle(hThread);
					}
				}
			}
		}
	}

	LRESULT OnNotify(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPNMHDR lpnmhdr = (LPNMHDR)lParam;
		ATLASSERT(lpnmhdr);
		HWND hWnd = lpnmhdr->hwndFrom;

		if (hWnd == m_viewInput)
		{
			DoInputNotification(lpnmhdr->code);
		}
		else if (hWnd == m_viewEdit)
		{
			DoEditNotification(lpnmhdr->code);
		}
		else if (hWnd == m_viewOutput)
		{
			DoOutputNotification(lpnmhdr->code);
		}
		else if (hWnd == m_viewTree)
		{
			DoKBTreeNotification(lpnmhdr->code);
		}
		return 0;
	}

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam == AI_TIMER_ASKROB && m_loadingPercent > 100)
		{
			MessageTask* p;
			EnterCriticalSection(&g_csReceMsg);
			//////////////////////////////////////////
			p = g_receQueue;
			while (p)
			{
				if (p->msg_state == 0) /* this message is new message */
				{
					if (p->msg_body && p->msg_length)
					{
						m_viewOutput.AppendText((const char*)p->msg_body, p->msg_length);
					}
					p->msg_state = 1;
					break;
				}
				p = p->next;
			}
			//////////////////////////////////////////
			LeaveCriticalSection(&g_csReceMsg);

			zx_PushIntoSendQueue(NULL); // clean up the last processed message task
			m_duiViewSB.InvalidateScreen();
			m_duiViewCG.InvalidateScreen();
			m_duiViewCT.InvalidateScreen();
			m_duiViewEG.InvalidateScreen();
			Invalidate();
		}
		return 0;
	}

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnFileAISettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DoAIConfiguration();
		return 0;
	}

	LRESULT OnFileTTYSetting(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DoTTYConfiguration();
		return 0;
	}

	LRESULT OnEditCopyPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (m_winStatus & XWIN_EDIT) // notepad window is available
		{
			if (m_viewEdit.DoEditCopy())
			{
				PTerm_do_paste(m_viewWork);
				m_viewWork.SetFocus();
			}
		}
		return 0;
	}

	LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		bool bDone = false;
		/* because there are 3 windows can do the copy, we copy from the focused window at first */
		if (!bDone)
		{
			if (m_winStatus & XWIN_CHAT) // notepad window is available
			{
				if (m_viewOutput.GetFocus())
				{
					bDone = m_viewOutput.DoEditCopy();
				}
			}
		}
		if (!bDone)
		{
			if (m_winStatus & XWIN_EDIT) // notepad window is available
			{
				if (m_viewEdit.GetFocus())
				{
					bDone = m_viewEdit.DoEditCopy();
				}
			}
		}
		if (!bDone)
		{
			if (m_winStatus & XWIN_INPUT) // notepad window is available
			{
				if (m_viewInput.GetFocus())
				{
					bDone = m_viewInput.DoEditCopy();
				}
			}
		}
		/* now no window is focuse, we try it again by an order */
		if (!bDone)
		{
			if (m_winStatus & XWIN_CHAT) // notepad window is available
			{
				bDone = m_viewOutput.DoEditCopy();
			}
		}
		if (!bDone)
		{
			if (m_winStatus & XWIN_EDIT) // notepad window is available
			{
				bDone = m_viewEdit.DoEditCopy();
			}
		}
		if (!bDone)
		{
			if (m_winStatus & XWIN_INPUT) // notepad window is available
			{
				bDone = m_viewInput.DoEditCopy();
			}
		}

		return 0;
	}

	LRESULT OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		bool bDone = false;
		if (m_winStatus & XWIN_EDIT) // notepad window is available
		{
			if (m_viewEdit.GetFocus())
			{
				m_viewEdit.DoEditPaste();
				bDone = true;
			}
		}
		if (!bDone && m_winStatus & XWIN_INPUT)
		{
			if (m_viewInput.GetFocus())
			{
				m_viewInput.DoEditPaste();
				bDone = true;
			}
		}
		return 0;
	}

	LRESULT OnEditCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		bool bDone = false;

		if (!bDone)
		{
			if (m_winStatus & XWIN_EDIT) // notepad window is available
			{
				if(m_viewEdit.GetFocus())
					bDone = m_viewEdit.DoEditCut();
			}
		}
		if (!bDone)
		{
			if (m_winStatus & XWIN_INPUT) // notepad window is available
			{
				if (m_viewInput.GetFocus())
					bDone = m_viewInput.DoEditCut();
			}
		}

		return 0;
	}

	LRESULT OnEditUndo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		bool bDone = false;

		if (!bDone)
		{
			if (m_winStatus & XWIN_EDIT) // notepad window is available
			{
				if (m_viewEdit.GetFocus())
					bDone = m_viewEdit.DoEditUndo();
			}
		}
		if (!bDone)
		{
			if (m_winStatus & XWIN_INPUT) // notepad window is available
			{
				if (m_viewInput.GetFocus())
					bDone = m_viewInput.DoEditUndo();
			}
		}
		return 0;
	}

	LRESULT OnViewNoteWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DoNotePadWindow();
		return 0;
	}

	LRESULT OnViewTTYWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DoTTYWindow();
		return 0;
	}

	LRESULT OnViewChatWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DoAIChatWindow();
		return 0;
	}

	LRESULT OnViewSwitchLayOut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DoSwitchLayout();
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

	// Attributes
	void SetSplitterRect(LPRECT lpRect = NULL, bool bUpdate = true)
	{
		if (lpRect == NULL)
		{
			GetClientRect(&m_rcSplitter);
		}
		else
		{
			m_rcSplitter = *lpRect;
		}

		if (IsProportional())
			UpdateProportionalPos();
		else if (IsRightAligned())
			UpdateRightAlignPos();

		if (bUpdate)
			UpdateSplitterLayout();
	}

	void GetSplitterRect(LPRECT lpRect) const
	{
		ATLASSERT(lpRect != NULL);
		*lpRect = m_rcSplitter;
	}

	bool SetSplitterPosLR(int LR, int xyPos = -1, bool bUpdate = true)
	{
		ATLASSERT(LR == SPLIT_PANE_LEFT || LR == SPLIT_PANE_RIGHT);
#if 0
		if (xyPos == -1)   // -1 == default position
		{
			if (m_bProportionalDefPos)
			{
				ATLASSERT((m_xySplitterDefPos >= 0) && (m_xySplitterDefPos <= m_nPropMax));

				if (m_bVertical)
					xyPos = ::MulDiv(m_xySplitterDefPos, m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge, m_nPropMax);
				else
					xyPos = ::MulDiv(m_xySplitterDefPos, m_rcSplitter.bottom - m_rcSplitter.top - m_cxySplitBar - m_cxyBarEdge, m_nPropMax);
			}
			else if (m_xySplitterDefPos != -1)
			{
				xyPos = m_xySplitterDefPos;
			}
			else   // not set, use middle position
			{
				if (m_bVertical)
					xyPos = (m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge) / 2;
				else
					xyPos = (m_rcSplitter.bottom - m_rcSplitter.top - m_cxySplitBar - m_cxyBarEdge) / 2;
			}
		}
#endif 
		// Adjust if out of valid range
		int cxyMax = m_rcSplitter.bottom - m_rcSplitter.top;

		if (xyPos < m_cxyMin + m_cxyBarEdge)
			xyPos = m_cxyMin;
		else if (xyPos > (cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin))
			xyPos = cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin;

		// Set new position and update if requested
		bool bRet = (m_xySplitterPosLR[LR] != xyPos);
		m_xySplitterPosLR[LR] = xyPos;
		m_heightPane1[LR] = m_rcSplitter.bottom - xyPos - (m_gapPane[LR] + m_heightStatusbar);
#if 0
		if (m_bUpdateProportionalPos)
		{
			if (IsProportional())
				StoreProportionalPos();
			else if (IsRightAligned())
				StoreRightAlignPos();
		}
		else
		{
			m_bUpdateProportionalPos = true;
		}
#endif 
		if (bUpdate && bRet)
			UpdateSplitterLayout();

		return bRet;
	}

	bool SetSplitterPos(int xyPos = -1, bool bUpdate = true)
	{
		if (xyPos == -1)   // -1 == default position
		{
			if (m_bProportionalDefPos)
			{
				ATLASSERT((m_xySplitterDefPos >= 0) && (m_xySplitterDefPos <= m_nPropMax));

				xyPos = ::MulDiv(m_xySplitterDefPos, m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge, m_nPropMax);
			}
			else if (m_xySplitterDefPos != -1)
			{
				xyPos = m_xySplitterDefPos;
			}
			else   // not set, use middle position
			{
				xyPos = (m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge) / 2;
			}
		}

		// Adjust if out of valid range
		int	cxyMax = m_rcSplitter.right - m_rcSplitter.left;

		if (xyPos < m_cxyMin + m_cxyBarEdge)
			xyPos = m_cxyMin;
		else if (xyPos > (cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin))
			xyPos = cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin;

		// Set new position and update if requested
		bool bRet = (m_xySplitterPos != xyPos);
		m_xySplitterPos = xyPos;

		if (m_bUpdateProportionalPos)
		{
			if (IsProportional())
				StoreProportionalPos();
			else if (IsRightAligned())
				StoreRightAlignPos();
		}
		else
		{
			m_bUpdateProportionalPos = true;
		}

		if (bUpdate && bRet)
			UpdateSplitterLayout();

		return bRet;
	}

	int GetSplitterPos() const
	{
		return m_xySplitterPos;
	}

	void SetSplitterPosPct(int nPct, bool bUpdate = true)
	{
		ATLASSERT((nPct >= 0) && (nPct <= 100));

		m_nProportionalPos = ::MulDiv(nPct, m_nPropMax, 100);
		UpdateProportionalPos();

		if (bUpdate)
			UpdateSplitterLayout();
	}

	int GetSplitterPosPct() const
	{
		int cxyTotal = (m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge);
		return ((cxyTotal > 0) && (m_xySplitterPos >= 0)) ? ::MulDiv(m_xySplitterPos, 100, cxyTotal) : -1;
	}

	bool SetSinglePaneMode(int nPane = SPLIT_PANE_NONE)
	{
		HWND hWnd;
		ATLASSERT((nPane == SPLIT_PANE_LEFT) || (nPane == SPLIT_PANE_RIGHT) || (nPane == SPLIT_PANE_NONE));
		if (!((nPane == SPLIT_PANE_LEFT) || (nPane == SPLIT_PANE_RIGHT) || (nPane == SPLIT_PANE_NONE)))
			return false;

		if (nPane != SPLIT_PANE_NONE)
		{
			int nOtherPane = (nPane == SPLIT_PANE_LEFT) ? SPLIT_PANE_RIGHT : SPLIT_PANE_LEFT;
			hWnd = m_hWndPane[nPane];
			if (nPane == m_nAIPane)
			{
				int btnId = m_duiViewCT.GetActiveButton();
				ATLASSERT(btnId == XVIEWCT_BUTTON_TABAI || btnId == XVIEWCT_BUTTON_TABKB);
				m_winStatus &= ~(XWIN_TTY | XWIN_EDIT);

				m_winStatus &= ~(XWIN_CHAT | XWIN_TREE);
				m_winStatus |= XWIN_INPUT;
				m_winStatus |= (btnId == XVIEWCT_BUTTON_TABAI) ? XWIN_CHAT : XWIN_TREE;
				if(btnId == XVIEWCT_BUTTON_TABKB)
					hWnd = m_viewTree;
			}
			else
			{
				m_winStatus &= ~(XWIN_CHAT | XWIN_TREE | XWIN_INPUT);
				m_winStatus |= XWIN_TTY;
			}

			if (::IsWindowVisible(hWnd) == FALSE)
				::ShowWindow(hWnd, SW_SHOW);
			
			::ShowWindow(m_hWndPane[nOtherPane], SW_HIDE);
			if (m_nDefActivePane != nPane)
				m_nDefActivePane = nPane;
		}
		else if (m_nSinglePane != SPLIT_PANE_NONE)
		{
			int nOtherPane = (m_nSinglePane == SPLIT_PANE_LEFT) ? SPLIT_PANE_RIGHT : SPLIT_PANE_LEFT;
			hWnd = m_hWndPane[nOtherPane];
			if (nOtherPane == m_nAIPane)
			{
				m_winStatus |= XWIN_INPUT;
				m_winStatus &= ~(XWIN_CHAT | XWIN_TREE);
				int btnId = m_duiViewCT.GetActiveButton();
				ATLASSERT(btnId == XVIEWCT_BUTTON_TABAI || btnId == XVIEWCT_BUTTON_TABKB);
				m_winStatus |= (btnId == XVIEWCT_BUTTON_TABAI) ? XWIN_CHAT : XWIN_TREE;
				if(btnId == XVIEWCT_BUTTON_TABKB)
					hWnd = m_viewTree;
			}
			::ShowWindow(hWnd, SW_SHOW);
		}

		m_nSinglePane = nPane;
		UpdateSplitterLayout();

		return true;
	}

	int GetSinglePaneMode() const
	{
		return m_nSinglePane;
	}

	DWORD GetSplitterExtendedStyle() const
	{
		return m_dwExtendedStyle;
	}

	DWORD SetSplitterExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwExtendedStyle;
		if (dwMask == 0)
			m_dwExtendedStyle = dwExtendedStyle;
		else
			m_dwExtendedStyle = (m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);

#ifdef _DEBUG
		if (IsProportional() && IsRightAligned())
			ATLTRACE2(atlTraceUI, 0, _T("CSplitterImpl::SetSplitterExtendedStyle - SPLIT_PROPORTIONAL and SPLIT_RIGHTALIGNED are mutually exclusive, defaulting to SPLIT_PROPORTIONAL.\n"));
#endif // _DEBUG

		return dwPrevStyle;
	}

	void SetSplitterDefaultPos(int xyPos = -1)
	{
		m_xySplitterDefPos = xyPos;
		m_bProportionalDefPos = false;
	}

	void SetSplitterDefaultPosPct(int nPct)
	{
		ATLASSERT((nPct >= 0) && (nPct <= 100));

		m_xySplitterDefPos = ::MulDiv(nPct, m_nPropMax, 100);
		m_bProportionalDefPos = true;
	}

	// Splitter operations
	void SetSplitterPanes(HWND hWndLeftTop, HWND hWndRightBottom, bool bUpdate = true)
	{
		m_hWndPane[SPLIT_PANE_LEFT] = hWndLeftTop;
		m_hWndPane[SPLIT_PANE_RIGHT] = hWndRightBottom;
		ATLASSERT((m_hWndPane[SPLIT_PANE_LEFT] == NULL) || (m_hWndPane[SPLIT_PANE_RIGHT] == NULL) || (m_hWndPane[SPLIT_PANE_LEFT] != m_hWndPane[SPLIT_PANE_RIGHT]));
		if (bUpdate)
			UpdateSplitterLayout();
	}

	bool SetSplitterPane(int nPane, HWND hWnd, bool bUpdate = true)
	{
		ATLASSERT((nPane == SPLIT_PANE_LEFT) || (nPane == SPLIT_PANE_RIGHT));
		if ((nPane != SPLIT_PANE_LEFT) && (nPane != SPLIT_PANE_RIGHT))
			return false;

		m_hWndPane[nPane] = hWnd;
		ATLASSERT((m_hWndPane[SPLIT_PANE_LEFT] == NULL) || (m_hWndPane[SPLIT_PANE_RIGHT] == NULL) || (m_hWndPane[SPLIT_PANE_LEFT] != m_hWndPane[SPLIT_PANE_RIGHT]));
		if (bUpdate)
			UpdateSplitterLayout();

		return true;
	}

	HWND GetSplitterPane(int nPane) const
	{
		ATLASSERT((nPane == SPLIT_PANE_LEFT) || (nPane == SPLIT_PANE_RIGHT));
		if ((nPane != SPLIT_PANE_LEFT) && (nPane != SPLIT_PANE_RIGHT))
			return NULL;

		return m_hWndPane[nPane];
	}

	bool SetActivePane(int nPane)
	{
		ATLASSERT((nPane == SPLIT_PANE_LEFT) || (nPane == SPLIT_PANE_RIGHT));
		if ((nPane != SPLIT_PANE_LEFT) && (nPane != SPLIT_PANE_RIGHT))
			return false;
		if ((m_nSinglePane != SPLIT_PANE_NONE) && (nPane != m_nSinglePane))
			return false;

		::SetFocus(m_hWndPane[nPane]);
		m_nDefActivePane = nPane;

		return true;
	}

	int GetActivePane() const
	{
		int nRet = SPLIT_PANE_NONE;
		HWND hWndFocus = ::GetFocus();
		if (hWndFocus != NULL)
		{
			for (int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if ((hWndFocus == m_hWndPane[nPane]) || (::IsChild(m_hWndPane[nPane], hWndFocus) != FALSE))
				{
					nRet = nPane;
					break;
				}
			}
		}

		return nRet;
	}

	bool ActivateNextPane(bool bNext = true)
	{
		int nPane = m_nSinglePane;
		if (nPane == SPLIT_PANE_NONE)
		{
			switch (GetActivePane())
			{
			case SPLIT_PANE_LEFT:
				nPane = SPLIT_PANE_RIGHT;
				break;
			case SPLIT_PANE_RIGHT:
				nPane = SPLIT_PANE_LEFT;
				break;
			default:
				nPane = bNext ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;
				break;
			}
		}

		return SetActivePane(nPane);
	}

	bool SetDefaultActivePane(int nPane)
	{
		ATLASSERT((nPane == SPLIT_PANE_LEFT) || (nPane == SPLIT_PANE_RIGHT));
		if ((nPane != SPLIT_PANE_LEFT) && (nPane != SPLIT_PANE_RIGHT))
			return false;

		m_nDefActivePane = nPane;

		return true;
	}

	bool SetDefaultActivePane(HWND hWnd)
	{
		for (int nPane = 0; nPane < m_nPanesCount; nPane++)
		{
			if (hWnd == m_hWndPane[nPane])
			{
				m_nDefActivePane = nPane;
				return true;
			}
		}

		return false;   // not found
	}

	int GetDefaultActivePane() const
	{
		return m_nDefActivePane;
	}
#if 0
	void DrawSplitter(CDCHandle dc)
	{
		ATLASSERT(dc.m_hDC != NULL);
		if ((m_nSinglePane == SPLIT_PANE_NONE) && (m_xySplitterPos == -1))
			return;

		if (m_nSinglePane == SPLIT_PANE_NONE)
		{
			DrawSplitterBar(dc);

			for (int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if (m_hWndPane[nPane] == NULL)
					DrawSplitterPane(dc, nPane);
			}
		}
		else
		{
			if (m_hWndPane[m_nSinglePane] == NULL)
				DrawSplitterPane(dc, m_nSinglePane);
		}
	}

	// call to initiate moving splitter bar with keyboard
	void MoveSplitterBar()
	{
		int x = 0;
		int y = 0;
		if (m_bVertical)
		{
			x = m_xySplitterPos + (m_cxySplitBar / 2) + m_cxyBarEdge;
			y = (m_rcSplitter.bottom - m_rcSplitter.top - m_cxySplitBar - m_cxyBarEdge) / 2;
		}
		else
		{
			x = (m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge) / 2;
			y = m_xySplitterPos + (m_cxySplitBar / 2) + m_cxyBarEdge;
		}

		POINT pt = { x, y };
		ClientToScreen(&pt);
		::SetCursorPos(pt.x, pt.y);

		m_xySplitterPosNew = m_xySplitterPos;
		SetCapture();
		m_hWndFocusSave = SetFocus();
		::SetCursor(m_hCursor);
		if (!m_bFullDrag)
			DrawGhostBar();
		if (m_bVertical)
			m_cxyDragOffset = x - m_rcSplitter.left - m_xySplitterPos;
		else
			m_cxyDragOffset = y - m_rcSplitter.top - m_xySplitterPos;
	}

	void SetOrientation(bool bVertical, bool bUpdate = true)
	{
		if (m_bVertical != bVertical)
		{
			m_bVertical = bVertical;

			m_hCursor = ::LoadCursor(NULL, m_bVertical ? IDC_SIZEWE : IDC_SIZENS);
			m_hCursorNS = ::LoadCursor(NULL, m_bVertical ? IDC_SIZENS : IDC_SIZEWE);

			GetSystemSettings(false);

			if (m_bVertical)
				m_xySplitterPos = ::MulDiv(m_xySplitterPos, m_rcSplitter.right - m_rcSplitter.left, m_rcSplitter.bottom - m_rcSplitter.top);
			else
				m_xySplitterPos = ::MulDiv(m_xySplitterPos, m_rcSplitter.bottom - m_rcSplitter.top, m_rcSplitter.right - m_rcSplitter.left);
		}

		if (bUpdate)
			UpdateSplitterLayout();
	}

	// Overrideables
	void DrawSplitterBar(CDCHandle dc)
	{
		RECT rect = {};
		if (GetSplitterBarRect(&rect))
		{
			dc.FillRect(&rect, COLOR_3DFACE);

			if ((m_dwExtendedStyle & SPLIT_FLATBAR) != 0)
			{
				RECT rect1 = rect;
				if (m_bVertical)
					rect1.right = rect1.left + 1;
				else
					rect1.bottom = rect1.top + 1;
				dc.FillRect(&rect1, COLOR_WINDOW);

				rect1 = rect;
				if (m_bVertical)
					rect1.left = rect1.right - 1;
				else
					rect1.top = rect1.bottom - 1;
				dc.FillRect(&rect1, COLOR_3DSHADOW);
			}
			else if ((m_dwExtendedStyle & SPLIT_GRADIENTBAR) != 0)
			{
				RECT rect2 = rect;
				if (m_bVertical)
					rect2.left = (rect.left + rect.right) / 2 - 1;
				else
					rect2.top = (rect.top + rect.bottom) / 2 - 1;

				dc.GradientFillRect(rect2, ::GetSysColor(COLOR_3DFACE), ::GetSysColor(COLOR_3DSHADOW), m_bVertical);
			}

			// draw 3D edge if needed
			if ((GetExStyle() & WS_EX_CLIENTEDGE) != 0)
				dc.DrawEdge(&rect, EDGE_RAISED, m_bVertical ? (BF_LEFT | BF_RIGHT) : (BF_TOP | BF_BOTTOM));
		}
	}

	// called only if pane is empty
	void DrawSplitterPane(CDCHandle dc, int nPane)
	{
		RECT rect = {};
		if (GetSplitterPaneRect(nPane, &rect))
		{
			if ((GetExStyle() & WS_EX_CLIENTEDGE) == 0)
				dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
			dc.FillRect(&rect, COLOR_APPWORKSPACE);
		}
	}
#endif

	// called only if pane is empty
	void DrawSplitterPaneD2D(int nPane)
	{
#if 0
		RECT rect = {};
		if (GetSplitterPaneRect(nPane, &rect))
		{
			if ((GetExStyle() & WS_EX_CLIENTEDGE) == 0)
				dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
			dc.FillRect(&rect, COLOR_APPWORKSPACE);
		}
#endif 
	}

	void DrawSplitterBarD2D()
	{
		RECT rect = {};
		if (GetSplitterBarRect(&rect))
		{
			if (m_pixelBitmap0)
			{
				D2D1_RECT_F rectF = D2D1::RectF(
					static_cast<FLOAT>(rect.left),
					static_cast<FLOAT>(rect.top),
					static_cast<FLOAT>(rect.right),
					static_cast<FLOAT>(rect.bottom)
				);
				m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap0, &rectF);
			}
#if 0
			dc.FillRect(&rect, COLOR_3DFACE);

			if ((m_dwExtendedStyle & SPLIT_FLATBAR) != 0)
			{
				RECT rect1 = rect;
				if (m_bVertical)
					rect1.right = rect1.left + 1;
				else
					rect1.bottom = rect1.top + 1;
				dc.FillRect(&rect1, COLOR_WINDOW);

				rect1 = rect;
				if (m_bVertical)
					rect1.left = rect1.right - 1;
				else
					rect1.top = rect1.bottom - 1;
				dc.FillRect(&rect1, COLOR_3DSHADOW);
			}
			else if ((m_dwExtendedStyle & SPLIT_GRADIENTBAR) != 0)
			{
				RECT rect2 = rect;
				if (m_bVertical)
					rect2.left = (rect.left + rect.right) / 2 - 1;
				else
					rect2.top = (rect.top + rect.bottom) / 2 - 1;

				dc.GradientFillRect(rect2, ::GetSysColor(COLOR_3DFACE), ::GetSysColor(COLOR_3DSHADOW), m_bVertical);
			}

			// draw 3D edge if needed
			T* pT = static_cast<T*>(this);
			if ((pT->GetExStyle() & WS_EX_CLIENTEDGE) != 0)
				dc.DrawEdge(&rect, EDGE_RAISED, m_bVertical ? (BF_LEFT | BF_RIGHT) : (BF_TOP | BF_BOTTOM));
#endif 
		}
	}

	void DrawSplitterD2D()
	{
		if ((m_nSinglePane == SPLIT_PANE_NONE) && (m_xySplitterPos == -1))
			return;

		if (m_nSinglePane == SPLIT_PANE_NONE)
		{
			DrawSplitterBarD2D();

			for (int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if (m_hWndPane[nPane] == NULL)
				{
					ATLASSERT(0);
					DrawSplitterPaneD2D(nPane);
				}
			}
		}
		else
		{
			if (m_hWndPane[m_nSinglePane] == NULL)
			{
				ATLASSERT(0);
				DrawSplitterPaneD2D(m_nSinglePane);
			}
		}
	}

	int GetFirstIntegralMultipleDeviceScaleFactor() const noexcept
	{
		return static_cast<int>(std::ceil(m_deviceScaleFactor));
	}

	D2D1_SIZE_U GetSizeUFromRect(const RECT& rc, const int scaleFactor) noexcept
	{
		const long width = rc.right - rc.left;
		const long height = rc.bottom - rc.top;
		const UINT32 scaledWidth = width * scaleFactor;
		const UINT32 scaledHeight = height * scaleFactor;
		return D2D1::SizeU(scaledWidth, scaledHeight);
	}

	HRESULT CreateDeviceDependantResource(int left, int top, int right, int bottom)
	{
		U8 result = 0;
		HRESULT hr = S_OK;
		if (nullptr == m_pD2DRenderTarget)  // Create a Direct2D render target.
		{
			RECT rc = { 0 };
			const int integralDeviceScaleFactor = GetFirstIntegralMultipleDeviceScaleFactor();
			D2D1_RENDER_TARGET_PROPERTIES drtp{};
			drtp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
			drtp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
			drtp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
			drtp.dpiX = 96.f * integralDeviceScaleFactor;
			drtp.dpiY = 96.f * integralDeviceScaleFactor;
			// drtp.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN);
			drtp.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);

			rc.left = left; rc.top = top; rc.right = right; rc.bottom = bottom;
			D2D1_HWND_RENDER_TARGET_PROPERTIES dhrtp{};
			dhrtp.hwnd = m_hWnd;
			dhrtp.pixelSize = GetSizeUFromRect(rc, integralDeviceScaleFactor);
			dhrtp.presentOptions = D2D1_PRESENT_OPTIONS_NONE;

			ATLASSERT(nullptr != g_pD2DFactory);

			ReleaseUnknown(m_pixelBitmap0);
			ReleaseUnknown(m_pixelBitmap1);
			//hr = g_pD2DFactory->CreateHwndRenderTarget(renderTargetProperties, 
			// hwndRenderTragetproperties, &m_pD2DRenderTarget);
			hr = g_pD2DFactory->CreateHwndRenderTarget(drtp, dhrtp, &m_pD2DRenderTarget);
			if (hr == S_OK && m_pD2DRenderTarget)
			{
				U32 pixel[4] = { 0xFFAAAAAA, 0xFFEEEEEE,0xFFEEEEEE,0xFFEEEEEE };
				hr = m_pD2DRenderTarget->CreateBitmap(
					D2D1::SizeU(4, 1), pixel, 4 << 2,
					D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
					&m_pixelBitmap0);
				if (hr == S_OK && m_pixelBitmap0)
				{
					pixel[0] = 0xFF056608;
					hr = m_pD2DRenderTarget->CreateBitmap(
						D2D1::SizeU(1, 1), pixel, 4, 
						D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
						&m_pixelBitmap1);
				}
			}
		}
		return hr;
	}

	void ShowLoadingProgress(int left, int top, int right, int bottom)
	{
		int w = right - left;
		int h = bottom - top;

		ATLASSERT(w > 0 && h > 0);

		if (w > 500 && h > 100)
		{
			int offset = m_loadingPercent * 5;
			int L = (w - 500) >> 1;
			int T = (h - 8) >> 1;
			int R = L + 500;
			int B = T + 8;

			D2D1_RECT_F r1 = D2D1::RectF(
				static_cast<FLOAT>(L), 
				static_cast<FLOAT>(T), 
				static_cast<FLOAT>(R), 
				static_cast<FLOAT>(T + 1));
			m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap1, &r1);

			D2D1_RECT_F r2 = D2D1::RectF(
				static_cast<FLOAT>(L), 
				static_cast<FLOAT>(B), 
				static_cast<FLOAT>(R), 
				static_cast<FLOAT>(B + 1));
			m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap1, &r2);

			D2D1_RECT_F r3 = D2D1::RectF(
				static_cast<FLOAT>(L), 
				static_cast<FLOAT>(T), 
				static_cast<FLOAT>(L + 1), 
				static_cast<FLOAT>(B));
			m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap1, &r3);

			D2D1_RECT_F r4 = D2D1::RectF(
				static_cast<FLOAT>(R), 
				static_cast<FLOAT>(T), 
				static_cast<FLOAT>(R + 1), 
				static_cast<FLOAT>(B));
			m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap1, &r4);

			D2D1_RECT_F r5 = D2D1::RectF(
				static_cast<FLOAT>(L), 
				static_cast<FLOAT>(T), 
				static_cast<FLOAT>(L + offset), 
				static_cast<FLOAT>(B));
			m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap1, &r5);
		}
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		HRESULT hr = S_OK;
		PAINTSTRUCT ps;
		BeginPaint(&ps);

		RECT rc;
		GetClientRect(&rc);

		// try setting position if not set
		if ((m_nSinglePane == SPLIT_PANE_NONE) && (m_xySplitterPos == -1))
			SetSplitterPos();

		hr = CreateDeviceDependantResource(rc.left, rc.top, rc.right, rc.bottom);
		if (S_OK == hr && nullptr != m_pD2DRenderTarget)
		{
			m_pD2DRenderTarget->BeginDraw();
			////////////////////////////////////////////////////////////////////////////////////////////////////
			m_pD2DRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			if (m_loadingPercent > 100)
			{
				m_duiViewSB.Draw(m_pD2DRenderTarget);
				m_duiViewCG.Draw(m_pD2DRenderTarget);
				m_duiViewCT.Draw(m_pD2DRenderTarget);
				m_duiViewEG.Draw(m_pD2DRenderTarget);
				DrawSplitterD2D();
			}
			else
			{
				m_pD2DRenderTarget->Clear(D2D1::ColorF(0xF0F0F0));
				ShowLoadingProgress(rc.left, rc.top, rc.right, rc.bottom);
			}

			hr = m_pD2DRenderTarget->EndDraw();
			////////////////////////////////////////////////////////////////////////////////////////////////////
			if (FAILED(hr) || D2DERR_RECREATE_TARGET == hr)
			{
				ReleaseUnknown(m_pD2DRenderTarget);
			}
		}

		EndPaint(&ps);
		return 0;
	}

#if 0
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);

		// try setting position if not set
		if ((m_nSinglePane == SPLIT_PANE_NONE) && (m_xySplitterPos == -1))
			pT->SetSplitterPos();

		// do painting
		if (wParam != NULL)
		{
			pT->DrawSplitter((HDC)wParam);
		}
		else
		{
			CPaintDC dc(pT->m_hWnd);
			pT->DrawSplitter(dc.m_hDC);
		}

		return 0;
	}
#endif 

	LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = DUIWindowCursorIsSet() ? TRUE : FALSE;
		ClearDUIWindowCursor();

		if (m_LRX >= 0)
		{
			ATLASSERT(m_LRX == SPLIT_PANE_LEFT || m_LRX == SPLIT_PANE_RIGHT || m_LRX == SPLIT_PANE_VERT);
			bHandled = true;
			m_LRX = -1;
			return 1;
		}

		if (((HWND)wParam == m_hWnd) && (LOWORD(lParam) == HTCLIENT))
		{
			DWORD dwPos = ::GetMessagePos();
			POINT ptPos = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
			ScreenToClient(&ptPos);
			if (IsOverSplitterBar(ptPos.x, ptPos.y))
				return 1;
		}
		//bHandled = FALSE;
		return 0;
	}

	bool AdjustXViewPos(int lrx)
	{
		int idx, nOtherPane;
		bool bRet = true;
		U32 size;
		U32* dst = m_screenFixed;
		RECT rcPane = {};
		XRECT area = { 0 };
		XRECT* r = &area;

		ATLASSERT(dst);
		ATLASSERT(m_nAIPane == SPLIT_PANE_LEFT || m_nAIPane == SPLIT_PANE_RIGHT);
		nOtherPane = (m_nAIPane == SPLIT_PANE_RIGHT) ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;

		idx = m_nAIPane;
		if (GetSplitterPaneRect(idx, &rcPane))
		{
			ATLASSERT(m_tabPane[idx] == WINTAB_HEIGHT);
			r->left = rcPane.left;
			r->right = rcPane.right;
			r->top = rcPane.top;
			r->bottom = r->top + m_tabPane[idx];
			m_duiViewCT.UpdateSize(r, dst);
			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			dst += size;

			ATLASSERT(m_gapPane[idx] == AICHAT_GAP);
			r->bottom = rcPane.bottom - (m_heightStatusbar + m_heightPane1[idx]);
			r->top = r->bottom - m_gapPane[idx];
			m_duiViewCG.UpdateSize(r, dst);
			m_duiViewCG.SetWindowShow(XWIN_INPUT & m_winStatus);

			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			dst += size;
		}

		idx = nOtherPane;
		if (GetSplitterPaneRect(idx, &rcPane))
		{
			r->left = rcPane.left;
			r->right = rcPane.right;
			r->bottom = rcPane.bottom - (m_heightStatusbar + m_heightPane1[idx]);
			r->top = r->bottom - m_gapPane[idx];
			m_duiViewEG.UpdateSize(r, dst);
			m_duiViewEG.SetWindowShow(XWIN_EDIT & m_winStatus);
		}
		return bRet;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		bool refreshScreen = false;
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if (::GetCapture() == m_hWnd)
		{
			if (m_LRX == SPLIT_PANE_VERT)
			{
				int xyNewSplitPos = xPos - m_rcSplitter.left - m_cxyDragOffset;

				if (xyNewSplitPos == -1)   // avoid -1, that means default position
					xyNewSplitPos = -2;

				if (m_xySplitterPos != xyNewSplitPos)
				{
					if (m_bFullDrag)
					{
						if (SetSplitterPos(xyNewSplitPos, true))
							UpdateWindow();
					}
#if 0
					else
					{

						DrawGhostBar();
						SetSplitterPos(xyNewSplitPos, false);
						DrawGhostBar();
					}
#endif 
					refreshScreen = AdjustXViewPos(m_LRX);
				}
			}
			else if (m_LRX == SPLIT_PANE_LEFT || m_LRX == SPLIT_PANE_RIGHT)
			{
				int xyNewSplitPos = yPos - m_rcSplitter.top - m_cxyDragOffsetLR[m_LRX];

				if (xyNewSplitPos == -1)   // avoid -1, that means default position
					xyNewSplitPos = -2;

				if (m_xySplitterPosLR[m_LRX] != xyNewSplitPos)
				{
					m_heightPane1[m_LRX] = xyNewSplitPos;
					ATLASSERT(m_bFullDrag);
					if (m_bFullDrag)
					{
						if (SetSplitterPosLR(m_LRX, xyNewSplitPos, true))
							UpdateWindow();
					}
					refreshScreen = AdjustXViewPos(m_LRX);
				}
			}
			else
			{
				ATLASSERT(0);
				m_LRX = -1;
			}
		}
		else		// not dragging, just set cursor
		{
			m_LRX = -1;
			if (IsOverSplitterBar(xPos, yPos))
			{
				m_LRX = SPLIT_PANE_VERT;
				ATLASSERT(m_hCursor);
				::SetCursor(m_hCursor);
			}
			else if (IsOverGapBar(xPos, yPos, m_LRX))
			{
				ATLASSERT(m_LRX == SPLIT_PANE_LEFT || m_LRX == SPLIT_PANE_RIGHT);
				ATLASSERT(m_hCursorNS);
				::SetCursor(m_hCursorNS);
			}
			//bHandled = FALSE;
		}

		if (refreshScreen)
			Invalidate();

		return 0;
	}

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int LR = -1;
	
		if (::GetCapture() != m_hWnd)
		{
			if (IsOverSplitterBar(xPos, yPos)) // the mouse is over the vertical split line
			{
				m_xySplitterPosNew = m_xySplitterPos;
				SetCapture();
				m_hWndFocusSave = SetFocus();
				::SetCursor(m_hCursor);
				m_LRX = SPLIT_PANE_VERT;
#if 0
				if (!m_bFullDrag)
					DrawGhostBar();
#endif
				m_cxyDragOffset = xPos - m_rcSplitter.left - m_xySplitterPos;
			}
			else if (IsOverGapBar(xPos, yPos, m_LRX)) // the mouse is hit the gap line
			{
				ATLASSERT(m_LRX == SPLIT_PANE_LEFT || m_LRX == SPLIT_PANE_RIGHT);
				m_xySplitterPosLRNew[m_LRX] = m_xySplitterPosLR[m_LRX];
				SetCapture();
				m_hWndFocusSave = SetFocus();
				::SetCursor(m_hCursorNS);
				m_cxyDragOffsetLR[m_LRX] = yPos - m_rcSplitter.top - m_xySplitterPosLR[m_LRX];
			}
		}
		else if ((::GetCapture() == m_hWnd) && !IsOverSplitterBar(xPos, yPos) && !IsOverGapBar(xPos, yPos, LR))
		{
			::ReleaseCapture();
			m_LRX = -1;
		}

		return 1;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (::GetCapture() == m_hWnd)
		{
			switch(m_LRX)
			{
			case SPLIT_PANE_LEFT:
			case SPLIT_PANE_RIGHT:
				m_xySplitterPosLRNew[m_LRX] = m_xySplitterPosLR[m_LRX];
				break;
			case SPLIT_PANE_VERT:
				m_xySplitterPosNew = m_xySplitterPos;
				break;
			default:
				break;
			}
			m_LRX = -1;
			::ReleaseCapture();
		}

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnLButtonDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
#if 0
		SetSplitterPos();   // default
#endif 
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
#if 0
		if (!m_bFullDrag)
			DrawGhostBar();
#endif 
		if ((m_xySplitterPosNew != -1) && (!m_bFullDrag || (m_xySplitterPos != m_xySplitterPosNew)))
		{
			m_xySplitterPos = m_xySplitterPosNew;
			m_xySplitterPosNew = -1;
			UpdateSplitterLayout();
			UpdateWindow();
		}

		if (m_hWndFocusSave != NULL)
			::SetFocus(m_hWndFocusSave);

		return 0;
	}

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (::GetCapture() == m_hWnd)
		{
			switch (wParam)
			{
			case VK_RETURN:
				m_xySplitterPosNew = m_xySplitterPos;
				// FALLTHROUGH
			case VK_ESCAPE:
				::ReleaseCapture();
				break;
			case VK_LEFT:
			case VK_RIGHT:
				{
					POINT pt = {};
					::GetCursorPos(&pt);
					int xyPos = m_xySplitterPos + ((wParam == VK_LEFT) ? -m_cxyStep : m_cxyStep);
					int cxyMax = m_rcSplitter.right - m_rcSplitter.left;
					if (xyPos < (m_cxyMin + m_cxyBarEdge))
						xyPos = m_cxyMin;
					else if (xyPos > (cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin))
						xyPos = cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin;
					pt.x += xyPos - m_xySplitterPos;
					::SetCursorPos(pt.x, pt.y);
				}
				break;
#if 0
			case VK_UP:
			case VK_DOWN:
				if (!m_bVertical)
				{
					POINT pt = {};
					::GetCursorPos(&pt);
					int xyPos = m_xySplitterPos + ((wParam == VK_UP) ? -m_cxyStep : m_cxyStep);
					int cxyMax = m_rcSplitter.bottom - m_rcSplitter.top;
					if (xyPos < (m_cxyMin + m_cxyBarEdge))
						xyPos = m_cxyMin;
					else if (xyPos > (cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin))
						xyPos = cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin;
					pt.y += xyPos - m_xySplitterPos;
					::SetCursorPos(pt.x, pt.y);
				}
				break;
#endif 
			default:
				break;
			}
		}
		else
		{
			bHandled = FALSE;
		}

		return 0;
	}

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM, BOOL& bHandled)
	{
		if (::GetCapture() != m_hWnd)
		{
			if (m_nSinglePane == SPLIT_PANE_NONE)
			{
				if ((m_nDefActivePane == SPLIT_PANE_LEFT) || (m_nDefActivePane == SPLIT_PANE_RIGHT))
					::SetFocus(m_hWndPane[m_nDefActivePane]);
			}
			else
			{
				::SetFocus(m_hWndPane[m_nSinglePane]);
			}
		}

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
		if ((lRet == MA_ACTIVATE) || (lRet == MA_ACTIVATEANDEAT))
		{
			DWORD dwPos = ::GetMessagePos();
			POINT pt = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
			ScreenToClient(&pt);
			RECT rcPane = {};
			for (int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if (GetSplitterPaneRect(nPane, &rcPane) && (::PtInRect(&rcPane, pt) != FALSE))
				{
					m_nDefActivePane = nPane;
					break;
				}
			}
		}

		return lRet;
	}

	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		GetSystemSettings(true);

		return 0;
	}

	// Implementation - internal helpers
	void Init()
	{
		m_hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
		m_hCursorNS = ::LoadCursor(NULL, IDC_SIZENS);

		ATLASSERT(m_hCursor);
		ATLASSERT(m_hCursorNS);

		GetSystemSettings(false);
	}

#if 0
	void UpdateSplitterLayout()
	{
		if ((m_nSinglePane == SPLIT_PANE_NONE) && (m_xySplitterPos == -1))
			return;

		T* pT = static_cast<T*>(this);
		RECT rect = {};
		if (m_nSinglePane == SPLIT_PANE_NONE)
		{
			if (GetSplitterBarRect(&rect))
				pT->InvalidateRect(&rect);

			for (int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if (GetSplitterPaneRect(nPane, &rect))
				{
					if (m_hWndPane[nPane] != NULL)
						::SetWindowPos(m_hWndPane[nPane], NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
					else
						pT->InvalidateRect(&rect);
				}
			}
		}
		else
		{
			if (GetSplitterPaneRect(m_nSinglePane, &rect))
			{
				if (m_hWndPane[m_nSinglePane] != NULL)
					::SetWindowPos(m_hWndPane[m_nSinglePane], NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
				else
					pT->InvalidateRect(&rect);
			}
		}
	}
#endif 

	void UpdateDUILayout()
	{
		U32* dst = m_screenFixed;
		if (dst)
		{
			int idx;
			U32 size;
			XRECT area = { 0 };
			XRECT* r = &area;
			RECT rcPane = {};
			int nOtherPane = (m_nAIPane == SPLIT_PANE_RIGHT) ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;

			ATLASSERT(m_nAIPane == SPLIT_PANE_LEFT || m_nAIPane == SPLIT_PANE_RIGHT);

			idx = m_nAIPane;
			if (GetSplitterPaneRect(idx, &rcPane))
			{
				ATLASSERT(m_tabPane[idx] == WINTAB_HEIGHT);
				r->left = rcPane.left;
				r->right = rcPane.right;
				r->top = rcPane.top;
				r->bottom = r->top + m_tabPane[idx];
				m_duiViewCT.UpdateSize(r, dst);
				size = (U32)((r->right - r->left) * (r->bottom - r->top));
				dst += size;

				ATLASSERT(m_gapPane[idx] == AICHAT_GAP);
				r->bottom = rcPane.bottom - (m_heightStatusbar + m_heightPane1[idx]);
				r->top = r->bottom - m_gapPane[idx];
				m_duiViewCG.UpdateSize(r, dst);
				m_duiViewCG.SetWindowShow(XWIN_INPUT & m_winStatus);

				size = (U32)((r->right - r->left) * (r->bottom - r->top));
				dst += size;
			}

			idx = nOtherPane;
			if (GetSplitterPaneRect(idx, &rcPane))
			{
				r->left = rcPane.left;
				r->right = rcPane.right;
				r->bottom = rcPane.bottom - (m_heightStatusbar + m_heightPane1[idx]);
				r->top = r->bottom - m_gapPane[idx];
				m_duiViewEG.UpdateSize(r, dst);
				m_duiViewEG.SetWindowShow(XWIN_EDIT & m_winStatus);
			}
			Invalidate();
		}
	}

	void UpdateSplitterLayout()
	{
		int top, bottom, winHeight, gap;
		bool bShow;
		int nCmdShow = SW_SHOW;
		HWND hWnd;
		RECT rect = {};

		if ((m_nSinglePane == SPLIT_PANE_NONE) && (m_xySplitterPos == -1))
			return;
		
		if (m_nSinglePane == SPLIT_PANE_NONE)
		{
			if (GetSplitterBarRect(&rect))
				InvalidateRect(&rect);

			for (int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if (GetSplitterPaneRect(nPane, &rect))
				{
					hWnd = m_hWndPane[nPane];
					winHeight = 0;
					gap = 0;
					bShow = false;
					if (nPane == m_nAIPane)
					{
						if (XWIN_TREE & m_winStatus) // chat window and tree window cannot be showing in the same time
						{
							ATLASSERT((XWIN_CHAT & m_winStatus) == 0);
							hWnd = m_viewTree;
							m_viewTree.ShowWindow(SW_SHOW);
							m_viewOutput.ShowWindow(SW_HIDE);
						}
						else
						{
							if(XWIN_CHAT & m_winStatus)
								m_viewOutput.ShowWindow(SW_SHOW);
							m_viewTree.ShowWindow(SW_HIDE);
							ATLASSERT(hWnd == m_viewOutput);
						}

						if (XWIN_INPUT & m_winStatus)
						{
							winHeight = m_heightPane1[nPane];
							gap = m_gapPane[nPane];
							bShow = true;
						}
					}
					else if (nPane != m_nAIPane && (XWIN_EDIT & m_winStatus))
					{
						winHeight = m_heightPane1[nPane];
						gap = m_gapPane[nPane];
						bShow = true;
					}

					top = rect.top + m_tabPane[nPane];
					bottom = rect.bottom - (m_heightStatusbar + winHeight + gap);
					if (bottom < rect.bottom)
					{
						m_xySplitterPosLR[nPane] = bottom;
					}
					else
					{
						m_xySplitterPosLR[nPane] = -1;
					}

					if (hWnd != NULL)
						::SetWindowPos(hWnd, NULL,
							rect.left,
							top,
							rect.right - rect.left,
							bottom - top,
							SWP_NOZORDER);
					else
						InvalidateRect(&rect);

					ATLASSERT(::IsWindow(m_hWndPane1[nPane]));
					if(bShow)
					{
						top = bottom + m_gapPane[nPane];
						bottom = rect.bottom - m_heightStatusbar;
						::SetWindowPos(m_hWndPane1[nPane], NULL,
							rect.left,
							top,
							rect.right - rect.left,
							bottom - top,
							SWP_NOZORDER);

						::ShowWindow(m_hWndPane1[nPane], (m_loadingPercent > 100)? SW_SHOW: SW_HIDE);
					}
					else
					{
						::ShowWindow(m_hWndPane1[nPane], SW_HIDE);
					}
				}
			}
		}
		else
		{
			ATLASSERT(m_nSinglePane == SPLIT_PANE_LEFT || m_nSinglePane == SPLIT_PANE_RIGHT);
			if (GetSplitterPaneRect(m_nSinglePane, &rect))
			{
				int nOtherPane = (m_nSinglePane == SPLIT_PANE_LEFT) ? SPLIT_PANE_RIGHT : SPLIT_PANE_LEFT;
				hWnd = m_hWndPane[m_nSinglePane];
				bShow = false;
				winHeight = 0;
				gap = 0;
				if (m_nSinglePane == m_nAIPane)
				{
					if (XWIN_TREE & m_winStatus) // chat window and tree window cannot be showing in the same time
					{
						ATLASSERT((XWIN_CHAT & m_winStatus) == 0);
						hWnd = m_viewTree;
					}
#ifdef _DEBUG
					else
					{
						ATLASSERT(hWnd == m_viewOutput);
					}
#endif

					if (XWIN_INPUT & m_winStatus)
					{
						winHeight = m_heightPane1[m_nSinglePane];
						gap = m_gapPane[m_nSinglePane];
						bShow = true;
					}
				}
				else if (m_nSinglePane != m_nAIPane && (XWIN_EDIT & m_winStatus))
				{
					winHeight = m_heightPane1[m_nSinglePane];
					gap = m_gapPane[m_nSinglePane];
					bShow = true;
				}

				top = rect.top + m_tabPane[m_nSinglePane];
				bottom = rect.bottom - (m_heightStatusbar + winHeight + gap);
				if (bottom < rect.bottom)
				{
					m_xySplitterPosLR[m_nSinglePane] = bottom;
				}
				else
				{
					m_xySplitterPosLR[m_nSinglePane] = -1;
				}

				ATLASSERT(::IsWindow(hWnd));
				if (TRUE) //m_hWndPane[m_nSinglePane] != NULL)
				{
					::SetWindowPos(hWnd, NULL,
						rect.left,
						top,
						rect.right - rect.left,
						bottom - top,
						SWP_NOZORDER);

					if (bShow)
					{
						ATLASSERT(rect.bottom > m_heightStatusbar);
						top = bottom + m_gapPane[m_nSinglePane];
						bottom = rect.bottom - m_heightStatusbar;
						::SetWindowPos(m_hWndPane1[m_nSinglePane], NULL,
							rect.left,
							top,
							rect.right - rect.left,
							bottom - top,
							SWP_NOZORDER);

						::ShowWindow(m_hWndPane1[m_nSinglePane], (m_loadingPercent > 100) ? SW_SHOW : SW_HIDE);
					}
					else
					{
						::ShowWindow(m_hWndPane1[m_nSinglePane], SW_HIDE);
					}
					::ShowWindow(m_hWndPane[nOtherPane], SW_HIDE);
					::ShowWindow(m_hWndPane1[nOtherPane], SW_HIDE);
				}
				else
					InvalidateRect(&rect);
			}
		}

		UpdateDUILayout();
	}

	bool GetSplitterBarRect(LPRECT lpRect) const
	{
		ATLASSERT(lpRect != NULL);
		if ((m_nSinglePane != SPLIT_PANE_NONE) || (m_xySplitterPos == -1))
			return false;

		lpRect->left = m_rcSplitter.left + m_xySplitterPos;
		lpRect->top = m_rcSplitter.top + m_heightToolbar;
		lpRect->right = m_rcSplitter.left + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
		lpRect->bottom = m_rcSplitter.bottom - m_heightStatusbar;

		return true;
	}

	bool GetSplitterPaneRect(int nPane, LPRECT lpRect) const
	{
		ATLASSERT((nPane == SPLIT_PANE_LEFT) || (nPane == SPLIT_PANE_RIGHT));
		ATLASSERT(lpRect != NULL);
		bool bRet = true;
		if (m_nSinglePane != SPLIT_PANE_NONE)
		{
			if (nPane == m_nSinglePane)
				*lpRect = m_rcSplitter;
			else
				bRet = false;
		}
		else if (nPane == SPLIT_PANE_LEFT)
		{
			lpRect->left = m_rcSplitter.left;
			lpRect->top = m_rcSplitter.top;
			lpRect->right = m_rcSplitter.left + m_xySplitterPos;
			lpRect->bottom = m_rcSplitter.bottom;
		}
		else if (nPane == SPLIT_PANE_RIGHT)
		{
			lpRect->left = m_rcSplitter.left + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
			lpRect->top = m_rcSplitter.top;
			lpRect->right = m_rcSplitter.right;
			lpRect->bottom = m_rcSplitter.bottom;
		}
		else
		{
			bRet = false;
		}

		return bRet;
	}

	bool IsOverSplitterRect(int x, int y) const
	{
		// -1 == don't check
		return (((x == -1) || ((x >= m_rcSplitter.left) && (x <= m_rcSplitter.right))) &&
			((y == -1) || ((y >= (m_rcSplitter.top + m_heightToolbar)) && (y <= (m_rcSplitter.bottom - m_heightStatusbar)))));
	}

	bool IsOverSplitterBar(int x, int y) const
	{
		if (m_nSinglePane != SPLIT_PANE_NONE)
			return false;
		if ((m_xySplitterPos == -1) || !IsOverSplitterRect(x, y))
			return false;
		int xy = x;
		int xyOff = m_rcSplitter.left;

		return ((xy >= (xyOff + m_xySplitterPos)) && (xy < (xyOff + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge)));
	}

	bool IsOverGapBar(int x, int y, int& LR) const
	{
		LR = -1;
		if (m_xySplitterPos >= m_rcSplitter.left && m_xySplitterPos < m_rcSplitter.right)
		{
			int Y, idx, winHeight, nOtherPane;
			if (m_nSinglePane == SPLIT_PANE_NONE)
			{
				nOtherPane = (m_nAIPane == SPLIT_PANE_RIGHT) ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;
				idx = SPLIT_PANE_LEFT;
				if (x >= m_rcSplitter.left && x < m_xySplitterPos)
				{
					winHeight = 0;
					if (nOtherPane == SPLIT_PANE_LEFT && (XWIN_EDIT & m_winStatus)) winHeight = m_heightPane1[idx];
					if (nOtherPane == SPLIT_PANE_RIGHT && (XWIN_INPUT & m_winStatus)) winHeight = m_heightPane1[idx];

					ATLASSERT(m_rcSplitter.bottom > (m_heightStatusbar + winHeight + m_gapPane[idx]));
					Y = m_rcSplitter.bottom - (m_heightStatusbar + winHeight + m_gapPane[idx]);
					if (Y < m_rcSplitter.bottom && y >= Y && y < Y + m_cxySplitBar)
					{
						LR = SPLIT_PANE_LEFT;
						return true;
					}
					Y = m_rcSplitter.bottom - (m_heightStatusbar + winHeight);
					if (Y < m_rcSplitter.bottom && y >= Y - m_cxySplitBar && y < Y)
					{
						LR = SPLIT_PANE_LEFT;
						return true;
					}
				}
				idx = SPLIT_PANE_RIGHT;
				if (x >= m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge && x < m_rcSplitter.right)
				{
					winHeight = 0;
					if (nOtherPane == SPLIT_PANE_LEFT && (XWIN_INPUT & m_winStatus)) winHeight = m_heightPane1[idx];
					if (nOtherPane == SPLIT_PANE_RIGHT && (XWIN_EDIT & m_winStatus)) winHeight = m_heightPane1[idx];

					ATLASSERT(m_rcSplitter.bottom > (m_heightStatusbar + winHeight + m_gapPane[idx]));
					Y = m_rcSplitter.bottom - (m_heightStatusbar + winHeight + m_gapPane[idx]);
					if (Y < m_rcSplitter.bottom && y >= Y && y < Y + m_cxySplitBar)
					{
						LR = SPLIT_PANE_RIGHT;
						return true;
					}
					Y = m_rcSplitter.bottom - (m_heightStatusbar + winHeight);
					if (Y < m_rcSplitter.bottom && y >= Y - m_cxySplitBar && y < Y)
					{
						LR = SPLIT_PANE_RIGHT;
						return true;
					}
				}
			}
			else
			{
				ATLASSERT(m_nSinglePane == SPLIT_PANE_LEFT || m_nSinglePane == SPLIT_PANE_RIGHT);
				idx = m_nSinglePane;
				if (x >= m_rcSplitter.left && x < m_rcSplitter.right)
				{
					winHeight = 0;
					if (m_nSinglePane == m_nAIPane && (XWIN_INPUT & m_winStatus)) winHeight = m_heightPane1[idx];
					if (m_nSinglePane != m_nAIPane && (XWIN_EDIT & m_winStatus)) winHeight = m_heightPane1[idx];

					ATLASSERT(m_rcSplitter.bottom > (m_heightStatusbar + winHeight + m_gapPane[idx]));
					Y = m_rcSplitter.bottom - (m_heightStatusbar + winHeight + m_gapPane[idx]);
					if (Y < m_rcSplitter.bottom && y >= Y && y < Y + m_cxySplitBar)
					{
						LR = m_nSinglePane;
						return true;
					}
					Y = m_rcSplitter.bottom - (m_heightStatusbar + winHeight);
					if (Y < m_rcSplitter.bottom && y >= Y - m_cxySplitBar && y < Y)
					{
						LR = m_nSinglePane;
						return true;
					}
				}
			}
		}
		return false;
	}

#if 0
	void DrawGhostBar()
	{
		RECT rect = {};
		if (GetSplitterBarRect(&rect))
		{
			// convert client to window coordinates
			RECT rcWnd = {};
			GetWindowRect(&rcWnd);
			::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rcWnd, 2);
			::OffsetRect(&rect, -rcWnd.left, -rcWnd.top);

			// invert the brush pattern (looks just like frame window sizing)
			CBrush brush(CDCHandle::GetHalftoneBrush());
			if (brush.m_hBrush != NULL)
			{
				CWindowDC dc(m_hWnd);
				CBrushHandle brushOld = dc.SelectBrush(brush);
				dc.PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);
				dc.SelectBrush(brushOld);
			}
		}
	}
#endif 
	void GetSystemSettings(bool bUpdate)
	{
		if ((m_dwExtendedStyle & SPLIT_FIXEDBARSIZE) == 0)
		{
			m_cxySplitBar = ::GetSystemMetrics(SM_CXSIZEFRAME);
		}

		if ((GetExStyle() & WS_EX_CLIENTEDGE) != 0)
		{
			m_cxyBarEdge = 2 * ::GetSystemMetrics(SM_CXEDGE);
			m_cxyMin = 0;
		}
		else
		{
			m_cxyBarEdge = 0;
			m_cxyMin = 2 * ::GetSystemMetrics(SM_CXEDGE);
		}

		::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &m_bFullDrag, 0);
		m_bFullDrag = true; // Wilson

		if (bUpdate)
			UpdateSplitterLayout();
	}

	bool IsProportional() const
	{
		return ((m_dwExtendedStyle & SPLIT_PROPORTIONAL) != 0);
	}

	void StoreProportionalPos()
	{
		int cxyTotal = (m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge);
		if (cxyTotal > 0)
			m_nProportionalPos = ::MulDiv(m_xySplitterPos, m_nPropMax, cxyTotal);
		else
			m_nProportionalPos = 0;
		ATLTRACE2(atlTraceUI, 0, _T("CSplitterImpl::StoreProportionalPos - %i\n"), m_nProportionalPos);
	}

	void UpdateProportionalPos()
	{
		int cxyTotal = (m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge);
		if (cxyTotal > 0)
		{
			int xyNewPos = ::MulDiv(m_nProportionalPos, cxyTotal, m_nPropMax);
			m_bUpdateProportionalPos = false;
			SetSplitterPos(xyNewPos, false);
		}
	}

	bool IsRightAligned() const
	{
		return ((m_dwExtendedStyle & SPLIT_RIGHTALIGNED) != 0);
	}

	void StoreRightAlignPos()
	{
		int cxyTotal = (m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge);
		if (cxyTotal > 0)
			m_nProportionalPos = cxyTotal - m_xySplitterPos;
		else
			m_nProportionalPos = 0;
		ATLTRACE2(atlTraceUI, 0, _T("CSplitterImpl::StoreRightAlignPos - %i\n"), m_nProportionalPos);
	}

	void UpdateRightAlignPos()
	{
		int cxyTotal = (m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge);
		if (cxyTotal > 0)
		{
			m_bUpdateProportionalPos = false;
			SetSplitterPos(cxyTotal - m_nProportionalPos, false);
		}
	}

	bool IsInteractive() const
	{
		return ((m_dwExtendedStyle & SPLIT_NONINTERACTIVE) == 0);
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// handled, no background painting needed
		return 1;
	}

	void DoNotePadWindow()
	{
		int nOtherPane = (m_nAIPane == SPLIT_PANE_RIGHT) ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;
		bool notepadWindowIsAvailable = (XWIN_EDIT & m_winStatus);

		if (notepadWindowIsAvailable) // we need to hide it
		{
			m_winStatus &= ~XWIN_EDIT;
		}
		else  // we need to show it.
		{
			m_winStatus |= XWIN_EDIT;
		}
		UpdateSplitterLayout();
	}

	bool DoTTYWindow()
	{
		bool ttyWindowIsAvailable = (m_winStatus & XWIN_TTY);
		if (ttyWindowIsAvailable) // we need to hide it.
		{
			m_winStatus &= ~XWIN_TTY;
			SetSinglePaneMode(m_nAIPane);
			m_duiViewCG.SetAIPos(m_nAIPane, false);
		}
		else // we need to show it.
		{
			m_winStatus |= XWIN_TTY;
			SetSinglePaneMode(SPLIT_PANE_NONE);
			m_duiViewCG.SetAIPos(m_nAIPane, true);
		}

		return ttyWindowIsAvailable;
	}

	bool DoAIChatWindow()
	{
		U32 idx, nOtherPane;
		bool chatWindowIsAvailable = (m_winStatus & XWIN_CHAT) || (m_winStatus & XWIN_TREE);

		nOtherPane = (m_nAIPane == SPLIT_PANE_RIGHT) ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;
		if (chatWindowIsAvailable && (XWIN_TTY & m_winStatus)) // we need to hide AI window
		{
			SetSinglePaneMode(nOtherPane);
		}
		else // we need to show AI window
		{
			SetSinglePaneMode(SPLIT_PANE_NONE);
		}
		return chatWindowIsAvailable;
	}

	void DoSwitchLayout()
	{
		int w = m_rcSplitter.right - m_rcSplitter.left;
		ATLASSERT(m_nAIPane == SPLIT_PANE_RIGHT || m_nAIPane == SPLIT_PANE_LEFT);

		if ((m_nSinglePane == SPLIT_PANE_NONE) && (m_xySplitterPos > 0) && (m_xySplitterPos < w))
		{
			if (::IsWindow(m_hWndPane[SPLIT_PANE_LEFT]) && ::IsWindow(m_hWndPane[SPLIT_PANE_RIGHT]))
			{
				HWND hWndTemp;
				int  iTemp;
				int cxyTotal = (m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge);

				m_xySplitterPos = w - m_xySplitterPos - m_cxySplitBar - m_cxyBarEdge;
				if (cxyTotal > 0)
				{
					m_nProportionalPos = ::MulDiv(m_xySplitterPos, m_nPropMax, cxyTotal);
				}
				else
				{
					ATLASSERT(0);
					m_nProportionalPos = 0;
				}

				hWndTemp = m_hWndPane[SPLIT_PANE_LEFT];
				m_hWndPane[SPLIT_PANE_LEFT] = m_hWndPane[SPLIT_PANE_RIGHT];
				m_hWndPane[SPLIT_PANE_RIGHT] = hWndTemp;

				hWndTemp = m_hWndPane1[SPLIT_PANE_LEFT];
				m_hWndPane1[SPLIT_PANE_LEFT] = m_hWndPane1[SPLIT_PANE_RIGHT];
				m_hWndPane1[SPLIT_PANE_RIGHT] = hWndTemp;

				iTemp = m_tabPane[SPLIT_PANE_LEFT];
				m_tabPane[SPLIT_PANE_LEFT] = m_tabPane[SPLIT_PANE_RIGHT];
				m_tabPane[SPLIT_PANE_RIGHT] = iTemp;

				iTemp = m_heightPane1[SPLIT_PANE_LEFT];
				m_heightPane1[SPLIT_PANE_LEFT] = m_heightPane1[SPLIT_PANE_RIGHT];
				m_heightPane1[SPLIT_PANE_RIGHT] = iTemp;

				iTemp = m_gapPane[SPLIT_PANE_LEFT];
				m_gapPane[SPLIT_PANE_LEFT] = m_gapPane[SPLIT_PANE_RIGHT];
				m_gapPane[SPLIT_PANE_RIGHT] = iTemp;

				if (m_nAIPane == SPLIT_PANE_RIGHT)
					m_nAIPane = SPLIT_PANE_LEFT;
				else
					m_nAIPane = SPLIT_PANE_RIGHT;

				m_duiViewCG.SetAIPos(m_nAIPane, true);

				UpdateSplitterLayout();
				m_viewOutput.ScrollToBottom();
				m_viewOutput.ShowWindow(SW_HIDE);
				if(XWIN_CHAT & m_winStatus)
					m_viewOutput.ShowWindow(SW_SHOW);
				m_viewInput.ShowWindow(SW_HIDE);
				if (XWIN_INPUT & m_winStatus)
					m_viewInput.ShowWindow(SW_SHOW);

				m_viewTree.ShowWindow(SW_HIDE);
				if (XWIN_TREE & m_winStatus)
					m_viewTree.ShowWindow(SW_SHOW);

				::InvalidateRect(m_hWndPane[SPLIT_PANE_LEFT], NULL, TRUE);
				::InvalidateRect(m_hWndPane[SPLIT_PANE_RIGHT], NULL, TRUE);

				hWndTemp = m_hWndPane1[SPLIT_PANE_LEFT];
				//if (::IsWindow(hWndTemp) && ::IsWindowVisible(hWndTemp))
				::InvalidateRect(hWndTemp, NULL, TRUE);
				hWndTemp = m_hWndPane1[SPLIT_PANE_RIGHT];
				//if (::IsWindow(hWndTemp) && ::IsWindowVisible(hWndTemp))
				::InvalidateRect(hWndTemp, NULL, TRUE);
			}
		}
	}

	void DoAskQuestion()
	{
		int w = m_rcSplitter.right - m_rcSplitter.left;

		ATLASSERT(m_winStatus & XWIN_CHAT);
		if ((m_nSinglePane == SPLIT_PANE_NONE) && (m_xySplitterPos > 0) && (m_xySplitterPos < w))
		{
			U8* screen_data;
			U32 screen_len;
			U8 ask[32] = { 0 };
			U8* q = ask;
			q[0] = '\n'; q[1] = 0xF0; q[2] = 0x9F; q[3] = 0xA4; q[4] = 0x9A; q[5] = '\n';
			q[6] = 'H';
			q[7] = 'o';
			q[8] = 'w';
			q[9] = ' ';
			q[10] = 't';
			q[11] = 'o';
			q[12] = ' ';
			q[13] = 'f';
			q[14] = 'i';
			q[15] = 'x';
			q[16] = ' ';
			q[17] = 'i';
			q[18] = 't';
			q[19] = '?';
			q[20] = '\n';
			q[21] = '\0';
			m_viewOutput.AppendText((const char*)q, 21);

			screen_len = 0;
			screen_data = m_viewWork.GetScreenData(screen_len);
			if (screen_data && screen_len)
			{
				MessageTask* mt = nullptr;
				U32 quesion_length = 15;
				q = ask + 6;
				screen_len += 8;
				U32 mt_len = sizeof(MessageTask) + quesion_length + screen_len + 1;
				mt = (MessageTask*)wt_palloc(g_sendMemPool, mt_len);
				if (mt)
				{
					U8* s = (U8*)mt;
					mt->next = NULL;
					mt->msg_state = 0;
					mt->msg_length = quesion_length + screen_len;
					mt->msg_body = s + sizeof(MessageTask);
					s = mt->msg_body;
					memcpy_s(s, quesion_length, q, quesion_length);
					s += quesion_length;
					*s++ = '"'; *s++ = '"'; *s++ = '"'; *s++ = '\n';
					memcpy_s(s, screen_len - 8, screen_data, screen_len - 8);
					s += screen_len - 8;
					*s++ = '\n'; *s++ = '"'; *s++ = '"'; *s++ = '"';
					*s = '\0';
					zx_PushIntoSendQueue(mt);
				}
			}
		}
	}

	void DoKnowledgeBaseSearch()
	{
		MessageBox(L"Not available now", L"Knowledage Base Search", MB_OK);
	}

	LRESULT OnChatGap(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_loadingPercent > 100)
		{
			if (wParam == 0)
			{
				U16 ctlId = (U16)lParam;
				ctlId &= 0xFF;
				switch (ctlId)
				{
				case XVIEWCG_BUTTON_ASKQUESTION:
					DoAskQuestion();
					break;
				case XVIEWCG_BUTTON_TOLEFTRIGHT:
					DoAIChatWindow();
					break;
				case XVIEWCG_BUTTON_SEARCHKB:
					DoKnowledgeBaseSearch();
					break;
				default:
					break;
				}
			}
		}
		return 0;
	}

	void DoTabIsChanged(U16 tabId)
	{
		int idx = m_nAIPane;
		RECT rect = {};
		ATLASSERT(tabId == XVIEWCT_BUTTON_TABAI || tabId == XVIEWCT_BUTTON_TABKB);
		if (GetSplitterPaneRect(idx, &rect))
		{
			int top, bottom;
			top = rect.top + m_tabPane[idx];
			bottom = rect.bottom;
			if (m_winStatus & XWIN_INPUT)
			{
				bottom -= (m_heightStatusbar + m_heightPane1[idx] + m_gapPane[idx]);
			}

			if (tabId == XVIEWCT_BUTTON_TABAI)
			{
				::SetWindowPos(m_viewOutput, NULL,
					rect.left,
					top,
					rect.right - rect.left,
					bottom - top,
					SWP_NOZORDER);
				m_viewOutput.ShowWindow(SW_SHOW);
				m_viewInput.ShowWindow(SW_SHOW);
				m_viewTree.ShowWindow(SW_HIDE);
				m_winStatus |= (XWIN_CHAT | XWIN_INPUT);
				m_winStatus &= ~XWIN_TREE;
				m_duiViewCG.SetWinMode(MODE_AICHAT, (m_nSinglePane == SPLIT_PANE_NONE)? true : false);
			}
			else
			{
				::SetWindowPos(m_viewTree, NULL,
					rect.left,
					top,
					rect.right - rect.left,
					bottom - top,
					SWP_NOZORDER);
				m_viewOutput.ShowWindow(SW_HIDE);
				m_viewInput.ShowWindow(SW_SHOW);
				m_viewTree.ShowWindow(SW_SHOW);
				m_winStatus |= (XWIN_TREE | XWIN_INPUT);
				m_winStatus &= ~XWIN_CHAT;
				m_duiViewCG.SetWinMode(MODE_KBTREE, (m_nSinglePane == SPLIT_PANE_NONE) ? true : false);
			}
		}
		m_duiViewCT.DoTabIsChanged(tabId);
		Invalidate();
	}

	void DoKnowledgeBaseSync()
	{
		DWORD in_threadid = 0; /* required for Win9x */
		HANDLE hThread = CreateThread(NULL, 0, openkb_threadfunc, m_hWnd, 0, &in_threadid);
		if (hThread) /* we don't need the thread handle */
			CloseHandle(hThread);
	}

	LRESULT OnChatTab(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_loadingPercent > 100)
		{
			if (wParam == 0)
			{
				U16 ctlId = (U16)lParam;
				ctlId &= 0xFF;
				switch (ctlId)
				{
				case XVIEWCT_BUTTON_EMPTYLOG:
					m_viewOutput.DoEmptyChatLog();
					break;
				case XVIEWCT_BUTTON_SAVELOG:
					m_viewOutput.DoSaveChatLog();
					break;
				case XVIEWCT_BUTTON_AISETTING:
					DoAIConfiguration();
					break;
				case XVIEWCT_BUTTON_TABAI:
				case XVIEWCT_BUTTON_TABKB:
					DoTabIsChanged(ctlId);
					break;
				case XVIEWCT_BUTTON_SYNC:
					DoKnowledgeBaseSync();
					break;
				default:
					break;
				}
			}
		}
		return 0;
	}

	void DoTTYConfiguration()
	{
		PTerm_do_settings(m_hWnd);
	}

	void DoAIConfiguration()
	{
		ZXConfig prevCf = { 0 };
		ZXConfig* cfNew = &ZXCONFIGURATION;
		ZXConfig* cfOld = &prevCf;
		ConfDlg dlg;

		// save the configuration before we open the dialog 
		memcpy_s(cfOld, sizeof(ZXConfig), cfNew, sizeof(ZXConfig));

		if (dlg.DoModal() == IDOK)
		{
			U8 network_changed = 0;
			U8 ui_changed = 0;
			U8 font_changed = 0;
			zx_SaveConfiguration(cfNew, cfOld, network_changed, ui_changed, font_changed);

			/* there is/are some network parameter changed, so we have to bounce the worker thread */
			if (network_changed)
				zx_BounceNetworkThread();
#if 0
			if (ui_changed)
				m_view.DoUIChange();
#endif 
			if (font_changed)
			{
				m_viewOutput.DoFontChange();
				m_viewInput.DoFontChange();
				m_viewEdit.DoFontChange();
			}

		}
	}

	void DoMainMenu()
	{
		POINT pt;
		HMENU hMenu;
		int nOtherPane = (m_nAIPane == SPLIT_PANE_RIGHT) ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;
		pt.x = 1;
		pt.y = m_rcSplitter.bottom - m_heightStatusbar - 2;
		ClientToScreen(&pt);

		hMenu = CreatePopupMenu();

		if (m_nSinglePane == SPLIT_PANE_NONE)
		{
			AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_VIEW_AI_WINDOW, _T("Hide AI Window"));
			AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_VIEW_LAYOUT_SWITCH, _T("Swith Layout"));
			AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_VIEW_TTY_WINDOW, _T("Hide TTY Window"));
			if (XWIN_EDIT & m_winStatus)
				AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_VIEW_NOTE_WINDOW, _T("Hide Notepad"));
			else
				AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_VIEW_NOTE_WINDOW, _T("Show Notepad"));

			AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_APP_TTY_SETTINGS, _T("TTY Settings"));
		}
		else
		{
			if (m_nSinglePane == m_nAIPane)
			{
				AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_VIEW_TTY_WINDOW, _T("Show TTY Window"));
			}
			else
			{
				AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_VIEW_AI_WINDOW, _T("Show AI Window"));
				if (XWIN_EDIT & m_winStatus)
					AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_VIEW_NOTE_WINDOW, _T("Hide Notepad"));
				else
					AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_VIEW_NOTE_WINDOW, _T("Show Notepad"));
				AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_APP_TTY_SETTINGS, _T("TTY Settings"));
			}
		}
		
		AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
		AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_APP_AI_SETTINGS, _T("AI Settings"));
		AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
		AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_APP_ABOUT, _T("About ZTerm@AI"));
		AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_APP_EXIT, _T("Exit"));
		TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);
	}

	LRESULT OnStatusbar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_loadingPercent > 100)
		{
			if (wParam == 0)
			{
				U16 ctlId = (U16)lParam;
				ctlId &= 0xFF;
				switch (ctlId)
				{
				case XVIEWSB_BUTTON_MAIN_MENU:
					DoMainMenu();
					break;
				case XVIEWSB_BUTTON_NOTEPAD_WINDOW:
					DoNotePadWindow();
					break;
				case XVIEWSB_BUTTON_LAYOUT_SWITCH:
					DoSwitchLayout();
					break;
				case XVIEWSB_BUTTON_AI_CHAT_WINDOW:
					DoAIChatWindow();
					break;
				default:
					break;
				}
			}
		}
		return 0;
	}

	void DoSize()
	{
		int idx, nOtherPane;
		U32  size;
		XRECT area = { 0 };
		XRECT* r = &area;
		RECT rcPane = {};
		U32* dst;

		ATLASSERT(m_rcSplitter.left == 0);
		ATLASSERT(m_rcSplitter.right > 0);
		ATLASSERT(m_rcSplitter.top == 0);
		ATLASSERT(m_rcSplitter.bottom > STATUSBAR_HEIGHT);

		U32 w = (U32)(m_rcSplitter.right - m_rcSplitter.left);
		U32 h = (U32)(STATUSBAR_HEIGHT + AICHAT_GAP + WINTAB_HEIGHT);

		if (nullptr != m_screenBuff)
		{
			VirtualFree(m_screenBuff, 0, MEM_RELEASE);
			m_screenBuff = nullptr;
			m_screenSize = 0;
		}

		m_screenSize = WT_ALIGN_PAGE(w * h * sizeof(U32));
		ATLASSERT(m_screenSize >= (w * h * sizeof(U32)));

		m_screenBuff = (U32*)VirtualAlloc(NULL, m_screenSize, MEM_COMMIT, PAGE_READWRITE);
		if (nullptr == m_screenBuff)
		{
			m_screenFixed = nullptr;
			m_duiViewSB.UpdateSize(nullptr, nullptr);
			m_duiViewCG.UpdateSize(nullptr, nullptr);
			m_duiViewCT.UpdateSize(nullptr, nullptr);
			m_duiViewEG.UpdateSize(nullptr, nullptr);
			Invalidate();
			return;
		}

		dst = m_screenBuff;
		r->left = m_rcSplitter.left;
		r->right = m_rcSplitter.right;
		r->bottom = m_rcSplitter.bottom;
		r->top = r->bottom - STATUSBAR_HEIGHT;
		m_duiViewSB.UpdateSize(r, dst);

		size = (U32)((r->right - r->left) * (r->bottom - r->top));
		dst += size;
		m_screenFixed = dst; // because statusbar are fixed

		ATLASSERT(m_nAIPane == SPLIT_PANE_RIGHT || m_nAIPane == SPLIT_PANE_LEFT);
		nOtherPane = (m_nAIPane == SPLIT_PANE_RIGHT) ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;

		idx = m_nAIPane;
		if (GetSplitterPaneRect(idx, &rcPane))
		{
			ATLASSERT(m_tabPane[idx] == WINTAB_HEIGHT);
			r->left = rcPane.left;
			r->right = rcPane.right;
			r->top = rcPane.top;
			r->bottom = r->top + m_tabPane[idx];
			m_duiViewCT.UpdateSize(r, dst);
			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			dst += size;

			ATLASSERT(m_gapPane[idx] == AICHAT_GAP);
			r->bottom = rcPane.bottom - (m_heightStatusbar + m_heightPane1[idx]);
			r->top = r->bottom - m_gapPane[idx];
			m_duiViewCG.UpdateSize(r, dst);
			m_duiViewCG.SetWindowShow(XWIN_INPUT & m_winStatus);

			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			dst += size;
		}

		idx = nOtherPane;
		if (GetSplitterPaneRect(idx, &rcPane))
		{
			r->left = rcPane.left;
			r->right = rcPane.right;
			r->bottom = rcPane.bottom - (m_heightStatusbar + m_heightPane1[idx]);
			r->top = r->bottom - m_gapPane[idx];
			m_duiViewEG.UpdateSize(r, dst);
			m_duiViewEG.SetWindowShow(XWIN_EDIT & m_winStatus);
		}
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam != SIZE_MINIMIZED)
		{
			this->SetSplitterRect();
			if(m_loadingPercent > 100)
				DoSize();
			ReleaseUnknown(m_pD2DRenderTarget);
			Invalidate();
		}
		//bHandled = FALSE;
		return 1;
	}

};

static DWORD WINAPI openweb_threadfunc(void* param)
{
	U64 dId = (U64)param;
	if (dId)
	{
		U8 DocID[16 + 1] = { 0 };
		wt_Raw2HexString((U8*)&dId, 8, DocID, NULL);
		zx_OpenWeb(DocID);
	}
	return 0;
}

static DWORD WINAPI openkb_threadfunc(void* param)
{
	HWND hWndUI = (HWND)param;

	if (::IsWindow(hWndUI))
	{
		zx_SyncKBDirectoryData(hWndUI);
	}
	return 0;
}
