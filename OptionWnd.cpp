// OptionWnd.cpp: COptionWnd クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "OptionWnd.h"
#include "MainWnd.h"
#include "resource.h"
#include <CommDlg.h>
#include <CommCtrl.h>

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

COptionWnd::COptionWnd() : CDialogBox(IDD_OPTION)
{

}

COptionWnd::~COptionWnd()
{

}

BOOL COptionWnd::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	CMainWnd* mainWnd = (CMainWnd*)this->GetParent();
	SetWindowText(GetDlgItem(IDC_WEBLOGIC_START), mainWnd->GetWebLogicStartCmd());
	SetWindowText(GetDlgItem(IDC_WEBLOGIC_STOP), mainWnd->GetWebLogicStopCmd());
	SendDlgItemMessage(IDC_SD_TRANSPARENT, TBM_SETRANGE, FALSE, MAKELPARAM(20, 100));
	SendDlgItemMessage(IDC_SD_TRANSPARENT, TBM_SETPOS, TRUE, mainWnd->GetTrans());
	return TRUE;
}

void COptionWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	this->MoveToParentCenter();
}

void COptionWnd::OnCommand(UINT uNotify, UINT uId, HWND hwndCtrl)
{
	switch(uId)
	{
	case IDC_BROWSE_START:
		this->OnClickBrowseStart();
		break;
	case IDC_BROWSE_STOP:
		this->OnClickBrowseStop();
		break;
	case IDC_SAVE:
		this->OnClickSave();
		EndDialog(TRUE);
		break;
	case IDC_CANCEL:
		EndDialog(FALSE);
		break;
	}
}

void COptionWnd::OnHScroll(UINT uCode, UINT uCurPos, HWND hwndCtrl)
{
	HWND hSd = GetDlgItem(IDC_SD_TRANSPARENT);
	if (hwndCtrl == hSd)
		this->OnChangeTransParent();
}

void COptionWnd::OnChangeTransParent()
{
	UINT uCurPos = SendDlgItemMessage(IDC_SD_TRANSPARENT, TBM_GETPOS, 0, 0);
	if (uCurPos < 20) 
	{
		uCurPos = 20;
	}
	CMainWnd* mainWnd = (CMainWnd*)this->GetParent();
	mainWnd->SetTrans(uCurPos);
}

void COptionWnd::OnClickSave()
{
	CMainWnd* mainWnd = (CMainWnd*)this->GetParent();
	TCHAR tmp[MAX_PATH];
	GetWindowText(GetDlgItem(IDC_WEBLOGIC_START), tmp, _countof(tmp));
	mainWnd->SetWebLogicStartCmd(tmp);
	GetWindowText(GetDlgItem(IDC_WEBLOGIC_STOP), tmp, _countof(tmp));
	mainWnd->SetWebLogicStopCmd(tmp);
}

void COptionWnd::OnClose(void)
{
	EndDialog(TRUE);
}

void COptionWnd::OnClickBrowseStart()
{
	TCHAR filename[MAX_PATH];
	GetWindowText(GetDlgItem(IDC_WEBLOGIC_START), filename, _countof(filename));
	if(!OpenFileBrowse(filename, _countof(filename)))
		return;
	
	SetWindowText(GetDlgItem(IDC_WEBLOGIC_START), filename);
}

void COptionWnd::OnClickBrowseStop()
{
	TCHAR filename[MAX_PATH];
	GetWindowText(GetDlgItem(IDC_WEBLOGIC_STOP), filename, _countof(filename));
	if(!OpenFileBrowse(filename, _countof(filename)))
		return;
	
	SetWindowText(GetDlgItem(IDC_WEBLOGIC_STOP), filename);
}

BOOL COptionWnd::OpenFileBrowse(LPTSTR filename, UINT cchLen)
{
	OPENFILENAME ofn;
	ZeroMemory(filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetHWnd();
	ofn.lpstrFilter = _T("CMDファイル(*.cmd)\0*.cmd\0全てのファイル(*.*)\0*.*\0");
	ofn.lpstrFile = filename;
	ofn.nMaxFile = cchLen;
	ofn.lpstrTitle = _T("ログファイルを指定して下さい。");
	return GetOpenFileName(&ofn);
}
