#pragma once
class CProcessConnection
{
public:
	CProcessConnection();
	~CProcessConnection();

public:
	int GetInfo();
	int GetInfo2(DWORD dwOwningId = (-1L));

public:
	std::vector<std::string> m_tcp_list;
	std::vector<std::string> m_udp_list;
};

