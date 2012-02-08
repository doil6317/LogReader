// LogBox.h: CLogBox クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGBOX_H__D60EE8A3_9746_4439_8147_2CB30C6953F3__INCLUDED_)
#define AFX_LOGBOX_H__D60EE8A3_9746_4439_8147_2CB30C6953F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EditBox.h"

class CLogBox : public CEditBox  
{
private:
	void OnKeyDown(UINT nChar, UINT nRepeat, UINT nFlags);
	BOOL OnContextMenu(HWND hwndCtrl, int xPos, int yPos);
	void OnCommand(UINT uNotify, UINT uId, HWND hwndCtrl);

	TCHAR szFilter[256];
	TCHAR szFilterDisp[256];
	BOOL bUseRegExp;
public:
	CLogBox(DWORD dwCtrlId);
	virtual ~CLogBox();

	HWND Create(int x, int y, int nWidth, int nHeight, CWin* pParent);
	LPCTSTR GetFilter(void) const;
	BOOL SetFilter(LPCTSTR newFilter);

	BOOL IsUseRegExp() const;
	void SetUseRegExp(BOOL bUseRegExp);
	void AddToEditByCheckFilter(LPCTSTR data);
	BOOL TestRegExp(LPCTSTR pattern);
	void ChangeSpecialChar(LPTSTR src, UINT bufSiz);
	LPCTSTR GetFilterDisp(void) const;
	void OnCopyToClibBoard();
};

#endif // !defined(AFX_LOGBOX_H__D60EE8A3_9746_4439_8147_2CB30C6953F3__INCLUDED_)
