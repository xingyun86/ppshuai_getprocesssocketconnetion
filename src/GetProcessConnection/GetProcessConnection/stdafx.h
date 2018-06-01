// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER		0x0501
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0600
#define _RICHEDIT_VER	0x0300

#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
#include <atlapp.h>

extern CServerAppModule _Module;

// This is here only to tell VC7 Class Wizard this is an ATL project
#ifdef ___VC7_CLWIZ_ONLY___
CComModule
CExeModule
#endif

#include <atlcom.h>
#include <atlhost.h>
#include <atlwin.h>
#include <atlctl.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

__inline static void StartConsole()
{
	AllocConsole();
	_tfreopen(_T("CONOUT$"), _T("wb"), stderr);
	_tfreopen(_T("CONOUT$"), _T("wb"), stdout);
	_tfreopen(_T("CONIN$"), _T("rb"), stdin);
}
__inline static void CloseConsole()
{
	fclose(stderr);
	fclose(stdout);
	fclose(stdin);
	FreeConsole();
}
#include <vector>
#include <tchar.h>
#include <stdio.h>
#include <locale.h>
#include <windows.h>
#include <iphlpapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

typedef struct
{
	DWORD   dwState;        	// 连接状态
	DWORD   dwLocalAddr;    	// 本地地址
	DWORD   dwLocalPort;    	// 本地端口
	DWORD   dwRemoteAddr;   	// 远程地址
	DWORD   dwRemotePort;   	// 远程端口
	DWORD	  dwProcessId;		// 进程ID号
} MIB_TCPEXROW, *PMIB_TCPEXROW;

typedef struct
{
	DWORD			dwNumEntries;
	MIB_TCPEXROW	table[ANY_SIZE];
} MIB_TCPEXTABLE, *PMIB_TCPEXTABLE;

typedef struct
{
	DWORD   dwLocalAddr;    	// 本地地址
	DWORD   dwLocalPort;    	// 本地端口
	DWORD	  dwProcessId;		// 进程ID号
} MIB_UDPEXROW, *PMIB_UDPEXROW;

typedef struct
{
	DWORD			dwNumEntries;
	MIB_UDPEXROW	table[ANY_SIZE];
} MIB_UDPEXTABLE, *PMIB_UDPEXTABLE;


// 扩展函数原型
typedef DWORD(WINAPI *PFNAllocateAndGetTcpExTableFromStack)(
	PMIB_TCPEXTABLE *pTcpTable,
	BOOL bOrder,
	HANDLE heap,
	DWORD zero,
	DWORD flags
	);

typedef DWORD(WINAPI *PFNAllocateAndGetUdpExTableFromStack)(
	PMIB_UDPEXTABLE *pUdpTable,
	BOOL bOrder,
	HANDLE heap,
	DWORD zero,
	DWORD flags
	);
typedef DWORD(WINAPI *PFNGetExtendedTcpTable)(
	_Out_   PVOID           pTcpTable,
	_Inout_ PDWORD          pdwSize,
	_In_    BOOL            bOrder,
	_In_    ULONG           ulAf,
	_In_    TCP_TABLE_CLASS TableClass,
	_In_    ULONG           Reserved
);
typedef DWORD(WINAPI *PFNGetExtendedUdpTable)(
	_Out_   PVOID           pUdpTable,
	_Inout_ PDWORD          pdwSize,
	_In_    BOOL            bOrder,
	_In_    ULONG           ulAf,
	_In_    UDP_TABLE_CLASS TableClass,
	_In_    ULONG           Reserved
);

// 将进程ID号（PID）转化为进程名称
__inline static LPTSTR ProcessPidToName(LPTSTR ProcessName, DWORD ProcessId, HANDLE hSnap = NULL)
{
	PROCESSENTRY32 processEntry = { 0 };
	processEntry.dwSize = sizeof(processEntry);
	
	// 找不到的话，默认进程名为“???”
	lstrcpy(ProcessName, _T("???"));

	HANDLE hSnapAll = hSnap ? hSnap : CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapAll && hSnapAll != INVALID_HANDLE_VALUE)
	{
		if (!::Process32First(hSnapAll, &processEntry))
		{
			return ProcessName;
		}
		do
		{
			if (processEntry.th32ProcessID == ProcessId) // 就是这个进程
			{
				lstrcpy(ProcessName, processEntry.szExeFile);
				break;
			}
		} while (::Process32Next(hSnapAll, &processEntry));

		if (!hSnap)
		{
			CloseHandle(hSnapAll);
		}
	}
	return ProcessName;
}
#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
