#pragma once
typedef struct tagTcpTable {
	CHAR czProcessName[MAX_PATH];
	DWORD dwProcessId;
	CHAR szLocalPort[64];
	CHAR szRemotePort[64];
	CHAR szStatus[64];
}TCPTABLE, *PTCPTABLE;
typedef struct tagUdpTable {
	CHAR czProcessName[MAX_PATH];
	DWORD dwProcessId;
	CHAR szLocalPort[64];
}UDPTABLE, *PUDPTABLE;
class CProcessConnection
{
public:
	CProcessConnection();
	~CProcessConnection();

public:
	int GetInfo();
	int GetInfo2(DWORD dwOwningId = (-1L));

public:
	std::vector<TCPTABLE> m_tcp_list;
	std::vector<UDPTABLE> m_udp_list;
};

