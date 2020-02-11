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

void TcpConnection::close()
{

	MIB_TCPROW _row;
	_row.dwLocalAddr = row.dwLocalAddr;
	_row.dwLocalPort = row.dwLocalPort & 0xffff;
	_row.dwRemoteAddr = row.dwRemoteAddr;
	_row.dwRemotePort = row.dwRemotePort & 0xffff;
	_row.dwState = MIB_TCP_STATE_DELETE_TCB;

	SetTcpEntry(&_row);
}
