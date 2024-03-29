#pragma once

#include "MemInvaderInclude.h"

class TcpConnection
{
	std::string remote_ip, local_ip;
	short local_port, remote_port;

	MIB_TCPROW2 row;

public:
	TcpConnection(MIB_TCPROW2& row);

	inline std::string get_remote_ip() const { return remote_ip; }
	inline std::string get_local_ip() const { return local_ip; }
	inline int get_remote_port() const { return remote_port; }
	inline int get_local_port() const { return local_port; }

	void close();
};

