// ListCtrl.cpp: CListCtrl クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "ListCtrl.h"

#define CLISTCTRLWNDCLASS	WC_LISTVIEW

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CListCtrl::CListCtrl(DWORD dwCtrlId) : CWin(dwCtrlId, CLISTCTRLWNDCLASS)
{

}

CListCtrl::~CListCtrl()
{

}

int CListCtrl::InsertColumn(int iCol, LPLVCOLUMN lplvcol)
{
	return ListView_InsertColumn(this->GetHWnd(), iCol, lplvcol);
}

int CListCtrl::InsertColumn(int iCol, LPCTSTR lpszColumnHeading, int nFormat, int nWidth, int nSubItem)
{
	LVCOLUMN lvcol = {0};
	lvcol.mask = LVCF_FMT | LVCF_TEXT | 
		((nWidth == -1) ? 0 : LVCF_WIDTH) |
		((nSubItem == -1) ? 0 : LVCF_SUBITEM);
	lvcol.fmt = nFormat;
	lvcol.cx = nWidth;
	lvcol.pszText = (LPTSTR)lpszColumnHeading;
	lvcol.cchTextMax = lpszColumnHeading ? lstrlen(lpszColumnHeading) : 0;
	lvcol.iSubItem = nSubItem;

	return this->InsertColumn(iCol, &lvcol);
}

int CListCtrl::InsertItem(const LPLVITEM pItem)
{
	return ListView_InsertItem(this->GetHWnd(), pItem);
}

int CListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = nItem;
	lvItem.pszText = (LPTSTR)lpszItem;

	return this->InsertItem(&lvItem);
}

int CListCtrl::InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
{
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
	lvItem.iItem = nItem;
	lvItem.pszText = (LPTSTR)lpszItem;
	lvItem.iImage = nImage;
	
	return this->InsertItem(&lvItem);	
}

int CListCtrl::InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem,
						  UINT nState, UINT nStateMask, int nImage, LPARAM lParam)
{
	LVITEM lvItem = {0};
	lvItem.mask = nMask;
	lvItem.iItem = nItem;
	lvItem.pszText = (LPTSTR)lpszItem;
	lvItem.state = nState;
	lvItem.stateMask = nStateMask;
	lvItem.iImage = nImage;
	lvItem.lParam = lParam;
	
	return this->InsertItem(&lvItem);
}

int CListCtrl::AddItem(LPCTSTR lpszItem)
{
	return this->InsertItem(this->GetItemCount(), lpszItem);
}

int CListCtrl::GetItem(LPLVITEM pItem)
{
	return ListView_GetItem(this->GetHWnd(), pItem);
}

int CListCtrl::GetItemCount(void)
{
	return ListView_GetItemCount(this->GetHWnd());
}