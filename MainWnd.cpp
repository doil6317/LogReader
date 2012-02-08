#include "MainWnd.h"
#include "resource.h"
#include <stdio.h>
#include <malloc.h>
#include <Process.h>
#include <tlhelp32.h>
#include "OptionWnd.h"
#include <Psapi.h>
#include <CommCtrl.h>
#include <CommDlg.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "pcre.lib")

#define MAINTITLE		_T("My LogView v0.01")
#define MAINWNDCLASS	_T("Main_Window_Class_Name")
#define DATAMUTEX		_T("My_LogViewer_Mutex")

#define CMDLINE			_T("C:\\Windows\\system32\\cmd.exe /C ")

#define GETDATATIMER	10001
#define SVRCHKTIMER		10002
#define CHKCPUUSE		10003
#define SPACE			10
#define STATUSBAR		1001
#define TIMERINTERVAL	10
#define SVRCHKINTERVAL	500
#define CHKCPUINTERVAL	100

static HANDLE hMutex = NULL;

#define StatusBar_SetPartText(hStatusBar, nPart, szText) \
	SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(nPart, 0), (LPARAM)szText)
#define Button_IsChecked(hButton) 	(BST_CHECKED == SendMessage(hButton, BM_GETCHECK, 0, 0))
#define Button_SetCheck(hButton, bCheck)	(SendMessage(hButton, BM_SETCHECK, (bCheck ? BST_CHECKED : BST_UNCHECKED), 0))

void EditComma(LPTSTR src, UINT cchMax);

typedef struct _DATANODE {
	_DATANODE* preNode;
	_DATANODE* nxtNode;
	PCHAR data;
} DATANODE, *PDATANODE;

PDATANODE ndHead = NULL;
PDATANODE ndTail = NULL;

BOOL CMainWnd::IsEmpty()
{
	return (ndHead == NULL);
}

void CMainWnd::AddToDataNoMutex(PCHAR data, UINT siz)
{
	PDATANODE newNode = (PDATANODE)malloc(sizeof(DATANODE));
	ZeroMemory(newNode, sizeof(DATANODE));
	if(ndHead == NULL)
		ndHead = newNode;
	else
	{
		ndTail->nxtNode = newNode;
		newNode->preNode = ndTail;
	}
	ndTail = newNode;
	newNode->data = (PCHAR)malloc((siz + 1) * sizeof(CHAR));
	strcpy(newNode->data, data);
}

void CMainWnd::AddToData(PCHAR data, UINT siz)
{
	WaitForSingleObject(hMutex, INFINITE);
	AddToDataNoMutex(data, siz);
	ReleaseMutex(hMutex);
}

PCHAR CMainWnd::GetListData()
{
	if (IsEmpty())
		return NULL;
	DWORD resMutex = WaitForSingleObject(hMutex, 1);
	if (resMutex != WAIT_OBJECT_0)
		return NULL;

	PDATANODE node = ndHead;
	ndHead = node->nxtNode;
	if (ndHead == NULL)
		ndTail = NULL;

	PCHAR res = node->data;
	free(node);
	ReleaseMutex(hMutex);
	return res;
}

CMainWnd::CMainWnd(void) : CWin(IDD_LOGVIEW, DefWindowProc), edTotal(IDC_ED_LOGVIEW), edFilter(IDC_ED_LOGVIEWFILTER)
{
	bCheckFile = FALSE;
	bRunThread = FALSE;
	bTop = FALSE;
	ZeroMemory(this->buff, sizeof(this->buff));
	this->uBufSize = 0;
	clrStaticBack = -1;
	hStFilter = NULL;
	hTap = NULL;
	hFilter = NULL;
	hStatusBar = NULL;
	hUseRegExp = NULL;
	hThread = NULL;
	bServerStarted = FALSE;
	hWebLogicProc = NULL;
	hJava = NULL;
	hWebLogicStopProc = NULL;
	dwOpenWndStyle = SW_SHOW;
	dwWebLogicPID = 0;
	bTransparent = FALSE;
	ZeroMemory(szWebLogicStartCmd, sizeof(szWebLogicStartCmd));
	ZeroMemory(szWebLogicStopCmd, sizeof(szWebLogicStopCmd));
	ZeroMemory(cpuTimes, sizeof(cpuTimes));
	ZeroMemory(javaTimes, sizeof(javaTimes));
	iCurTime = 0;
	uTrans = 100;
}

CMainWnd::~CMainWnd(void)
{
	OnClickForceShutdown();
	StopFileCheck();
}

