#pragma once
#include "win.h"
class CEditBox :
	public CWin
{
public:
	CEditBox(DWORD dwCtrlId);
	virtual ~CEditBox(void);

	HWND Create(DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, CWin* pParent);
	void GetSel(LPDWORD pStart, LPDWORD pEnd);
	int GetTextLength();
	int SetText(LPCTSTR szText);
	int GetText(LPTSTR szBuff, UINT bufsiz);
	void SetLimit(int iLimit);
	void AddText(LPCTSTR szText);
	void Clear(void);
	void SelectAll(void);
};