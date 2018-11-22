#pragma once
#include <string>
#include <map>

#include <muduo/net/websocket/WebSocketServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Types.h>
#include <muduo/base/Mutex.h>

class GetwayServer
{
public:
	GetwayServer(muduo::net::EventLoop* loop,
		const muduo::net::InetAddress websocketAddr,
		const muduo::net::InetAddress TcpAddr,
		muduo::net::ssl::sslAttrivutesPtr sslAttr);
	~GetwayServer();

	void start();

private:
	muduo::net::wss::WebSocketServer websocketServer_;
	muduo::net::TcpServer tcpServer_;
	std::map<int, muduo::net::TcpConnectionPtr> Conections_;
	muduo::MutexLock mutex_;
	int maxId;
};
