#pragma once
/****************************************************************
* Author		��ZhuPei.pur@outlook.com
* Create		��2018/12/1
* Version		��1.0.0.1
* * Description	��retransmission all message send by client amd server
* * History		��
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

#include "Getway.pb.h"
#include "../../publlic/Codec.h"
#include "../../publlic/MessageDispatcher.h"

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
	void onClientMessage(const muduo::net::TcpConnectionPtr&, 
							muduo::net::Buffer* buf, 
							muduo::Timestamp receiveTime);
	void onServerConnection(const muduo::net::TcpConnectionPtr& conn);
	void onServerMessage(const muduo::net::TcpConnectionPtr& conn,
						muduo::net::Buffer* buf,
						muduo::Timestamp receiveTime);
	//notify service's change
	void broadcastNewServerConnected(int hash);
	void broadcastServerDisconnected(int hash);
	void sendAllConnectedServer(int hash);

	//parse message
	void parseClientMessage(int hash);
	void parseServiceMessage(int hash);

	//send to getway's message
	void onServerRegister(int hash, 
						const std::shared_ptr<Getway::ServerRegister>& message, 
						muduo::Timestamp);
	void onUnKnownMessage(int hash, const MessagePtr&message, muduo::Timestamp);
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

	//message handle
	ProtobufDispatcher dispatcher_;
	ProtobufCodec codec_;
};
