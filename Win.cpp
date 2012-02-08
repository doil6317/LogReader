#include "Win.h"
#include <WindowsX.h>
#include <stdio.h>
#include <stdarg.h>

#define CWINPROCSTR			_T("CWin_Class_String_For_Window_Message_Procedure_Function")

const CWin* CWin::winTop = (CWin*)HWND_TOP;
const CWin* CWin::winBotton = (CWin*)HWND_BOTTOM;
const CWin* CWin::winTopMost = (CWin*)HWND_TOPMOST;
const CWin* CWin::winNoTopMost = (CWin*)HWND_NOTOPMOST;

CWin* CWin::s_this = NULL;

CWin::CWin(DWORD dwWinId, WNDPROC preWndproc)
	: m_dwWinId(dwWinId), m_preWndproc(preWndproc)
{
	m_hWnd = NULL;
}

CWin::CWin(DWORD dwCtrlId, LPCTSTR lpszWndClassName)
{
	CWin(dwCtrlId, GetClassWndproc(lpszWndClassName));
}

CWin::~CWin(void)
{
}

LRESULT CALLBACK CWin::_WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWin* pThis = reinterpret_cast<CWin*>(::GetProp(hWnd, CWINPROCSTR));
	if(ISNULL(pThis))
	{
		if (ISNULL(CWin::s_this))
		{
			WINDOWINFO wi;
			WNDPROC lpfnWndProc;

			::GetWindowInfo(hWnd, &wi);
			lpfnWndProc = CWin::GetClassWndproc(reinterpret_cast<LPCTSTR>(wi.atomWindowType));
			if (lpfnWndProc != CWin::_WinProc)
				return CallWindowProc(lpfnWndProc, hWnd, uMsg, wParam, lParam);
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		pThis = CWin::s_this;
		CWin::s_this = NULL;
		pThis->m_hWnd = hWnd;
		::SetProp(hWnd, CWINPROCSTR, reinterpret_cast<HANDLE>(pThis));
	}
	return pThis->WinProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CWin::WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		return this->OnCreate(reinterpret_cast<LPCREATESTRUCT>(lParam));
	case WM_SHOWWINDOW:
		this->OnShowWindow((BOOL)wParam, (UINT)lParam);
		break;
	case WM_COMMAND:
		this->OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;
	case WM_NOTIFY:
		this->OnNotify((LPNMHDR)lParam);
		break;
	case WM_GETDLGCODE:
		return DLGC_WANTTAB | DLGC_WANTALLKEYS;
	case WM_SIZE:
		this->OnSize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOVING:
		this->OnMoving((LPRECT)lParam);
		break;
	case WM_MOVE:
		this->OnMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_TIMER:
		this->OnTimer(wParam);
		break;
	case WM_KEYDOWN:
		this->OnKeyDown((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYUP:
		this->OnKeyUp((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_CONTEXTMENU:
		if(this->OnContextMenu((HWND)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;
	case WM_GETMINMAXINFO:
		this->OnGetMinMaxInfo((LPMINMAXINFO)lParam);
		break;
	case WM_CTLCOLORSTATIC :
		return this->OnCtlColorStatic((HDC)wParam, (HWND)lParam);
	case WM_CLOSE:
		this->OnClose();
		break;
	case WM_NCDESTROY:
		this->AfterNCDestroy();
		break;
	}
	return CallWindowProc(this->m_preWndproc, hWnd, uMsg, wParam, lParam);
}

void CWin::AfterNCDestroy(void)
{
	SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG>(this->m_preWndproc));
	RemoveProp(m_hWnd, CWINPROCSTR);
}

// Window Message Procedures
LRESULT CWin::OnCreate(LPCREATESTRUCT lpcs)
{
	return 0;
}

void CWin::OnCommand(UINT uNotify, UINT uId, HWND hwndCtrl) 
{
}

void CWin::OnShowWindow(BOOL bShow, UINT nStatus)
{
}

void CWin::OnNotify(LPNMHDR lpnmh)
{
}

void CWin::OnSize(UINT width, UINT height)
{
}

void CWin::OnMoving(LPRECT lpRect)
{
}

void CWin::OnMove(UINT width, UINT height)
{
}

void CWin::OnTimer(UINT uId)
{
}

void CWin::OnKeyDown(UINT nChar, UINT nRepeat, UINT nFlags)
{
}

void CWin::OnKeyUp(UINT nChar, UINT nRepeat, UINT nFlags)
{
}

BOOL CWin::OnContextMenu(HWND hwndCtrl, int xPos, int yPos)
{
	return FALSE;
}

void CWin::OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
{
}

LRESULT CWin::OnCtlColorStatic(HDC hStaticDC, HWND hStatic)
{
	return CallWindowProc(this->m_preWndproc, this->GetHWnd(), WM_CTLCOLORSTATIC, (WPARAM)hStaticDC, (LPARAM)hStatic);
}

void CWin::OnClose(void)
{
}

// Window API Functions
HWND CWin::GetHWnd(void) const
{
	return this->m_hWnd;
}

HINSTANCE CWin::GethInstance(void)
{
	return reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
}

WNDPROC CWin::GetClassWndproc(LPCTSTR lpszClass)
{
	WNDCLASSEX wcex;
	if(ISNULL(GetClassInfoEx(CWin::GethInstance(), lpszClass, &wcex)))
		return reinterpret_cast<WNDPROC>(NULL);
	return wcex.lpfnWndProc;
}

WNDPROC CWin::SetWindowProc(WNDPROC newWndproc)
{
	return (WNDPROC)::SetWindowLongPtr(this->GetHWnd(), GWLP_WNDPROC, (LONG_PTR)newWndproc);
}

BOOL CWin::GetWindowInfo(PWINDOWINFO pwi)
{
	return ::GetWindowInfo(this->m_hWnd, pwi);
}

HWND CWin::Create(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
	DWORD dwStyle, int x, int y, int nWidth, int nHeight,
	CWin* pParent, HMENU nIDorHMenu, LPVOID lpParam)
{
	HWND hWndParent = pParent ? pParent->GetHWnd() : NULL;
	CWin::s_this = this;
	return ::CreateWindowEx(dwExStyle, lpszClassName, lpszWindowName, dwStyle, x, y, nWidth, nHeight,
		hWndParent, nIDorHMenu, CWin::GethInstance(), lpParam);
}

BOOL CWin::ShowWindow(int nCmdShow) const
{
	return ::ShowWindow(this->m_hWnd, nCmdShow);
}

int CWin::MessageBox(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) const
{
	return ::MessageBox(this->m_hWnd, lpText, lpCaption, uType);
}

void CWin::DebugLog(LPCTSTR format, ...)
{
	TCHAR buff[1024];
	va_list args;
	va_start(args, format);
	_vsntprintf(buff, _countof(buff), format, args);
	va_end(args);
	OutputDebugString(buff);
}

BOOL CWin::IsWindow(void)
{
	if (this->GetHWnd())
		return ::IsWindow(this->GetHWnd());
	return FALSE;
}

HANDLE CWin::GetProp(LPCTSTR lpString)
{
	return ::GetProp(this->GetHWnd(), lpString);
}

BOOL CWin::SetProp(LPCTSTR lpString, HANDLE hData)
{
	return ::SetProp(this->GetHWnd(), lpString, hData);
}

CWin* CWin::GetParent(void)
{
	HWND hwndParent = ::GetParent(this->GetHWnd());
	if (::IsWindow(hwndParent))
		return reinterpret_cast<CWin*>(::GetProp(hwndParent, CWINPROCSTR));
	return NULL;
}

BOOL CWin::GetWindowRect(LPRECT pRc)
{
	return ::GetWindowRect(this->GetHWnd(), pRc);
}

BOOL CWin::GetClientRect(LPRECT pRc)
{
	return ::GetClientRect(this->GetHWnd(), pRc);
}

BOOL CWin::SetWindowPos(CWin* pWinAfterInsert, int X, int Y, int cx, int cy, UINT uFlags)
{
	HWND hwndAfterInsert;
	switch((UINT)pWinAfterInsert)
	{
	case HWND_BOTTOM:
	case HWND_NOTOPMOST:
	case HWND_TOP:
	case HWND_TOPMOST:
		hwndAfterInsert = (HWND)pWinAfterInsert;
		break;
	default:
		hwndAfterInsert = pWinAfterInsert->GetHWnd();
		break;
	}
	return ::SetWindowPos(this->GetHWnd(), hwndAfterInsert, X, Y, cx, cy, uFlags);
}

void CWin::SetFont(HFONT hNewFont, BOOL bRedraw)
{
	SendMessage(GetHWnd(), WM_SETFONT, (WPARAM)hNewFont, MAKELPARAM(bRedraw, 0));
}

BOOL CWin::MoveToParentCenter(void)
{
	CWin* pParent = this->GetParent();
	RECT rcParent, rcMe;
	
	if (pParent)
		pParent->GetWindowRect(&rcParent);
	else
		// Parent is Desktop
		::SetRect(&rcParent, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	this->GetWindowRect(&rcMe);
	
	return this->SetWindowPos(NULL,
		rcParent.left + ((rcParent.right - rcParent.left) - (rcMe.right - rcMe.left)) / 2,
		rcParent.top + ((rcParent.bottom - rcParent.top) - (rcMe.bottom - rcMe.top)) / 2,
		0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

BOOL CWin::AttachToWindow(HWND hWndCtrl)
{
	this->m_hWnd = hWndCtrl;
	::SetProp(this->GetHWnd(), CWINPROCSTR, reinterpret_cast<HANDLE>(this));
	m_preWndproc = SetWindowProc(CWin::_WinProc);
	return TRUE;
}