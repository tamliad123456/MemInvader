#include "TcpConnection.h"

TcpConnection::TcpConnection(MIB_TCPROW2& row)
{
	struct in_addr IpAddr;

	local_port = ntohs((u_short)row.dwLocalPort);

	//local ip
	IpAddr.S_un.S_addr = (u_long)row.dwLocalAddr;
	local_ip = inet_ntoa(IpAddr);

	//remote ip
	IpAddr.S_un.S_addr = (u_long)row.dwRemoteAddr;
	remote_ip = inet_ntoa(IpAddr);

	remote_port = ntohs((u_short)row.dwRemotePort);
}