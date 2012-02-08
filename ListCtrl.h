// ListCtrl.h: CListCtrl クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LISTCTRL_H__B9D11CE8_0F12_4E62_AF44_9746F1C7B344__INCLUDED_)
#define AFX_LISTCTRL_H__B9D11CE8_0F12_4E62_AF44_9746F1C7B344__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"

class CListCtrl : public CWin  
{
public:
	CListCtrl(DWORD dwCtrlId);
	virtual ~CListCtrl();

	// Functions For List View Control
	int InsertColumn(int iCol, LPLVCOLUMN lplvcol);
	int InsertColumn(int iCol, LPCTSTR lpszColumnHeading,
		int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1);
	int InsertItem(const LPLVITEM pItem);
	int InsertItem(int nItem, LPCTSTR lpszItem);
	int InsertItem(int nItem, LPCTSTR lpszItem, int nImage);
	int InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask, int nImage, LPARAM lParam);
	int AddItem(LPCTSTR lpszItem);
	int GetItem(LPLVITEM pItem);
	int GetItemCount(void);
};

#endif // !defined(AFX_LISTCTRL_H__B9D11CE8_0F12_4E62_AF44_9746F1C7B344__INCLUDED_)
