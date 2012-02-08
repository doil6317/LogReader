#pragma once
#include "ListCtrl.h"
#include "LogBox.h"
class CMainWnd :
	public CWin
{
private:
	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	void OnShowWindow(BOOL bShow, UINT nStatus);
	void OnCommand(UINT uNotify, UINT uId, HWND hwndCtrl);
	void OnNotify(LPNMHDR lpnmh);
	void OnTabChange();
	void OnSize(UINT width, UINT height);
	void OnMoving(LPRECT lpRect);
	void OnMove(UINT width, UINT height);
	void OnTimer(UINT uId);
	void OnGetMinMaxInfo(LPMINMAXINFO lpMMI);
	LRESULT OnCtlColorStatic(HDC hStaticDC, HWND hStatic);
	void OnClose(void);
	void OpenLogFile(void);
	void ClearLog();
	void SetTop();
	void OnCheckFile();
	void OnClickStart();
	void OnChangeFilter();
	void OnSelectAll();
	void OnCopyToClibBoard();
	void addtoBuff(PCHAR tmp, long len);
	void addtoEdit(PCHAR buff);
	void ErrMessageBox();
	void OnClickForceShutdown();
	void OnClickSetTransparent();
	void SetMainWndTitle(LPTSTR SubInfo);
	void OpenOption();
	void GetBuffDataWorker();
	void SeeWebServerStatus();
	void SeeCpuUseRate();
	BOOL AddToTab(LPCTSTR szTabTitle);

	void StopFileCheck();
	static UINT CALLBACK _FileCheckFunc(PVOID arg); 
	UINT FileCheckFunc();

	UINT StartWebLogic();
	static UINT CALLBACK _StartWebLogic(PVOID arg);
	UINT StopWebLogic();
	static UINT CALLBACK _StopWebLogic(PVOID arg);

	ATOM Register(void);

	UINT uCurSize;
	BOOL bCheckFile;
	BOOL bRunThread;
	BOOL bTop;
	BOOL bTransparent;
	HANDLE hThread;
	DWORD dwThreadId;
	TCHAR filename[MAX_PATH];
	
	HWND hStFilter;
	HWND hTap;
	CLogBox edTotal;
	CLogBox edFilter;
	HWND hFilter;
	HWND hStatusBar;
	HWND hUseRegExp;
	HFONT hFont;
	COLORREF clrStaticBack; 

	HANDLE hWebLogicProc;
	HANDLE hJava;
	HANDLE hWebLogicStopProc;
	DWORD dwWebLogicPID;

	UINT uTrans;

	BYTE buff[65535];
	UINT uBufSize;

	TCHAR szWebLogicStartCmd[MAX_PATH];
	TCHAR szWebLogicStopCmd[MAX_PATH];

	DWORD dwOpenWndStyle;

	BOOL bServerStarted;

	RECT rcInitWnd;

	BOOL IsEmpty();
	void AddToDataNoMutex(PCHAR data, UINT siz);
	void AddToData(PCHAR data, UINT siz);
	PCHAR GetListData();

	BOOL LoadInitData();
	BOOL SaveInitData();

	double cpuTimes[10];
	double javaTimes[10];
	int iCurTime;
public:
	LPCTSTR GetWebLogicStartCmd();
	void SetWebLogicStartCmd(LPCTSTR szNewWebLogicStartCmd);
	LPCTSTR GetWebLogicStopCmd();
	void SetWebLogicStopCmd(LPCTSTR szNewWebLogicStopCmd);

	UINT GetTrans() const;
	void SetTrans(UINT newTrans);

	int PumpMessage(void);
	HWND Create();

	CMainWnd(void);
	virtual ~CMainWnd(void);
};