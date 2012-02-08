// DialogBox.h: CDialogBox クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIALOGBOX_H__0A33D1E6_53FB_4B72_A44A_DD125C9E165D__INCLUDED_)
#define AFX_DIALOGBOX_H__0A33D1E6_53FB_4B72_A44A_DD125C9E165D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"

class CDialogBox : public CWin  
{
protected:
	virtual LRESULT WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Dialog Event Procedures
	virtual BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);
	virtual void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnCommand(UINT uNotify, UINT uId, HWND hwndCtrl);
	virtual void OnNotify(LPNMHDR lpnmh);
	virtual void OnHScroll(UINT uCode, UINT uCurPos, HWND hwndCtrl);
	virtual void OnSize(UINT width, UINT height);
	virtual void OnTimer(UINT uId);
	virtual void OnClose(void);

public:
	CDialogBox(DWORD dwDialogId);
	virtual ~CDialogBox();

	virtual BOOL doModal(CWin* pParent);
	// Dialog API Functions
	BOOL EndDialog(int nResult);
	HWND GetDlgItem(DWORD dwCtrlId);
	LONG SendDlgItemMessage(__in int nIDDlgItem, __in UINT Msg, __in WPARAM wParam, __in LPARAM lParam);
};

#endif // !defined(AFX_DIALOGBOX_H__0A33D1E6_53FB_4B72_A44A_DD125C9E165D__INCLUDED_)
