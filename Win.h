#ifndef _MYWINDOWCLASS_1234567890
#define _MYWINDOWCLASS_1234567890

#define WIN32_LEAN_AND_MEAN
#define ISOLATION_AWARE_ENABLED	1	// For XP Style
#define _WIN32_WINNT 0x501			// For XP Style
#define _WIN32_IE 0x0600			// Comctl32.dll 6.0バージョン使用

#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include "define.h"

class CWin
{
protected:
	static CWin* s_this;
	static LRESULT CALLBACK _WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	WNDPROC m_preWndproc;
	virtual LRESULT WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Functions For Combine Window And CWin Class
	virtual void AfterNCDestroy(void);

	// Window Event Procedures
	virtual LRESULT OnCreate(LPCREATESTRUCT lpcs);
	virtual void OnCommand(UINT uNotify, UINT uId, HWND hwndCtrl);
	virtual void OnNotify(LPNMHDR lpnmh);
	virtual void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnSize(UINT width, UINT height);
	virtual void OnMoving(LPRECT lpRect);
	virtual void OnMove(UINT width, UINT height);
	virtual void OnTimer(UINT uId);
	virtual void OnKeyDown(UINT nChar, UINT nRepeat, UINT nFlags);
	virtual void OnKeyUp(UINT nChar, UINT nRepeat, UINT nFlags);
	virtual BOOL OnContextMenu(HWND hwndCtrl, int xPos, int yPos);
	virtual void OnGetMinMaxInfo(LPMINMAXINFO lpMMI);
	virtual LRESULT OnCtlColorStatic(HDC hStaticDC, HWND hStatic);
	virtual void OnClose(void);

	void DebugLog(LPCTSTR format, ...);

	HWND m_hWnd;
	DWORD m_dwWinId;
public:
	CWin(DWORD dwWinId, WNDPROC preWndproc);
	CWin(DWORD dwCtrlId, LPCTSTR lpszWndClassName);
	virtual ~CWin(void);

	// API Functions
	static HINSTANCE GethInstance(void);
	static WNDPROC GetClassWndproc(LPCTSTR lpszClass);
	HWND GetHWnd(void) const;
	BOOL GetWindowInfo(PWINDOWINFO pwi);
	HWND Create(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
		DWORD dwStyle, int x, int y, int nWidth, int nHeight,
		CWin* pParent, HMENU nIDorHMenu = NULL, LPVOID lpParam = NULL);
	BOOL ShowWindow(int nCmdShow) const;
	int MessageBox(LPCTSTR lpText, LPCTSTR lpCaption = NULL, UINT uType = NULL) const;
	BOOL IsWindow(void);
	HANDLE GetProp(LPCTSTR lpString);
	BOOL SetProp(LPCTSTR lpString, HANDLE hData);
	CWin* GetParent(void);
	BOOL GetWindowRect(LPRECT pRc);
	BOOL GetClientRect(LPRECT pRc);
	void SetFont(HFONT hNewFont, BOOL bRedraw = TRUE);


	static const CWin* winTop;
	static const CWin* winBotton;
	static const CWin* winTopMost;
	static const CWin* winNoTopMost;
	BOOL SetWindowPos(CWin* pWinAfterInsert, int X, int Y, int cx, int cy, UINT uFlags);
	BOOL MoveToParentCenter(void);

	// Functions For Framework;
	BOOL AttachToWindow(HWND hWndCtrl);
	WNDPROC SetWindowProc(WNDPROC newWndproc);
};

#endif 