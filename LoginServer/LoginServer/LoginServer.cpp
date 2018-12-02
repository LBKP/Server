#include "LoginServer.h"

LoginServer::LoginServer(muduo::net::EventLoop* loop,
	const muduo::net::InetAddress& serverAddr,
	const muduo::string& nameArg)
	:connection_(loop, serverAddr, nameArg)
{
}

LoginServer::~LoginServer()
{
}
