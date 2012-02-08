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
	case WM_COMMAND:
		return this->OnCommand();
	case WM_CLOSE:
		::EndDialog(hWnd, TRUE);
		break;
	}
	return FALSE;
}

BOOL CDialogBox::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	return TRUE;
}

BOOL CDialogBox::doModal(CWin* pParent)
{
	return DialogBox(CWin::GethInstance(), MAKEINTRESOURCE(this->m_dwWinId),
		pParent->GetHWnd(), (DLGPROC)CWin::_WinProc);
}