#pragma once
#include <muduo/base/noncopyable.h>

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>

#include "./Message/MessageRegister.h"
#include "./Message/Gateway.pb.h"
#include "../../publlic/Codec.h"
#include "../../publlic/MessageDispatcher.h"


/****************************************************************
* Author		：ZhuPei.pur@outlook.com
* Create		：2018/12/1 20:16
* Version		：1.0.0.1
* * Description	：User login and help user into Lobby server
* * History		：
******************************************************************
* Copyright ZhuPei  All rights reserved*
*****************************************************************/

class LoginServer :muduo::noncopyable
{
public:
	LoginServer(muduo::net::EventLoop* loop,
		const muduo::net::InetAddress& serverAddr,
		const muduo::string& nameArg);
	~LoginServer();
	void start();
private:
	//callbacks
	void onConnected(const muduo::net::TcpConnectionPtr& conn);
	void onUnKnownMessage(const muduo::net::TcpConnectionPtr& conn,
		const int hash,
		const MessagePtr&message,
		muduo::Timestamp);
private:
	muduo::net::TcpClient connection_;
	muduo::net::EventLoop* loop_;
	ProtobufDispatcher dispatcher_;
	ProtobufCodec codec_;
};