LRESULT CMainWnd::OnCreate(LPCREATESTRUCT lpcs)
{
	INITCOMMONCONTROLSEX icce = {0};
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_WIN95_CLASSES | ICC_INTERNET_CLASSES | ICC_BAR_CLASSES | ICC_TAB_CLASSES;
	InitCommonControlsEx(&icce);

	hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0, SHIFTJIS_CHARSET, 3, 2, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("ＭＳ ゴシック"));
	SendMessage(this->GetHWnd(), WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

	hTap = CreateWindowEx(0, WC_TABCONTROL, _T(""),
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_CLIPCHILDREN | 
		TCS_HOTTRACK | TCS_RIGHTJUSTIFY | TCS_FOCUSNEVER, 
        SPACE, SPACE, 200, 100, this->GetHWnd(), (HMENU)IDC_TB_MAINTAB, GethInstance(), NULL);
	AddToTab(_T("全体"));
	AddToTab(_T("フィルタ"));
	
	hStFilter = CreateWindowEx(0, _T("Static"), _T("フィルタ"), WS_CHILD,
		SPACE + 10, SPACE + 30, 48, 13, this->GetHWnd(), (HMENU)-1, GethInstance(), NULL);
	hFilter = CreateWindowEx(WS_EX_CLIENTEDGE, _T("Edit"), _T(""), 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | 
		ES_LEFT,
		80, SPACE + 30, 200, 16, this->GetHWnd(), (HMENU)IDC_ED_FILTER, GethInstance(), NULL);
	hUseRegExp = CreateWindowEx(WS_EX_TRANSPARENT, _T("Button"), _T("正規表現使用"), WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP,
		295, SPACE + 31, 88, 13, this->GetHWnd(), (HMENU)IDC_CB_USEREGEXP, GethInstance(), NULL);
	edTotal.Create(SPACE + 10, SPACE + 30, 200, 100, this);
	edFilter.Create(SPACE + 10 , SPACE + 55, 200, 100, this);
	SendMessage(hTap, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(hFilter, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(hUseRegExp, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(hStFilter, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	edTotal.SetFont(hFont);
	edTotal.SetLimit(0xffffffff);
	edFilter.SetFont(hFont);
	edFilter.SetLimit(0xffffffff);

	hStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, NULL, this->GetHWnd(), STATUSBAR);
	SendMessage(hStatusBar, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	int Statwidths[] = {210, 350, 500, -1};
	
	SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)(sizeof(Statwidths)/sizeof(int)), (LPARAM)Statwidths);

	StatusBar_SetPartText(hStatusBar, 0, _T("WebLogic:"));
	StatusBar_SetPartText(hStatusBar, 1, _T("CPU 使用率:"));

	hMutex = CreateMutex(NULL, FALSE, DATAMUTEX);
	SetTimer(this->GetHWnd(), GETDATATIMER, TIMERINTERVAL, NULL);
	SetTimer(this->GetHWnd(), SVRCHKTIMER, SVRCHKINTERVAL, NULL);
	SetTimer(this->GetHWnd(), CHKCPUUSE, CHKCPUINTERVAL, NULL);
	return 0;
}

BOOL CMainWnd::AddToTab(LPCTSTR szTabTitle) 
{ 
    TCITEM tie; 
	
    tie.mask = TCIF_TEXT | TCIF_IMAGE; 
    tie.iImage = -1; 
    tie.pszText = (LPTSTR)szTabTitle; 
	
    return !(TabCtrl_InsertItem(hTap, TabCtrl_GetItemCount(hTap), &tie) == -1);
}

void CMainWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if(!this->LoadInitData())
	{
		this->MoveToParentCenter();
		return;
	}

	HMENU hMenu = GetMenu(this->GetHWnd());
	CheckMenuItem(hMenu, IDM_TOP, MF_BYCOMMAND | (bTop ? MF_CHECKED : MF_UNCHECKED));
	::SetWindowPos(this->GetHWnd(), bTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	rcInitWnd.right -= rcInitWnd.left;
	rcInitWnd.bottom -= rcInitWnd.top;
	this->SetWindowPos(NULL, rcInitWnd.left, rcInitWnd.top, rcInitWnd.right, rcInitWnd.bottom, SWP_NOZORDER);
	switch(dwOpenWndStyle)
	{
	case SW_MAXIMIZE:
		ShowWindow(SW_MAXIMIZE);
		break;
	case SW_MINIMIZE:
		ShowWindow(SW_MINIMIZE);
		break;
	}
}

void CMainWnd::OnCommand(UINT uNotify, UINT uId, HWND hwndCtrl)
{
	switch(uId)
	{
	case IDM_LOGOPEN:
		this->OpenLogFile();
		break;
	case IDC_ED_FILTER:
		if (uNotify == EN_CHANGE)
			this->OnChangeFilter();
		break;
	case IDC_CB_USEREGEXP:
		edFilter.SetUseRegExp(Button_IsChecked(hUseRegExp));
		OnChangeFilter();
		break;
	case IDA_CTRL_A:
		this->OnSelectAll();
		break;
	case IDA_CTRL_C:
		this->OnCopyToClibBoard();
		break;
	case IDM_CLEARLOG:
		this->ClearLog();
		break;
	case IDM_TOP:
		this->SetTop();
		break;
	case IDM_WEBLOGIC_START:
		this->OnClickStart();
		break;
	case IDM_WEBLOGIC_FORCESHUTDOWN:
		this->OnClickForceShutdown();
		break;
	case IDM_WNDTRANSPARENT:
		this->OnClickSetTransparent();
		break;
	case IDM_LOGCLOSE:
		this->StopFileCheck();
		break;
	case IDM_OPTION:
		this->OpenOption();
		break;
	case IDM_LOG_ALLSEL:
		MessageBox(_T("AllSEl!"));
		break;
	case IDM_CLOSE:
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;
	}
}

void CMainWnd::OnNotify(LPNMHDR lpnmh)
{
	switch(lpnmh->code)
	{
	case TCN_SELCHANGE:
		OnTabChange();
		break;
	}
}

void CMainWnd::OnTabChange()
{
	RECT rcWnd;
	GetClientRect(&rcWnd);
	OnSize(rcWnd.right, rcWnd.bottom);
}

void CMainWnd::OnTimer(UINT uId)
{
	switch(uId)
	{
	case GETDATATIMER:
		this->GetBuffDataWorker();
		break;
	case SVRCHKTIMER:
		SeeWebServerStatus();
		break;
	case CHKCPUUSE:
		SeeCpuUseRate();
		break;
	}
}

void CMainWnd::GetBuffDataWorker()
{
	if (IsEmpty())
		return;
	KillTimer(this->GetHWnd(), GETDATATIMER);
	PCHAR data = GetListData();
	if (data != NULL)
	{
		UINT bufSiz = strlen(data) * 2;
		PCHAR pBuff = (PCHAR)calloc(bufSiz, 1);
		UINT len = 0;
		while(data)
		{
			UINT datalen = lstrlen(data);
			if(bufSiz <= datalen + len)
			{
				UINT tmplen = bufSiz + (datalen * 2);
				PCHAR tmp = (PCHAR)realloc(pBuff, tmplen);
				if (tmp) 
				{
					pBuff = tmp;
					bufSiz = tmplen;
				}
				else
				{
					break;
				}
			}
			strcpy(&pBuff[len], data);
			len += datalen;
			free(data);
			data = GetListData();
		}
		
		this->addtoEdit(pBuff);
		free(pBuff);
		if (data) {
			OutputDebugString("<=================== data is Not Null!!! ====================>\n");
			this->addtoEdit(data);
			free(data);
		}
	}
	SetTimer(this->GetHWnd(), GETDATATIMER, TIMERINTERVAL, NULL);
}

void CMainWnd::SeeWebServerStatus()
{
	__try
	{
		KillTimer(this->GetHWnd(), SVRCHKTIMER);
		if (!hWebLogicProc)
		{
			StatusBar_SetPartText(hStatusBar, 0, _T("WebLogic：停止"));
			return;
		}
		if (!hJava)
		{
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			
			PROCESSENTRY32 pe32 = {0};
			pe32.dwSize = sizeof(pe32);
			
			char tmp[MAX_PATH];
			
			if (Process32First(hSnap, &pe32)) {
				do {
					if (pe32.th32ParentProcessID == dwWebLogicPID)
					{
						sprintf(tmp, "Found it! PID=%d PPID=%d exeName=[%s]\r\n", pe32.th32ProcessID, pe32.th32ParentProcessID, pe32.szExeFile);
						this->addtoEdit(tmp);
						if(!(hJava = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID)))
						{
							return;
						}
						DWORD dwExitCode;
						GetExitCodeProcess(hJava, &dwExitCode);
						if (dwExitCode == STILL_ACTIVE) {
							break;
						} else {
							CloseHandle(hJava);
							hJava =NULL;
						}
					}
				} while(Process32Next(hSnap, &pe32));
			} else {
				ErrMessageBox();
			}
			
			CloseHandle(hSnap);
			if (!hJava)
				return;
		}
		DWORD dwExitCode;
		GetExitCodeProcess(hJava, &dwExitCode);
		if (dwExitCode != STILL_ACTIVE) {
			CloseHandle(hJava);
			hJava =NULL;
			return;
		}
		
		PROCESS_MEMORY_COUNTERS pmc = {0};
		pmc.cb = sizeof(pmc);
		if (GetProcessMemoryInfo( hJava, &pmc, sizeof(pmc)))
		{
			char tmp[256];
			char num[256];
			_sntprintf(num, _countof(num), _T("%d"), pmc.WorkingSetSize / 1024);

			EditComma(num, _countof(num));

			_sntprintf(tmp, _countof(tmp), _T("WebLogic メモリ使用量: %s K"), num);
			StatusBar_SetPartText(hStatusBar, 0, tmp);
		}

	}
	__finally
	{
		SetTimer(this->GetHWnd(), SVRCHKTIMER, SVRCHKINTERVAL, NULL);
	}
}

void CMainWnd::SeeCpuUseRate()
{
	__try
	{
		KillTimer(this->GetHWnd(), CHKCPUUSE);
		FILETIME siTime, skTime, suTime;
		static __int64 lasti64si, lasti64sk, lasti64su;
		__int64 i64si, i64sk, i64su;
		if(!GetSystemTimes(&siTime, &skTime, &suTime))
		{
			ErrMessageBox();
			return;
		}
		memcpy(&i64si, &siTime, sizeof(i64si));
		memcpy(&i64sk, &skTime, sizeof(i64sk));
		memcpy(&i64su, &suTime, sizeof(i64su));
		
		__int64 usrTime = i64su - lasti64su;
		__int64 kerTime = i64sk - lasti64sk;
		__int64 idlTime = i64si - lasti64si;
		__int64 sysTime = usrTime + kerTime;
		lasti64su = i64su;
		lasti64sk = i64sk;
		lasti64si = i64si;
		
		cpuTimes[iCurTime] = ((double)(sysTime - idlTime) / sysTime);

		if (hJava)
		{
			FILETIME cTime, eTime, kTime, uTime;
			__int64 i64c, i64e, i64k, i64u;
			static __int64 lasti64k, lasti64u;
			if(!GetProcessTimes(hJava, &cTime, &eTime, &kTime, &uTime))
			{
				return;
			}
			memcpy(&i64c, &cTime, sizeof(i64c));
			memcpy(&i64e, &eTime, sizeof(i64e));
			memcpy(&i64k, &kTime, sizeof(i64k));
			memcpy(&i64u, &uTime, sizeof(i64u));

			__int64 javaTime = (i64k - lasti64k) + (i64u - lasti64u);
			double java = (double)(javaTime) / (sysTime);
			if (java < 0)
				java = 0;
			javaTimes[iCurTime] = java;

			lasti64k = i64k;
			lasti64u = i64u;
		}

		iCurTime = iCurTime + 1;
		if (iCurTime < 10)
			return;

		double totalCpu = 0;
		for(int i = 0; i < 10; i++)
			totalCpu += cpuTimes[i];
		UINT arrCpu = (UINT)((totalCpu / 10.0) * 100.0);

		TCHAR systemCpu[25];
		_sntprintf(systemCpu, _countof(systemCpu), _T("%d%%"), arrCpu);
		TCHAR javaCpu[25];
		if (hJava)
		{
			double totalJava = 0;
			for(i = 0; i < 10; i++)
				totalJava += javaTimes[i];
			UINT arrJava = (UINT)((totalJava / 10.0) * 100.0);
			
			_sntprintf(javaCpu, _countof(javaCpu), _T("(%d%%)"), arrJava);
		}
		else
			javaCpu[0] = 0;
		
		char tmp[256];
		_sntprintf(tmp, _countof(tmp), _T("CPU 使用率: %s%s"), systemCpu, javaCpu);
		StatusBar_SetPartText(hStatusBar, 1, tmp);
		
		iCurTime = 0;
		ZeroMemory(cpuTimes, sizeof(cpuTimes));
		ZeroMemory(javaTimes, sizeof(javaTimes));
	}
	__finally
	{
		SetTimer(this->GetHWnd(), CHKCPUUSE, CHKCPUINTERVAL, NULL);
	}
}

void CMainWnd::OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
{
	lpMMI->ptMinTrackSize.x = 475;
	lpMMI->ptMinTrackSize.y = 200;
}

LRESULT CMainWnd::OnCtlColorStatic(HDC hStaticDC, HWND hStatic)
{
	if (clrStaticBack == -1) 
	{
		HDC hTapDC = GetDC(hTap);
		SetBkMode(hStaticDC, TRANSPARENT);
		clrStaticBack = GetBkColor(hTapDC);
		ReleaseDC(hTap, hTapDC);
	}
	return (LONG)clrStaticBack;
}

void CMainWnd::OnChangeFilter()
{
	CHAR tmp[256];
	GetWindowText(hFilter, tmp, _countof(tmp));

	if (edFilter.SetFilter(tmp))
		StatusBar_SetPartText(hStatusBar, 3, _T(""));
	else
		StatusBar_SetPartText(hStatusBar, 3, _T("正規表現が正しくありません。"));
}

void CMainWnd::OnSelectAll()
{
	int iCurTab = TabCtrl_GetCurSel(hTap);
	if (iCurTab == 0)
		edTotal.SelectAll();
	else
		edFilter.SelectAll();
}

void CMainWnd::OnCopyToClibBoard()
{
	int iCurTab = TabCtrl_GetCurSel(hTap);
	if (iCurTab == 0)
		edTotal.OnCopyToClibBoard();
	else
		edFilter.OnCopyToClibBoard();
}

void CMainWnd::OnSize(UINT width, UINT height)
{
	RECT rtRect, rtWnd;
	SendMessage(hStatusBar, WM_SIZE, 0, 0);

	::GetWindowRect(hStatusBar, &rtRect);
	rtRect.bottom -= rtRect.top;
	RECT rcFilter;

	::SetWindowPos(hTap, 0, 0, 0, width - SPACE * 2, height - SPACE * 2 - rtRect.bottom, SWP_NOZORDER | SWP_NOMOVE);
	int iCurTab = TabCtrl_GetCurSel(hTap);
	switch(iCurTab)
	{
	case 0:
		edTotal.ShowWindow(SW_SHOW);
		::ShowWindow(hStFilter, SW_HIDE);
		edFilter.ShowWindow(SW_HIDE);
		::ShowWindow(hUseRegExp, SW_HIDE);
		::ShowWindow(hFilter, SW_HIDE);
		edTotal.SetWindowPos(NULL, 0, 0, width - SPACE * 4, height - SPACE * 4 - 20 - rtRect.bottom, SWP_NOZORDER | SWP_NOMOVE);
		break;
	case 1:
		edTotal.ShowWindow(SW_HIDE);
		::ShowWindow(hStFilter, SW_SHOW);
		edFilter.ShowWindow(SW_SHOW);
		::ShowWindow(hUseRegExp, SW_SHOW);
		::ShowWindow(hFilter, SW_SHOW);
		::GetWindowRect(hFilter, &rcFilter);
		rcFilter.right -= rcFilter.left;
		rcFilter.bottom -= rcFilter.top;
		::ScreenToClient(this->hTap, (LPPOINT)&rcFilter);
		edFilter.SetWindowPos(NULL, 0, 0, width - SPACE * 4, height - SPACE * 4 - 45 - rtRect.bottom, SWP_NOZORDER | SWP_NOMOVE);
		break;
	}

	TCHAR tmp[100];
	::GetWindowRect(this->GetHWnd(), &rtWnd);
	rtWnd.right -= rtWnd.left;
	rtWnd.bottom -= rtWnd.top;
	_sntprintf(tmp, _countof(tmp), _T("%d,%d(%dx%d)"), rtWnd.left, rtWnd.top, rtWnd.right, rtWnd.bottom);
	StatusBar_SetPartText(hStatusBar, 2, tmp);
}

void CMainWnd::OnMoving(LPRECT lpRect)
{
	TCHAR tmp[100];
	RECT rtWnd;
	::GetWindowRect(this->GetHWnd(), &rtWnd);
	rtWnd.right -= rtWnd.left;
	rtWnd.bottom -= rtWnd.top;
	_sntprintf(tmp, _countof(tmp), _T("%d,%d(%dx%d)"), rtWnd.left, rtWnd.top, rtWnd.right, rtWnd.bottom);
	StatusBar_SetPartText(hStatusBar, 2, tmp);
}

void CMainWnd::OnMove(UINT width, UINT height)
{
	TCHAR tmp[100];
	RECT rtWnd;
	::GetWindowRect(this->GetHWnd(), &rtWnd);
	rtWnd.right -= rtWnd.left;
	rtWnd.bottom -= rtWnd.top;
	_sntprintf(tmp, _countof(tmp), _T("%d,%d(%dx%d)"), rtWnd.left, rtWnd.top, rtWnd.right, rtWnd.bottom);
	StatusBar_SetPartText(hStatusBar, 2, tmp);
}

void CMainWnd::OnClose(void)
{
	SaveInitData();
	KillTimer(this->GetHWnd(), GETDATATIMER);
	OnClickForceShutdown();
	CloseHandle(hMutex);
	PostQuitMessage(0);
}

void CMainWnd::OpenLogFile(void)
{
	OPENFILENAME ofn;
	ZeroMemory(filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetHWnd();
	ofn.lpstrFilter = _T("ログファイル(*.log)\0*.log\0テキストファイル(*.txt)\0*.txt\0全てのファイル(*.*)\0*.*\0");
	ofn.lpstrFile = filename;
	ofn.nMaxFile = _countof(filename);
	ofn.lpstrTitle = _T("ログファイルを指定して下さい。");
	int res = GetOpenFileName(&ofn);
	if (!res)
	{
		//ErrMessageBox();
		return;
	}

	StopFileCheck();
	HMENU hMenu = GetMenu(this->GetHWnd());
	EnableMenuItem(hMenu, IDM_LOGCLOSE, MF_BYCOMMAND | MF_ENABLED);
	bCheckFile = TRUE;
	this->hThread = (HANDLE)_beginthreadex(NULL, 0, _FileCheckFunc, (PVOID)this, 0, (PUINT)&dwThreadId);
}

void CMainWnd::addtoBuff(PCHAR tmp, long len)
{
	for (long i = 0; i < len; i++) {
		buff[this->uBufSize] = tmp[i];
		if (buff[this->uBufSize] == '\n' && buff[this->uBufSize - 1]) {
			AddToDataNoMutex((PCHAR)buff, this->uBufSize + 1);
			ZeroMemory(buff, sizeof(buff));
			uBufSize = 0;
		} else {
			uBufSize++;
		}
	}
}

void CMainWnd::addtoEdit(PCHAR buff)
{
	edTotal.AddText(buff);
	edFilter.AddToEditByCheckFilter(buff);
}

void CMainWnd::ClearLog()
{
	int iCurTab = TabCtrl_GetCurSel(hTap);
	if (iCurTab == 0)
		edTotal.Clear();
	else
		edFilter.Clear();
}

void CMainWnd::SetTop()
{
	HMENU hMenu = GetMenu(this->GetHWnd());
	bTop = !bTop;
	CheckMenuItem(hMenu, IDM_TOP, MF_BYCOMMAND | (bTop ? MF_CHECKED : MF_UNCHECKED));
	::SetWindowPos(this->GetHWnd(), bTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void CMainWnd::ErrMessageBox() {
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 
	
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
	
    ::MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK); 
	
    LocalFree(lpMsgBuf);
}

UINT CALLBACK CMainWnd::_FileCheckFunc(PVOID arg)
{
	return ((CMainWnd*)arg)->FileCheckFunc();
}

UINT CMainWnd::FileCheckFunc()
{
	HANDLE hLogFile = INVALID_HANDLE_VALUE;
	__try
	{
		OutputDebugString("StartThread!\n");
		StopFileCheck();
		OutputDebugString(filename);
		hLogFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (hLogFile == INVALID_HANDLE_VALUE)
		{
			ErrMessageBox();
			return -1;
		}
		SetMainWndTitle(filename);
		OutputDebugString("File Opend!\n");
		bRunThread = TRUE;

		DWORD dwFileSizeH;
		this->uCurSize = GetFileSize(hLogFile, &dwFileSizeH);
		SetFilePointer(hLogFile, 0, 0, FILE_END);

		while (bCheckFile)
		{
			DWORD dwFileSizeH;
			DWORD curSize = GetFileSize(hLogFile, &dwFileSizeH);
			if (curSize != this->uCurSize)
			{
				WaitForSingleObject(hMutex, INFINITE);
				char tmp[65535];
				DWORD dwRead;
				while(ReadFile(hLogFile, tmp, sizeof(tmp), &dwRead, NULL))
				{
					if (dwRead == 0)
						break;
					tmp[dwRead] = '\0';
					this->addtoBuff(tmp, dwRead);
				}
				this->uCurSize = GetFileSize(hLogFile, &dwFileSizeH);;
				ReleaseMutex(hMutex);
			}
			Sleep(1);
		}		
	}
	__finally
	{
		CloseHandle(hLogFile);
		CloseHandle(hThread);
		hThread = NULL;
		bCheckFile = FALSE;
		bRunThread = FALSE;
		OutputDebugString("EndThread!\n");
	}
	return 0;
}

void CMainWnd::StopFileCheck()
{
	if(!bRunThread)
		return;

	HMENU hMenu = GetMenu(this->GetHWnd());
	EnableMenuItem(hMenu, IDM_LOGCLOSE, MF_BYCOMMAND | MF_GRAYED);

	bCheckFile = FALSE;
	while(bRunThread)
		Sleep(1);
	SetMainWndTitle(NULL);
}

void CMainWnd::OnClickStart()
{
	if (!bServerStarted)
	{
		CloseHandle((HANDLE)_beginthreadex(NULL, 0, _StartWebLogic, (PVOID)this, 0, (PUINT)&dwThreadId));
	}
	else
	{
		HMENU hMenu = GetMenu(this->GetHWnd());
		EnableMenuItem(hMenu, IDM_WEBLOGIC_START, MF_BYCOMMAND | MF_GRAYED);
		CloseHandle((HANDLE)_beginthreadex(NULL, 0, _StopWebLogic, (PVOID)this, 0, (PUINT)&dwThreadId));
	}
}

UINT CALLBACK CMainWnd::_StartWebLogic(PVOID arg)
{
	return ((CMainWnd*)arg)->StartWebLogic();
}

UINT CALLBACK CMainWnd::_StopWebLogic(PVOID arg)
{
	return ((CMainWnd*)arg)->StopWebLogic();
}

UINT CMainWnd::StartWebLogic()
{
	BOOL bFlag;
	HANDLE hRead, hWrite;
	HANDLE hInputRead, hInputWrite;
	
	SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
	
    bFlag = CreatePipe(&hRead, &hWrite, &sa, 0);
    if (!bFlag)
    {
		MessageBoxW(this->m_hWnd, L"Fail to open pipe.", L"警告", MB_OK);
		return -1;
    }
	
	bFlag = CreatePipe(&hInputRead, &hInputWrite, &sa, 0);
	if(!bFlag)
	{
		MessageBoxW(this->m_hWnd, L"Fail to open input pipe.", L"警告", MB_OK);
		return -2;
	}
	
    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
	si.hStdInput = hInputRead;
    PROCESS_INFORMATION pi;

	TCHAR strExecFileName[MAX_PATH * 2];
	_sntprintf(strExecFileName, _countof(strExecFileName), _T("%s\"%s\" 2>&1"), CMDLINE, this->szWebLogicStartCmd);
	
	bFlag = CreateProcess(NULL, strExecFileName, 
		NULL, NULL, TRUE, CREATE_NO_WINDOW/*CREATE_NEW_CONSOLE*/, NULL, NULL, &si, &pi);   
	if(!bFlag) 
	{
		ErrMessageBox();
		return -3;
	}
	CloseHandle(hWrite);
	CloseHandle(hInputRead);

	bServerStarted = TRUE;
	SetMainWndTitle(NULL);

	HMENU hMenu = GetMenu(this->GetHWnd());
	ModifyMenu(hMenu, IDM_WEBLOGIC_START, MF_BYCOMMAND | MF_STRING, IDM_WEBLOGIC_START, _T("Stop"));	
	EnableMenuItem(hMenu, IDM_WEBLOGIC_FORCESHUTDOWN, MF_BYCOMMAND | MF_ENABLED);
	SetMainWndTitle(_T("[WebLogicサーバー Running]"));

	this->addtoEdit(strExecFileName);
	this->addtoEdit(_T("\r\n"));
	hWebLogicProc = pi.hProcess;
	dwWebLogicPID = pi.dwProcessId;

	TCHAR tmp[50];
	sprintf(tmp, "PID=%d TID=%d\r\n", pi.dwProcessId, pi.dwThreadId);
	this->addtoEdit(tmp);
	
	DWORD BytesRead;
	char buffer[512];   
	
	while(ReadFile(hRead, buffer, sizeof(buffer)-1, &BytesRead, NULL) && BytesRead)   
	{
		WaitForSingleObject(hMutex, INFINITE);
		this->addtoBuff(buffer, BytesRead);
		ReleaseMutex(hMutex);
	}   
	CloseHandle(hRead);
	CloseHandle(hInputWrite);

	while (hWebLogicStopProc)
		Sleep(1);
		
	DWORD dwExitCode;
	do
	{
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
		Sleep(1);
	} while(dwExitCode == STILL_ACTIVE);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	hWebLogicProc = NULL;
	if (hJava)
	{
		CloseHandle(hJava);
		hJava = NULL;
	}
	dwWebLogicPID = 0;
	EnableMenuItem(hMenu, IDM_WEBLOGIC_START, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_WEBLOGIC_FORCESHUTDOWN, MF_BYCOMMAND | MF_GRAYED);
	ModifyMenu(hMenu, IDM_WEBLOGIC_START, MF_BYCOMMAND | MF_STRING, IDM_WEBLOGIC_START, _T("Start"));
	SetMainWndTitle(NULL);
	bServerStarted = FALSE;

	this->addtoEdit(_T("=======================>WebLogic Server Stoped!<=======================\r\n"));
	return 0;
}

UINT CMainWnd::StopWebLogic()
{
	BOOL bFlag;
	HANDLE hRead, hWrite;
	HANDLE hInputRead, hInputWrite;
	
	SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
	
    bFlag = CreatePipe(&hRead, &hWrite, &sa, 0);
    if (!bFlag)
    {
		MessageBoxW(this->m_hWnd, L"Fail to open pipe.", L"警告", MB_OK);
		return -1;
    }
	
	bFlag = CreatePipe(&hInputRead, &hInputWrite, &sa, 0);
	if(!bFlag)
	{
		MessageBoxW(this->m_hWnd, L"Fail to open input pipe.", L"警告", MB_OK);
		return -2;
	}
	
    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
	si.hStdInput = hInputRead;
    PROCESS_INFORMATION pi;

	TCHAR strExecFileName[MAX_PATH * 2];
	_sntprintf(strExecFileName, _countof(strExecFileName), _T("%s\"%s\" 2>&1"), CMDLINE, this->szWebLogicStopCmd);
	
	bFlag = CreateProcess(NULL, strExecFileName, 
		NULL, NULL, TRUE, CREATE_NO_WINDOW/*CREATE_NEW_CONSOLE*/, NULL, NULL, &si, &pi);   
	if(!bFlag) 
	{
		ErrMessageBox();
		return -3;
	}
	CloseHandle(hWrite);
	CloseHandle(hInputRead);
	
	this->addtoEdit(strExecFileName);
	this->addtoEdit(_T("\r\n"));
	hWebLogicStopProc = pi.hProcess;
	
	DWORD BytesRead;
	char buffer[512];   
	
	while(ReadFile(hRead, buffer, sizeof(buffer)-1, &BytesRead, NULL) && BytesRead)   
	{   
		WaitForSingleObject(hMutex, INFINITE);
		this->addtoBuff(buffer, BytesRead);
		ReleaseMutex(hMutex);
	}   
	CloseHandle(hRead);
	CloseHandle(hInputWrite);
	
	DWORD dwExitCode;
	do
	{
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
		Sleep(1);
	} while(dwExitCode == STILL_ACTIVE);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	hWebLogicStopProc = NULL;
	
	bServerStarted = FALSE;
	return 0;
}

void CMainWnd::OnClickForceShutdown()
{
	if(!hWebLogicProc)
		return;

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe32 = {0};
	pe32.dwSize = sizeof(pe32);
	char tmp[MAX_PATH];
	HANDLE hJava = NULL;

	if (Process32First(hSnap, &pe32)) {
		do {
			if (pe32.th32ParentProcessID == dwWebLogicPID)
			{
				sprintf(tmp, "PID=%d PPID=%d exeName=[%s]\r\n", pe32.th32ProcessID, pe32.th32ParentProcessID, pe32.szExeFile);
				this->addtoEdit(tmp);
				if(!(hJava = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID)))
				{
					ErrMessageBox();
				}
				DWORD dwExitCode;
				GetExitCodeProcess(hJava, &dwExitCode);
				if (dwExitCode == STILL_ACTIVE) {
					break;
				} else {
					CloseHandle(hJava);
					hJava =NULL;
				}
			}
		} while(Process32Next(hSnap, &pe32));
	} else {
		ErrMessageBox();
	}

	CloseHandle(hSnap);
	if (hJava)
	{
		this->addtoEdit(_T("Weblogic Serverプロセス 強制終了！\r\n"));
		if(!TerminateProcess(hJava, -1))
		{
			ErrMessageBox();
		}
		CloseHandle(hJava);
		hJava = NULL;
	}

	if(!TerminateProcess(hWebLogicProc, -1))
	{
		ErrMessageBox();
	}
	while(hWebLogicProc)
		Sleep(1);
}

void CMainWnd::OnClickSetTransparent()
{
	bTransparent = !bTransparent;
	HMENU hMenu = GetMenu(this->GetHWnd());
	CheckMenuItem(hMenu, IDM_WNDTRANSPARENT, MF_BYCOMMAND | (bTransparent ? MF_CHECKED : MF_UNCHECKED));
	
	if (bTransparent)
	{
		::SetWindowLong(this->GetHWnd(), GWL_EXSTYLE, GetWindowLong(this->GetHWnd(), GWL_EXSTYLE) | WS_EX_LAYERED);
		//::SetLayeredWindowAttributes(this->GetHWnd(), RGB(0,0,1),0,LWA_COLORKEY);
		SetTrans(uTrans);
	}
	else
	{
		::SetWindowLong(this->GetHWnd(), GWL_EXSTYLE, GetWindowLong(this->GetHWnd(), GWL_EXSTYLE) & ~WS_EX_LAYERED);
	}
}

#define REGSUBKEY	_T("Software\\LogViewer")
#define TOP			_T("TOP")
#define LEFT		_T("LEFT")
#define BOTTOM		_T("BOTTOM")
#define RIGHT		_T("RIGHT")
#define WNDSTATUS	_T("WNDSTATUS")
#define TOPMOST		_T("TOPMOST")
#define WEBSTARTCMD	_T("WebStartCmd")
#define WEBSTOPCMD	_T("WebStopCmd")
#define USEREGEXP	_T("UseRegExp")

BOOL CMainWnd::LoadInitData()
{
	HKEY hKey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSUBKEY, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	ZeroMemory(&rcInitWnd, sizeof(rcInitWnd));
	DWORD dwType = REG_DWORD;
	DWORD dwDataSize = sizeof(DWORD);
	__try
	{
		if(RegQueryValueEx(hKey, TOP, NULL, &dwType, (PBYTE)&rcInitWnd.top, &dwDataSize) != ERROR_SUCCESS)
			return FALSE;
		if(RegQueryValueEx(hKey, LEFT, NULL, &dwType, (PBYTE)&rcInitWnd.left, &dwDataSize) != ERROR_SUCCESS)
			return FALSE;
		if(RegQueryValueEx(hKey, BOTTOM, NULL, &dwType, (PBYTE)&rcInitWnd.bottom, &dwDataSize) != ERROR_SUCCESS)
			return FALSE;
		if(RegQueryValueEx(hKey, RIGHT, NULL, &dwType, (PBYTE)&rcInitWnd.right, &dwDataSize) != ERROR_SUCCESS)
			return FALSE;
		if(RegQueryValueEx(hKey, TOPMOST, NULL, &dwType, (PBYTE)&bTop, &dwDataSize) != ERROR_SUCCESS)
			return FALSE;
		if(RegQueryValueEx(hKey, WNDSTATUS, NULL, &dwType, (PBYTE)&dwOpenWndStyle, &dwDataSize) != ERROR_SUCCESS)
			return FALSE;
		dwType = REG_SZ;
		dwDataSize = _countof(szWebLogicStartCmd);
		if(RegQueryValueEx(hKey, WEBSTARTCMD, NULL, &dwType, (PBYTE)szWebLogicStartCmd, &dwDataSize) != ERROR_SUCCESS)
			return FALSE;
		dwDataSize = _countof(szWebLogicStopCmd);
		if(RegQueryValueEx(hKey, WEBSTOPCMD, NULL, &dwType, (PBYTE)szWebLogicStopCmd, &dwDataSize) != ERROR_SUCCESS)
			return FALSE;
	}
	__finally
	{
		RegCloseKey(hKey);
	}
		
	return TRUE;
}

BOOL CMainWnd::SaveInitData()
{
	HKEY hKey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSUBKEY, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		if(RegCreateKey(HKEY_LOCAL_MACHINE, REGSUBKEY, &hKey) != ERROR_SUCCESS)
		{
			ErrMessageBox();
			return FALSE;
		}
	}
	WINDOWPLACEMENT wndpl = {0};
	wndpl.length = sizeof(wndpl);

	GetWindowPlacement(this->GetHWnd(), &wndpl);
	memcpy(&rcInitWnd, &wndpl.rcNormalPosition, sizeof(rcInitWnd));
	dwOpenWndStyle = SW_SHOW;
	if (IsIconic(this->GetHWnd())) {
		dwOpenWndStyle = SW_MINIMIZE;
	} else if (IsZoomed(this->GetHWnd())) {
		dwOpenWndStyle = SW_MAXIMIZE;
	}

	RegSetValueEx(hKey, TOP, 0, REG_DWORD, (PBYTE)&rcInitWnd.top, REG_DWORD);
	RegSetValueEx(hKey, LEFT, 0, REG_DWORD, (PBYTE)&rcInitWnd.left, REG_DWORD);
	RegSetValueEx(hKey, BOTTOM, 0, REG_DWORD, (PBYTE)&rcInitWnd.bottom, REG_DWORD);
	RegSetValueEx(hKey, RIGHT, 0, REG_DWORD, (PBYTE)&rcInitWnd.right, REG_DWORD);
	RegSetValueEx(hKey, TOPMOST, 0, REG_DWORD, (PBYTE)&bTop, REG_DWORD);
	RegSetValueEx(hKey, WNDSTATUS, 0, REG_DWORD, (PBYTE)&dwOpenWndStyle, REG_DWORD);
	RegSetValueEx(hKey, WEBSTARTCMD, 0, REG_SZ, (PBYTE)szWebLogicStartCmd, lstrlen(szWebLogicStartCmd) + 1);
	RegSetValueEx(hKey, WEBSTOPCMD, 0, REG_SZ, (PBYTE)szWebLogicStopCmd, lstrlen(szWebLogicStopCmd) + 1);
	RegCloseKey(hKey);
	return TRUE;
}

UINT CMainWnd::GetTrans() const
{
	return uTrans;
}

void CMainWnd::SetTrans(UINT newTrans)
{
	uTrans = newTrans;
	if (bTransparent)
	{
		UINT trans = (UINT)((uTrans / 100.0) * 255.0);
		::SetLayeredWindowAttributes(this->GetHWnd(), RGB(0,0,0),trans,LWA_ALPHA);
	}
}

void CMainWnd::OpenOption()
{
	COptionWnd optWnd;
	optWnd.doModal(this);
}

void CMainWnd::SetMainWndTitle(LPTSTR SubInfo)
{
	TCHAR winTitle[256];
	if (SubInfo)
		_sntprintf(winTitle, _countof(winTitle), _T("%s:%s"), MAINTITLE, SubInfo);
	else
		_tcscpy(winTitle, MAINTITLE);
	SetWindowText(this->GetHWnd(), winTitle);
}

LPCTSTR CMainWnd::GetWebLogicStartCmd()
{
	return this->szWebLogicStartCmd;
}

void CMainWnd::SetWebLogicStartCmd(LPCTSTR szNewWebLogicStartCmd)
{
	_tcsncpy(this->szWebLogicStartCmd, szNewWebLogicStartCmd, _countof(this->szWebLogicStartCmd));
}

LPCTSTR CMainWnd::GetWebLogicStopCmd()
{
	return this->szWebLogicStopCmd;
}

void CMainWnd::SetWebLogicStopCmd(LPCTSTR szNewWebLogicStopCmd)
{
	_tcsncpy(this->szWebLogicStopCmd, szNewWebLogicStopCmd, _countof(this->szWebLogicStopCmd));
}

ATOM CMainWnd::Register(void)
{
	WNDCLASSEX wcex;
	
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = CWin::_WinProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = CWin::GethInstance();
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNSHADOW);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wcex.lpszClassName = MAINWNDCLASS;
	wcex.hIconSm = NULL;
	
	return ::RegisterClassEx(&wcex);
}

