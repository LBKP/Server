#include "GetwayServer.h"

GetwayServer::GetwayServer(muduo::net::EventLoop* loop,
	const muduo::net::InetAddress websocketAddr,
	const muduo::net::InetAddress TcpAddr,
	muduo::net::ssl::sslAttrivutesPtr sslAttr)
	:websocketServer_(loop, websocketAddr, "WebsocketServer", muduo::net::TcpServer::kReusePort, sslAttr),
	tcpServer_(loop, TcpAddr, "TcpServer", muduo::net::TcpServer::kReusePort)
{
	websocketServer_.setConnectionCallback(std::bind(onClientConnection, this, std::placeholders::_1));
	websocketServer_.setMessageCallback(std::bind(onClientMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	tcpServer_.setConnectionCallback(std::bind(onServerConnection, this, std::placeholders::_1));
	tcpServer_.setMessageCallback(std::bind(onServerMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

}

GetwayServer::~GetwayServer()
{
}

void GetwayServer::start()
{
	LOG_INFO << "GetwayServer started by " << muduo::Timestamp::now();
	websocketServer_.setThreadNum(2);
	websocketServer_.start();
	tcpServer_.start();
}

void GetwayServer::onClientConnection(const muduo::net::TcpConnectionPtr & conn)
{
}

void GetwayServer::onClientMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer * buf, muduo::Timestamp receiveTime)
{
}

void GetwayServer::onServerConnection(const muduo::net::TcpConnectionPtr & conn)
{
}

void GetwayServer::onServerMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer * buf, muduo::Timestamp receiveTime)
{
}
