#pragma once
/****************************************************************
* Author		£ºZhuPei.pur@outlook.com
* Create		£º2018/12/1
* Version		£º1.0.0.1
* * Description	£ºretransmission all message send by client amd server
* * History		£º
******************************************************************
* Copyright ZhuPei 2018 All rights reserved*
*****************************************************************/

#include <string>
#include <unordered_map>
#include <queue>

#include <muduo/net/websocket/WebSocketServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Types.h>
#include <muduo/base/Mutex.h>

enum serverType :char
{
	LoginServer,
	GameLobby,
	ErrorServer = 10
};

enum ErrorCode
{
	kNoError = 0,
	kInvalidLength,
	kInvalidDestination,
	kCheckSumError,
	kInvalidNameLen,
	kParseError,
};

const static int kHeaderLen = 9;
const static int kMinClientMessageLen = kHeaderLen + 4;//header + checksum
const static int kMinServerMessageLen = kHeaderLen;//header + checksum
const static int kMaxMessageLen = 64 * 1024 * 1024;

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
	//Callback's
	void onClientConnection(const muduo::net::TcpConnectionPtr& conn);
	void onClientMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
	void onServerConnection(const muduo::net::TcpConnectionPtr& conn);
	void onServerMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
	//notify service's change
	void broadcastNewServerConnected(int hash);
	void broadcastServerDisconnected(int hash);
	void sendAllConnectedServer(int hash);

	//parse message
	void parseClientMessage(int hash);
	void parseServiceMessage(int hash);
private:
	//recive all connection
	muduo::net::wss::WebSocketServer websocketServer_;
	muduo::net::TcpServer tcpServer_;
	muduo::net::EventLoop* loop_;

	//manage all connection
	muduo::MutexLock mutex_;
	std::unordered_map<int, muduo::net::TcpConnectionPtr> Conections_;
	int maxId_;
	std::queue<int> priorIds_;

};
