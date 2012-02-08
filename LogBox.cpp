// LogBox.cpp: CLogBox クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "LogBox.h"
#include <stdlib.h>
#include <strsafe.h>
#include <malloc.h>
#define PCRE_STATIC
#include "pcre.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CLogBox::CLogBox(DWORD dwCtrlId) : CEditBox(dwCtrlId)
{
	ZeroMemory(szFilter, sizeof(szFilter));
	ZeroMemory(szFilterDisp, sizeof(szFilterDisp));
	bUseRegExp = FALSE;
}

CLogBox::~CLogBox()
{

}

HWND CLogBox::Create(int x, int y, int nWidth, int nHeight, CWin* pParent)
{
	return CEditBox::Create(WS_EX_CLIENTEDGE, _T(""), WS_TABSTOP | WS_VISIBLE | WS_CHILD | 
		ES_LEFT | ES_MULTILINE | ES_READONLY | ES_WANTRETURN | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL | ES_NOHIDESEL,
		x, y, nWidth, nHeight, pParent);
}

BOOL CLogBox::OnContextMenu(HWND hwndCtrl, int xPos, int yPos)
{
	HMENU hMenu = LoadMenu(GethInstance(), MAKEINTRESOURCE(IDR_LOGMENU));
	__try
	{
		HMENU hEditMenu = GetSubMenu(hMenu, 0);
		
		DWORD selSt, selEd;
		
		if (GetTextLength() < 1)
		{
			ModifyMenu(hEditMenu, IDM_LOG_ALLSEL, MF_BYCOMMAND | MF_GRAYED, IDM_LOG_ALLSEL, _T("すべて選択(&A)"));
			ModifyMenu(hEditMenu, IDM_LOG_COPY, MF_BYCOMMAND | MF_GRAYED, IDM_LOG_COPY, _T("コピー(&C)"));
			ModifyMenu(hEditMenu, IDM_LOG_CLEAR, MF_BYCOMMAND | MF_GRAYED, IDM_LOG_CLEAR, _T("クリア(&D)"));
		}
		else
		{
			this->GetSel(&selSt, &selEd);
			if (selSt == selEd)
			{
				// 選択なし
				ModifyMenu(hEditMenu, IDM_LOG_COPY, MF_BYCOMMAND | MF_GRAYED, IDM_LOG_COPY, _T("コピー(&C)"));
				
			}
		}
		
		TrackPopupMenu(hEditMenu, TPM_RIGHTBUTTON, xPos, yPos, 0, hwndCtrl, NULL);
	}
	__finally
	{
		DestroyMenu(hMenu);
	}
	
	return TRUE;
}

void CLogBox::OnKeyDown(UINT nChar, UINT nRepeat, UINT nFlags)
{
	SHORT res = GetAsyncKeyState(VK_LCONTROL);
	DebugLog(_T("nChar[0x%X] nRepeat[0x%X] nFlags[0x%X] res[0x%X]\r\n"), nChar, nRepeat, nFlags, res);
	if (res == 0)
		return;
	switch(nChar)
	{
	case 'C':
		OnCopyToClibBoard();
		break;
	case 'A':
		SelectAll();
		break;
	}
}

void CLogBox::OnCommand(UINT uNotify, UINT uId, HWND hwndCtrl)
{
	switch(uId)
	{
	case IDM_LOG_ALLSEL:
		SendMessage(this->GetHWnd(), EM_SETSEL, 0, this->GetTextLength());
		break;
	case IDM_LOG_CLEAR:
		SetText(_T(""));
		break;
	case IDM_LOG_COPY:
		OnCopyToClibBoard();
		break;
	}
}

