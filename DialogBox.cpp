// DialogBox.cpp: CDialogBox クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "DialogBox.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CDialogBox::CDialogBox(DWORD dwDialogId) : CWin(dwDialogId, DefDlgProc)
{

}

CDialogBox::~CDialogBox()
{

}

LRESULT CDialogBox::WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return (LRESULT)this->DlgProc(hWnd, uMsg, wParam, lParam);
}

BOOL CDialogBox::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return this->OnInitDialog((HWND)wParam, lParam);
	case WM_SHOWWINDOW:
		this->OnShowWindow((BOOL)wParam, (UINT)lParam);
		break;
	case WM_COMMAND:
		this->OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;
	case WM_NOTIFY:
		this->OnNotify((LPNMHDR)lParam);
		break;
	case WM_SIZE:
		this->OnSize(LOWORD(lParam), HIWORD(lParam));
	case WM_HSCROLL:
		this->OnHScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;
	case WM_TIMER:
		this->OnTimer(wParam);
		break;
	case WM_CLOSE:
		this->OnClose();
		break;
	}
	return FALSE;
}

BOOL CDialogBox::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	return TRUE;
}

void CDialogBox::OnShowWindow(BOOL bShow, UINT nStatus)
{
}

void CDialogBox::OnCommand(UINT uNotify, UINT uId, HWND hwndCtrl) 
{
}

void CDialogBox::OnNotify(LPNMHDR lpnmh)
{
}

void CDialogBox::OnSize(UINT width, UINT height)
{
}

void CDialogBox::OnHScroll(UINT uCode, UINT uCurPos, HWND hwndCtrl)
{
}

void CDialogBox::OnTimer(UINT uId)
{
}

void CDialogBox::OnClose(void)
{
}

BOOL CDialogBox::doModal(CWin* pParent)
{
	CWin::s_this = this;
	return DialogBox(CWin::GethInstance(), MAKEINTRESOURCE(this->m_dwWinId),
		pParent ? pParent->GetHWnd() : NULL, (DLGPROC)CWin::_WinProc);
}

BOOL CDialogBox::EndDialog(int nResult)
{
	return ::EndDialog(this->GetHWnd(), nResult);
}

HWND CDialogBox::GetDlgItem(DWORD dwCtrlId)
{
	return ::GetDlgItem(this->GetHWnd(), dwCtrlId);
}

LONG CDialogBox::SendDlgItemMessage(__in int nIDDlgItem, __in UINT Msg, __in WPARAM wParam, __in LPARAM lParam)
{
	return ::SendDlgItemMessage(this->GetHWnd(), nIDDlgItem, Msg, wParam, lParam);
}