// OptionWnd.h: COptionWnd クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONWND_H__1B6D434C_4291_4BC1_8FBA_EA22A90AED5C__INCLUDED_)
#define AFX_OPTIONWND_H__1B6D434C_4291_4BC1_8FBA_EA22A90AED5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DialogBox.h"

class COptionWnd : public CDialogBox  
{
	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);
	void OnShowWindow(BOOL bShow, UINT nStatus);
	void OnCommand(UINT uNotify, UINT uId, HWND hwndCtrl);
	void OnHScroll(UINT uCode, UINT uCurPos, HWND hwndCtrl);
	void OnChangeTransParent();
	void OnClickSave();
	void OnClose();
	void OnClickBrowseStart();
	void OnClickBrowseStop();
	BOOL OpenFileBrowse(LPTSTR filename, UINT cchLen);
public:
	COptionWnd();
	virtual ~COptionWnd();
};

#endif // !defined(AFX_OPTIONWND_H__1B6D434C_4291_4BC1_8FBA_EA22A90AED5C__INCLUDED_)