void CLogBox::OnCopyToClibBoard()
{
	DWORD dwSt, dwEd;
	GetSel(&dwSt, &dwEd);
	
	if (dwSt == dwEd)
		return;
	
	UINT uLen = GetTextLength() + 1;
	
	LPTSTR buff = (LPTSTR)malloc(uLen * sizeof(TCHAR));
	
	GetText(buff, uLen - 1);
	
	HGLOBAL hContext;
	hContext = GlobalAlloc(GHND, (dwEd - dwSt + 1) * sizeof(TCHAR));
	LPTSTR tmp = (LPTSTR)GlobalLock(hContext);
	_tcsncpy(tmp, &buff[dwSt], dwEd - dwSt);
	GlobalUnlock(hContext);
	
	free(buff);
	
	if(OpenClipboard(this->m_hWnd))
	{
		EmptyClipboard();
		SetClipboardData(CF_OEMTEXT, hContext);
		CloseClipboard();
	}
	else
	{
		GlobalFree(hContext);
	}	
}

LPCTSTR CLogBox::GetFilterDisp(void) const
{
	return szFilterDisp;
}

LPCTSTR CLogBox::GetFilter(void) const
{
	return szFilter;
}

BOOL CLogBox::SetFilter(LPCTSTR newFilter)
{
	TCHAR tmp[_countof(szFilter)];

	StringCchCopy(szFilterDisp, _countof(szFilterDisp), newFilter);

	if (bUseRegExp)
	{
		StringCchPrintf(szFilter, _countof(szFilter), _T("[^\n].*%s.*\n"), newFilter);
		// テスト正規表現
		if (TestRegExp(szFilter))
			return TRUE;
		return FALSE;
	}
	StringCchCopy(tmp, _countof(tmp), newFilter);
	ChangeSpecialChar(tmp, _countof(tmp));	
	StringCchPrintf(szFilter, _countof(szFilter), _T("[^\n].*%s.*\n"), tmp);
	return TRUE;
}

BOOL CLogBox::IsUseRegExp() const
{
	return this->bUseRegExp;
}

void CLogBox::SetUseRegExp(BOOL bUseRegExp)
{
	this->bUseRegExp = bUseRegExp;
}

void CLogBox::AddToEditByCheckFilter(LPCTSTR data)
{
	if (lstrlen(szFilterDisp) < 1)
		return ;

	pcre* regexp;
	const char* err;
	int erroffset;
	LPSTR res = NULL;
	regexp = pcre_compile(this->szFilter, 0, &err, &erroffset, NULL);
	if (!regexp)
	{
		// 正規表現ミス
		return;
	}
	__try
	{
		int result;
		const int ovectorlength = 30;
		int ovector[ovectorlength];
		int st = 0;
		int prelen = 0;
		int curlen = 0;
		while ((result = pcre_exec(regexp,NULL,data,strlen(data),st,0,ovector,ovectorlength)) > 0)
		{
			curlen += ovector[1] - ovector[0];
			if (res == NULL)
			{
				res = (LPSTR)malloc(curlen + 1);
			}
			else
			{
				res = (LPSTR)realloc(res, curlen + 1);
			}
			
			strncpy(&res[prelen], &data[ovector[0]], ovector[1] - ovector[0]);
			res[curlen] = 0;
			prelen = curlen;
			st = ovector[1];
		}
	}
	__finally
	{
		pcre_free(regexp);
	}
	if (res)
	{
		AddText(res);
		free(res);
	}
}

BOOL CLogBox::TestRegExp(LPCTSTR pattern)
{
	pcre* regexp;
	const char* err;
	int erroffset;
	regexp = pcre_compile(pattern, 0, &err, &erroffset, NULL);
	if (!regexp)
	{
		return FALSE;
	}
	pcre_free(regexp);
	return TRUE;
}

void CLogBox::ChangeSpecialChar(LPTSTR src, UINT bufSiz)
{
	UINT len = lstrlen(src);
	const LPCTSTR spCh = "\\^$.[]|()?*+{}";
	const UINT spLen = 14;
	
	LPTSTR tmp = (LPTSTR)_alloca(bufSiz * sizeof(TCHAR));
	StringCchCopy(tmp, bufSiz, src);
	
	for (UINT i = 0, k = 0; i < len && k < bufSiz; i++, k++)
	{
		for(UINT j = 0; j < spLen; j++)
		{
			if (tmp[i] == spCh[j])
			{
				src[k] = '\\';
				k++;
				break;
			}
		}
		src[k] = tmp[i];
	}
}