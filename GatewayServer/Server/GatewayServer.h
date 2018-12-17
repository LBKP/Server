#pragma once

#include <string>
#include <unordered_map>
#include <queue>

#include <muduo/net/websocket/WebSocketServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Types.h>
#include <muduo/base/Mutex.h>

#include "./Gateway.pb.h"
#include "../../publlic/Codec.h"
#include "../../publlic/MessageDispatcher.h"

/****************************************************************
* Author		：ZhuPei.pur@outlook.com
* Create		：
* Version		：1.0.0.1
* * Description	：
* * History		：
******************************************************************
* Copyright ZhuPei  All rights reserved*
*****************************************************************/

enum ErrorCode
{
	kNoError = 0,
	kInvalidLength,
	kInvalidDestination,
	kCheckSumError,
	kInvalidNameLen,
	kParseError,
};

const static int kHeaderLen = 4;
const static int kMinClientMessageLen = 3 * kHeaderLen + 1;//header + destination + namelen + checksum
const static int kMinServerMessageLen = 2 * kHeaderLen + 1;//header + destination + namelen
const static int kMaxMessageLen = 64 * 1024 * 1024;
const static int kMinClientHash = 100;

class GatewayServer
{
public:
	GatewayServer(muduo::net::EventLoop* loop,
		const muduo::net::InetAddress websocketAddr,
		const muduo::net::InetAddress TcpAddr,
		muduo::net::ssl::sslAttrivutesPtr sslAttr);
	~GatewayServer();

	void start();
private:
	//Callback's
	void onClientConnection(const muduo::net::TcpConnectionPtr& conn);
	void onClientMessage(const muduo::net::TcpConnectionPtr& conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp receiveTime);
	void onServerConnection(const muduo::net::TcpConnectionPtr& conn);
	void onServerMessage(const muduo::net::TcpConnectionPtr& conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp receiveTime);
	//notify service's change
	void broadcastNewServerConnected(int hash);
	void sendAllConnectedServer(int hash);


	//send to getway's message
	void onServerRegister(const muduo::net::TcpConnectionPtr& conn,
		int hash,
		const std::shared_ptr<Gateway::ServerRegister_SG>& message,
		muduo::Timestamp);
	void onUnKnownMessage(const muduo::net::TcpConnectionPtr& conn,
		int hash,
		const MessagePtr&message,
		muduo::Timestamp);

	//server status
	void printServerStatus();
private:
	//recive all connection
	muduo::net::wss::WebSocketServer websocketServer_;
	muduo::net::TcpServer tcpServer_;
	muduo::net::EventLoop* loop_;

	//manage all connection
	muduo::MutexLock mutex_;
	std::unordered_map<int32_t, muduo::net::TcpConnectionPtr> Connections_;
	int32_t maxHash_;
	std::queue<int32_t> priorIds_;

	//message handle
	ProtobufDispatcher dispatcher_;
	ProtobufCodec codec_;
};