int CMainWnd::PumpMessage(void)
{
	BOOL bRet;
	MSG msg = {0};
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(this->GethInstance(), (LPCTSTR)IDR_ACCEL);
	
	while( (bRet = GetMessage( &msg, (HWND)NULL, 0, 0 )) != FALSE)
	{
		if (bRet == -1)
		{
			return -1;
		}
		else
		{
			//if(!IsDialogMessage(msg.hwnd, &msg)) {
				if( !TranslateAccelerator (msg.hwnd, hAccelTable, &msg) ) 
				{
					TranslateMessage(&msg); 
					DispatchMessage(&msg); 
				}
			//}
		}
	}
	return 0;
}

HWND CMainWnd::Create()
{
	if(ISNULL(this->Register()))
		return NULL;
	return CWin::Create(WS_EX_CONTROLPARENT, MAINWNDCLASS, MAINTITLE, WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, NULL, NULL);
}

void EditComma(LPTSTR src, UINT cchMax)
{
	UINT len = lstrlen(src);

	if (len < 4)
		return;

	UINT commaCnt = (int)(len / 3) - (len % 3 == 0 ? 1 : 0);

	LPTSTR tmp = (LPTSTR)_alloca(cchMax * sizeof(TCHAR));

	_tcscpy(tmp, src);
	int i, j, k;
	for (i = len, j = len + commaCnt, k = 0; i > -1; i--, j--, k++)
	{
		if (k != 1 && k % 3 == 1) {
			src[j] = ',';
			j--;
		}
		src[j] = tmp[i];
	}
}

int CALLBACK _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
	CMainWnd mainwnd;
	//mainwnd.doModal(NULL);
	mainwnd.Create();
	mainwnd.ShowWindow(SW_SHOW);
	mainwnd.PumpMessage();
	return 0;
}