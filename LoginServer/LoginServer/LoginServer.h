#pragma once
#include <muduo/base/noncopyable.h>

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>

#include "MessageRegister.h"
#include "../../publlic/Codec.h"
#include "../../publlic/MessageDispatcher.h"


/****************************************************************
* Author		£ºZhuPei.pur@outlook.com
* Create		£º2018/12/1 20:16
* Version		£º1.0.0.1
* * Description	£ºUser login and help user into Lobby server
* * History		£º
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
	void started();
private:
	
private:
	muduo::net::TcpClient connection_;
	muduo::net::EventLoop* loop_;
	MessageRegister dispatcher_;
	ProtobufCodec codec_;
};

