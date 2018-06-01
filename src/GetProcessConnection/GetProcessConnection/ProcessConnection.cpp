#include "stdafx.h"
#include "ProcessConnection.h"

CProcessConnection::CProcessConnection()
{
}


CProcessConnection::~CProcessConnection()
{
}

int CProcessConnection::GetInfo()
{
	// 定义扩展函数指针
	PFNAllocateAndGetTcpExTableFromStack pAllocateAndGetTcpExTableFromStack;
	PFNAllocateAndGetUdpExTableFromStack pAllocateAndGetUdpExTableFromStack;

	// 获取扩展函数的入口地址	
	HMODULE hModule = ::LoadLibrary(_T("iphlpapi.dll"));

	pAllocateAndGetTcpExTableFromStack =
		(PFNAllocateAndGetTcpExTableFromStack)::GetProcAddress(hModule,
			"AllocateAndGetTcpExTableFromStack");

	pAllocateAndGetUdpExTableFromStack =
		(PFNAllocateAndGetUdpExTableFromStack)::GetProcAddress(hModule,
			"AllocateAndGetUdpExTableFromStack");

	if (pAllocateAndGetTcpExTableFromStack == NULL || pAllocateAndGetUdpExTableFromStack == NULL)
	{
		printf(" Ex APIs are not present \n ");
		// 说明你应该调用普通的IP帮助API去获取TCP连接表和UDP监听表
		return 0;
	}

	// 调用扩展函数，获取TCP扩展连接表和UDP扩展监听表

	PMIB_TCPEXTABLE pTcpExTable;
	PMIB_UDPEXTABLE pUdpExTable;

	// pTcpExTable和pUdpExTable所指的缓冲区自动由扩展函数在进程堆中申请
	if (pAllocateAndGetTcpExTableFromStack(&pTcpExTable, TRUE, GetProcessHeap(), 2, 2) != 0)
	{
		printf(" Failed to snapshot TCP endpoints.\n");
		return -1;
	}
	if (pAllocateAndGetUdpExTableFromStack(&pUdpExTable, TRUE, GetProcessHeap(), 2, 2) != 0)
	{
		printf(" Failed to snapshot UDP endpoints.\n");
		return -1;
	}

	// 给系统内的所有进程拍一个快照
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf(" Failed to take process snapshot. Process names will not be shown.\n\n");
		return -1;
	}

	printf(" Active Connections \n\n");
	char	szLocalAddr[128];
	char	szRemoteAddr[128];
	_TCHAR	szProcessName[128];
	in_addr inadLocal, inadRemote;
	char    strState[128];
	DWORD   dwRemotePort = 0;
	UINT i = 0;

	// 打印TCP扩展连接表信息
	for (i = 0; i < pTcpExTable->dwNumEntries; ++i)
	{
		// 状态
		switch (pTcpExTable->table[i].dwState)
		{
		case MIB_TCP_STATE_CLOSED:
			strcpy(strState, "CLOSED");
			break;
		case MIB_TCP_STATE_TIME_WAIT:
			strcpy(strState, "TIME_WAIT");
			break;
		case MIB_TCP_STATE_LAST_ACK:
			strcpy(strState, "LAST_ACK");
			break;
		case MIB_TCP_STATE_CLOSING:
			strcpy(strState, "CLOSING");
			break;
		case MIB_TCP_STATE_CLOSE_WAIT:
			strcpy(strState, "CLOSE_WAIT");
			break;
		case MIB_TCP_STATE_FIN_WAIT1:
			strcpy(strState, "FIN_WAIT1");
			break;
		case MIB_TCP_STATE_ESTAB:
			strcpy(strState, "ESTAB");
			break;
		case MIB_TCP_STATE_SYN_RCVD:
			strcpy(strState, "SYN_RCVD");
			break;
		case MIB_TCP_STATE_SYN_SENT:
			strcpy(strState, "SYN_SENT");
			break;
		case MIB_TCP_STATE_LISTEN:
			strcpy(strState, "LISTEN");
			break;
		case MIB_TCP_STATE_DELETE_TCB:
			strcpy(strState, "DELETE");
			break;
		default:
			printf("Error: unknown state!\n");
			break;
		}
		// 本地IP地址
		inadLocal.s_addr = pTcpExTable->table[i].dwLocalAddr;

		// 远程端口
		if (strcmp(strState, "LISTEN") != 0)
		{
			dwRemotePort = pTcpExTable->table[i].dwRemotePort;
		}
		else
		{
			dwRemotePort = 0;
		}
		// 远程IP地址
		inadRemote.s_addr = pTcpExTable->table[i].dwRemoteAddr;


		sprintf(szLocalAddr, "%s:%u", inet_ntoa(inadLocal),
			ntohs((unsigned short)(0x0000FFFF & pTcpExTable->table[i].dwLocalPort)));
		sprintf(szRemoteAddr, "%s:%u", inet_ntoa(inadRemote),
			ntohs((unsigned short)(0x0000FFFF & dwRemotePort)));

		// 打印出此入口的信息
		printf("%-5s %s:%d\n      State:   %s\n", "[TCP]",
			ProcessPidToName(szProcessName, pTcpExTable->table[i].dwProcessId,hProcessSnap),
			pTcpExTable->table[i].dwProcessId,
			strState);

		printf("      Local:   %s\n      Remote:  %s\n",
			szLocalAddr, szRemoteAddr);
	}

	// 打印UDP监听表信息
	for (i = 0; i < pUdpExTable->dwNumEntries; ++i)
	{
		// 本地IP地址
		inadLocal.s_addr = pUdpExTable->table[i].dwLocalAddr;

		sprintf(szLocalAddr, "%s:%u", inet_ntoa(inadLocal),
			ntohs((unsigned short)(0x0000FFFF & pUdpExTable->table[i].dwLocalPort)));

		// 打印出此入口的信息
		printf("%-5s %s:%d\n", "[UDP]",
			ProcessPidToName(szProcessName, pUdpExTable->table[i].dwProcessId, hProcessSnap),
			pUdpExTable->table[i].dwProcessId);
		printf("      Local:   %s\n      Remote:  %s\n",
			szLocalAddr, "*.*.*.*:*");
	}


	::CloseHandle(hProcessSnap);
	::LocalFree(pTcpExTable);
	::LocalFree(pUdpExTable);
	::FreeLibrary(hModule);
	return 0;
}

