#include "LoginServer.h"

LoginServer::LoginServer(muduo::net::EventLoop* loop,
	const muduo::net::InetAddress& serverAddr,
	const muduo::string& nameArg)
	:connection_(loop, serverAddr, nameArg),
	loop_(loop),
	codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
{
}

LoginServer::~LoginServer()
{
}

void LoginServer::started()
{
	connection_.connect();
}
