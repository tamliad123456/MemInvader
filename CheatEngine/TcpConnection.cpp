#include "TcpConnection.h"

TcpConnection::TcpConnection(MIB_TCPROW2& row) : row(row)
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

bool TcpConnection::close_connection()
{
	MIB_TCPROW row;
	row.dwLocalAddr = this->row.dwLocalAddr;
	row.dwLocalPort = this->row.dwLocalPort & 0xffff;
	row.dwRemoteAddr = this->row.dwRemoteAddr;
	row.dwRemotePort = this->row.dwRemotePort & 0xffff;
	row.dwState = MIB_TCP_STATE_DELETE_TCB;

	DWORD result;

	return SetTcpEntry(&row) == 0;
}