int CProcessConnection::GetInfo2(DWORD dwOwningId/* = (-1L)*/)
{
	int nResult = (0);

	PFNGetExtendedTcpTable fnGetExtendedTcpTable = NULL;
	PFNGetExtendedUdpTable fnGetExtendedUdpTable = NULL;

	HMODULE hModule = ::LoadLibrary(_T("iphlpapi.dll"));
	if (hModule)
	{
		fnGetExtendedTcpTable = (PFNGetExtendedTcpTable)GetProcAddress(hModule, "GetExtendedTcpTable");
		fnGetExtendedUdpTable = (PFNGetExtendedUdpTable)GetProcAddress(hModule, "GetExtendedUdpTable");

		if (!fnGetExtendedTcpTable || !fnGetExtendedUdpTable)
		{
			return (-1);
		}
		PVOID pTcpTable = NULL;
		DWORD dwTcpTableSize = 0;
		PVOID pUdpTable = NULL;
		DWORD dwUdpTableSize = 0;
		BOOL bOrder = FALSE;
		ULONG ulAf = AF_INET;

		m_tcp_list.clear();
		m_udp_list.clear();

		if (fnGetExtendedTcpTable(pTcpTable, &dwTcpTableSize, bOrder, ulAf, TCP_TABLE_OWNER_PID_ALL, 0) == ERROR_INSUFFICIENT_BUFFER)
		{
			pTcpTable = malloc(dwTcpTableSize);
			if (fnGetExtendedTcpTable(pTcpTable, &dwTcpTableSize, bOrder, ulAf, TCP_TABLE_OWNER_PID_ALL, 0) != NO_ERROR)
			{
				return (-1);
			}
		}
		if (fnGetExtendedUdpTable(pTcpTable, &dwUdpTableSize, bOrder, ulAf, UDP_TABLE_OWNER_PID, 0) == ERROR_INSUFFICIENT_BUFFER)
		{
			pUdpTable = malloc(dwUdpTableSize);
			if (fnGetExtendedUdpTable(pUdpTable, &dwUdpTableSize, bOrder, ulAf, UDP_TABLE_OWNER_PID, 0) != NO_ERROR)
			{
				return (-1);
			}
		}
#if defined(DEBUG) || defined(_DEBUG)
		printf("===================================TCP=====================================\n");
#endif
		PMIB_TCPTABLE_OWNER_PID pMibTcpTable2 = (PMIB_TCPTABLE_OWNER_PID)pTcpTable;
		for (UINT uIdx = 0; uIdx < pMibTcpTable2->dwNumEntries; uIdx++)
		{
			char szStatus[MAXCHAR] = { 0 };
			char szLineData[MAX_PATH * 2] = { 0 };
			struct in_addr localaddr = { 0 };
			struct in_addr remoteaddr = { 0 };
			char szLocalAddr[MAXCHAR] = { 0 };
			char szRemoteAddr[MAXCHAR] = { 0 };

			_TCHAR tProcessName[MAX_PATH] = { 0 };
			if ((dwOwningId == (-1L)) || (pMibTcpTable2->table[uIdx].dwOwningPid == dwOwningId))
			{
				localaddr.s_addr = pMibTcpTable2->table[uIdx].dwLocalAddr;
				strcpy(szLocalAddr, inet_ntoa(localaddr));
				remoteaddr.s_addr = pMibTcpTable2->table[uIdx].dwRemoteAddr;
				strcpy(szRemoteAddr, inet_ntoa(remoteaddr));

#if defined(DEBUG) || defined(_DEBUG)
				printf("进程名称:%ws,进程ID:%ld,本地:%s:%ld,远程:%s:%ld,状态:",
					ProcessPidToName(tProcessName, pMibTcpTable2->table[uIdx].dwOwningPid), 
					pMibTcpTable2->table[uIdx].dwOwningPid,
					szLocalAddr, ntohs(pMibTcpTable2->table[uIdx].dwLocalPort),
					szRemoteAddr, ntohs(pMibTcpTable2->table[uIdx].dwRemotePort));
#endif
				sprintf(szLineData, "进程名称:%ws,进程ID:%ld,本地:%s:%ld,远程:%s:%ld,状态:",
					ProcessPidToName(tProcessName, pMibTcpTable2->table[uIdx].dwOwningPid),
					pMibTcpTable2->table[uIdx].dwOwningPid,
					szLocalAddr, ntohs(pMibTcpTable2->table[uIdx].dwLocalPort),
					szRemoteAddr, ntohs(pMibTcpTable2->table[uIdx].dwRemotePort));
				// 状态
				switch (pMibTcpTable2->table[uIdx].dwState)
				{
				case MIB_TCP_STATE_CLOSED:
#if defined(DEBUG) || defined(_DEBUG)
					printf("CLOSED");
#endif
					strcat(szStatus, "CLOSED");
					break;
				case MIB_TCP_STATE_TIME_WAIT:
#if defined(DEBUG) || defined(_DEBUG)
					printf("TIME_WAIT");
#endif
					strcat(szStatus, "TIME_WAIT");
					break;
				case MIB_TCP_STATE_LAST_ACK:
#if defined(DEBUG) || defined(_DEBUG)
					printf("LAST_ACK");
#endif
					strcat(szStatus, "LAST_ACK");
					break;
				case MIB_TCP_STATE_CLOSING:
#if defined(DEBUG) || defined(_DEBUG)
					printf("CLOSING");
#endif
					strcat(szStatus, "CLOSING");
					break;
				case MIB_TCP_STATE_CLOSE_WAIT:
#if defined(DEBUG) || defined(_DEBUG)
					printf("CLOSE_WAIT");
#endif
					strcat(szStatus, "CLOSE_WAIT");
					break;
				case MIB_TCP_STATE_FIN_WAIT1:
#if defined(DEBUG) || defined(_DEBUG)
					printf("FIN_WAIT1");
#endif
					strcat(szStatus, "CLOSED");
					break;
				case MIB_TCP_STATE_FIN_WAIT2:
#if defined(DEBUG) || defined(_DEBUG)
					printf("FIN_WAIT2");
#endif
					strcat(szStatus, "FIN_WAIT2");
					break;
				case MIB_TCP_STATE_ESTAB:
#if defined(DEBUG) || defined(_DEBUG)
					printf("ESTAB");
#endif
					strcat(szStatus, "ESTAB");
					break;
				case MIB_TCP_STATE_SYN_RCVD:
#if defined(DEBUG) || defined(_DEBUG)
					printf("SYN_RCVD");
#endif
					strcat(szStatus, "SYN_RCVD");
					break;
				case MIB_TCP_STATE_SYN_SENT:
#if defined(DEBUG) || defined(_DEBUG)
					printf("SYN_SENT");
#endif
					strcat(szStatus, "SYN_SENT");
					break;
				case MIB_TCP_STATE_LISTEN:
#if defined(DEBUG) || defined(_DEBUG)
					printf("LISTEN");
#endif
					strcat(szStatus, "LISTEN");
					break;
				case MIB_TCP_STATE_DELETE_TCB:
#if defined(DEBUG) || defined(_DEBUG)
					printf("DELETE");
#endif
					strcat(szStatus, "DELETE");
					break;
				default:
#if defined(DEBUG) || defined(_DEBUG)
					printf("Error: Unknown State");
#endif
					strcat(szStatus, "Error: Unknown State");
					break;
				}
#if defined(DEBUG) || defined(_DEBUG)
				printf("\n==============================================\n");
#endif
				strcat(szLineData, szStatus);
				m_tcp_list.push_back(szLineData);
			}
		}

#if defined(DEBUG) || defined(_DEBUG)
		printf("===================================UDP=====================================\n");
#endif
		PMIB_UDPTABLE_OWNER_PID pMibUdpTable2 = (PMIB_UDPTABLE_OWNER_PID)pUdpTable;
		for (UINT uIdx = 0; uIdx < pMibUdpTable2->dwNumEntries; uIdx++)
		{
			char szLineData[MAX_PATH * 2] = { 0 };
			struct in_addr localaddr = { 0 };
			char szLocalAddr[MAXCHAR] = { 0 };
			_TCHAR tProcessName[MAX_PATH] = { 0 };
			if ((dwOwningId == (-1L)) || (pMibUdpTable2->table[uIdx].dwOwningPid == dwOwningId))
			{
				localaddr.s_addr = pMibUdpTable2->table[uIdx].dwLocalAddr;
				strcpy(szLocalAddr, inet_ntoa(localaddr));
#if defined(DEBUG) || defined(_DEBUG)
				printf("进程名称:%ws,进程ID:%ld,本地:%s:%ld",
					ProcessPidToName(tProcessName, pMibUdpTable2->table[uIdx].dwOwningPid),
					pMibUdpTable2->table[uIdx].dwOwningPid,
					szLocalAddr, ntohs(pMibUdpTable2->table[uIdx].dwLocalPort));
#endif
				sprintf(szLineData, "进程名称:%ws,进程ID:%ld,本地:%s:%ld",
					ProcessPidToName(tProcessName, pMibUdpTable2->table[uIdx].dwOwningPid),
					pMibUdpTable2->table[uIdx].dwOwningPid,
					szLocalAddr, ntohs(pMibUdpTable2->table[uIdx].dwLocalPort));

#if defined(DEBUG) || defined(_DEBUG)
				printf("\n==============================================\n");
#endif
				m_udp_list.push_back(szLineData);
			}
		}

		FreeLibrary(hModule);
	}
	return nResult;
}