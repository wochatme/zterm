// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#define SCREEN_BUF_MAX		(1<<18)
class CView
{
public:
	U8* m_utf8Buf = nullptr;
	U32 m_utf8Max = 0;
	U32 m_utf8Len = 0;

	WCHAR* m_utf16Buf = nullptr;
	U32 m_utf16Max = 0;
	U32 m_utf16Lne = 0;

public:
	HWND m_hWnd = nullptr;

	CView()
	{
		m_utf16Max = SCREEN_BUF_MAX;
		m_utf16Lne = 0;
		m_utf16Buf = (WCHAR*)VirtualAlloc(NULL, m_utf16Max, MEM_COMMIT, PAGE_READWRITE);
		ATLASSERT(m_utf16Buf);
		m_utf8Max = 0;
	}

	~CView()
	{
		if (nullptr != m_utf16Buf)
		{
			VirtualFree(m_utf16Buf, 0, MEM_RELEASE);
			m_utf16Buf = nullptr;
		}
		m_utf16Lne = 0;

		if (nullptr != m_utf8Buf)
		{
			VirtualFree(m_utf16Buf, 0, MEM_RELEASE);
			m_utf8Buf = nullptr;
		}
		m_utf8Len = 0;
	}

	operator HWND() const throw()
	{
		return m_hWnd;
	}

	BOOL IsWindow() const throw()
	{
		return ::IsWindow(m_hWnd);
	}

	BOOL ShowWindow(int nCmdShow)
	{
		BOOL bRet = FALSE;

		if (IsWindow())
		{
			bRet = ::ShowWindow(m_hWnd, nCmdShow);
		}

		return bRet;
	}

	HWND SetFocus()
	{
		HWND hWnd = NULL;
		if (IsWindow())
		{
			hWnd = ::SetFocus(m_hWnd);
		}
		return hWnd;
	}

	HWND Create(
		_In_opt_ HWND hWndParent,
		_In_ _U_RECT rect = NULL,
		_In_opt_z_ LPCTSTR szWindowName = NULL,
		_In_ DWORD dwStyle = 0,
		_In_ DWORD dwExStyle = 0,
		_In_ _U_MENUorID MenuOrID = 0U,
		_In_opt_ LPVOID lpCreateParam = NULL)
	{
		m_hWnd = PTerm_CreateWindow(hWndParent);
		return m_hWnd;
	}

	BOOL SetWindowPos(
		HWND hWndInsertAfter,
		int  X,
		int  Y,
		int  cx,
		int  cy,
		UINT uFlags
	)
	{
		BOOL bRet = FALSE;
		if (IsWindow())
		{
			bRet = ::SetWindowPos(m_hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
		}
		return bRet;
	}

	bool DoEditCopy()
	{
		return true;
	}

	U8* GetScreenData(U32& length)
	{
		U8* sbuf = nullptr;
		if (m_utf16Buf)
		{
			U32 slen = PTerm_copy_screen(m_utf16Buf, m_utf16Max);
			if (slen == 0)
			{
				VirtualFree(m_utf16Buf, 0, MEM_RELEASE);
				m_utf16Max <<= 1;
				m_utf16Buf = (wchar_t*)VirtualAlloc(NULL, m_utf16Max, MEM_COMMIT, PAGE_READWRITE);
				if (m_utf16Buf)
				{
					slen = PTerm_copy_screen(m_utf16Buf, m_utf16Max);
				}
			}

			if (slen)
			{
				U32 utf8len = 0;
				ATLASSERT(m_utf16Buf);
				U32 status = wt_UTF16ToUTF8((U16*)m_utf16Buf, slen, NULL, &utf8len);
				if (status == WT_OK && utf8len)
				{
					if (utf8len > m_utf8Max)
					{
						if (m_utf8Buf)
						{
							VirtualFree(m_utf8Buf, 0, MEM_RELEASE);
						}
						m_utf8Max = WT_ALIGN_PAGE(utf8len);
						m_utf8Buf = (U8*)VirtualAlloc(NULL, m_utf8Max, MEM_COMMIT, PAGE_READWRITE);
					}
					ATLASSERT(m_utf8Max > utf8len);
					if (m_utf8Buf)
					{
						status = wt_UTF16ToUTF8((U16*)m_utf16Buf, slen, m_utf8Buf, NULL);
						length = utf8len;
						m_utf8Buf[utf8len] = '\0';
						sbuf = m_utf8Buf;
					}
				}
			}
		}
		return sbuf;
	}
};
