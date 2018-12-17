#include "LoginServer.h"

using namespace std::placeholders;

LoginServer::LoginServer(muduo::net::EventLoop* loop,
	const muduo::net::InetAddress& serverAddr,
	const muduo::string& nameArg)
	:connection_(loop, serverAddr, nameArg),
	loop_(loop),
	dispatcher_(std::bind(&LoginServer::onUnKnownMessage, this, _1, _2, _3, _4)),
	codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3, _4))
{
	connection_.setConnectionCallback(
		std::bind(&LoginServer::onConnected, this, _1));
	connection_.setMessageCallback(
		std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
}

LoginServer::~LoginServer()
{
}

void LoginServer::start()
{
	connection_.connect();
	LOG_INFO << "Login server started ";
}

void LoginServer::onConnected(const muduo::net::TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		Gateway::ServerRegister_SG msg;
		auto attr = msg.mutable_server();
		attr->set_type(Gateway::LOGIN);
		codec_.send(conn, 0, msg);
		LOG_INFO << "Connected Gateway server " << connection_.connection()->getTcpInfoString();
	}
	else
	{
		LOG_ERROR << "Connection is disable, and try to reconnected";
		connection_.connect();
	}
}

void LoginServer::onUnKnownMessage(const muduo::net::TcpConnectionPtr & conn, const int hash, const MessagePtr & message, muduo::Timestamp)
{
}
