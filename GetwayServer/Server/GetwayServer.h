#pragma once
#include <string>
#include <map>
#include <queue>

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
	void onClientConnection(const muduo::net::TcpConnectionPtr& conn);
	void onClientMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
	void onServerConnection(const muduo::net::TcpConnectionPtr& conn);
	void onServerMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer* buf, muduo::Timestamp receiveTime);

private:
	muduo::net::wss::WebSocketServer websocketServer_;
	muduo::net::TcpServer tcpServer_;
	std::map<int, muduo::net::TcpConnectionPtr> Conections_;
	muduo::MutexLock mutex_;
	int maxId_; 
	std::queue<int> priorIds_;
};
