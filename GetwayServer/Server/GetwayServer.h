#pragma once
#include <string>

#include <muduo/net/websocket/WebSocketServer.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Types.h>

class GetwayServer
{
public:
	GetwayServer(muduo::net::EventLoop* loop,
		const muduo::net::InetAddress websocketAddr,
		const muduo::net::InetAddress TcpAddr,
		muduo::net::ssl::sslAttrivutesPtr sslAttr);
	~GetwayServer();

private:
	//muduo::net::wss::WebSocketServer websocketServer_;
	//muduo::net::TcpServer tcpServer_;

};
