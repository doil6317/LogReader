#include "EditBox.h"

#define CEDITWNDCLASS	_T("Edit")

#define Edit_AddString(hEdit, lpwStr)	\
	{ S}
#define Edit_GetSel(hEdit, pSt, pEd)	SendMessage(hEdit, EM_GETSEL, (WPARAM)pSt, (LPARAM)pEd)

CEditBox::CEditBox(DWORD dwCtrlId) : CWin(dwCtrlId, CEDITWNDCLASS)
{
}

CEditBox::~CEditBox(void)
{
}

HWND CEditBox::Create(DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, CWin* pParent)
{
	HWND hRes = CWin::Create(dwExStyle, CEDITWNDCLASS, lpszWindowName, dwStyle, x, y, nWidth, nHeight, pParent, (HMENU)this->m_dwWinId);
	AttachToWindow(hRes);
	return hRes;
}

void CEditBox::GetSel(LPDWORD pStart, LPDWORD pEnd)
{
	Edit_GetSel(this->GetHWnd(), pStart, pEnd);
}

int CEditBox::GetTextLength()
{
	return GetWindowTextLength(this->GetHWnd());
}

int CEditBox::SetText(LPCTSTR szText)
{
	return SetWindowText(this->GetHWnd(), szText);
}

int CEditBox::GetText(LPTSTR szBuff, UINT bufsiz)
{
	return GetWindowText(this->GetHWnd(), szBuff, bufsiz);
}

void CEditBox::AddText(LPCTSTR szText)
{
	SendMessage(this->GetHWnd(), EM_SETSEL, GetTextLength(), -1);
	SendMessage(this->GetHWnd(), EM_REPLACESEL, 0L, (LPARAM)(LPTSTR)(szText));
}

void CEditBox::Clear(void)
{
	SetText(_T(""));
}

void CEditBox::SelectAll(void)
{
	SendMessage(this->GetHWnd(), EM_SETSEL, 0, GetTextLength());
}

void CEditBox::SetLimit(int iLimit)
{
	SendMessage(this->GetHWnd(), EM_LIMITTEXT, (WPARAM)iLimit, 0);
}