#include "GetwayServer.h"

GetwayServer::GetwayServer(muduo::net::EventLoop* loop,
	const muduo::net::InetAddress websocketAddr,
	const muduo::net::InetAddress TcpAddr,
	muduo::net::ssl::sslAttrivutesPtr sslAttr)
	//://websocketServer_(loop, websocketAddr, "WebsocketServer", muduo::net::TcpServer::kReusePort, sslAttr),
	//tcpServer_(loop, TcpAddr, "TcpServer", muduo::net::TcpServer::kReusePort)
{
}

GetwayServer::~GetwayServer()
{
}
