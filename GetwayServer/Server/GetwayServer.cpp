#include "GetwayServer.h"

#include <any>

GetwayServer::GetwayServer(muduo::net::EventLoop* loop,
	const muduo::net::InetAddress websocketAddr,
	const muduo::net::InetAddress TcpAddr,
	muduo::net::ssl::sslAttrivutesPtr sslAttr)
	:websocketServer_(loop, websocketAddr, "WebsocketServer", muduo::net::TcpServer::kReusePort, sslAttr),
	tcpServer_(loop, TcpAddr, "TcpServer", muduo::net::TcpServer::kReusePort),
	loop_(loop),
	maxId_(100)
{
	websocketServer_.setConnectionCallback(std::bind(&GetwayServer::onClientConnection, this, std::placeholders::_1));
	websocketServer_.setMessageCallback(std::bind(&GetwayServer::onClientMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	tcpServer_.setConnectionCallback(std::bind(&GetwayServer::onServerConnection, this, std::placeholders::_1));
	tcpServer_.setMessageCallback(std::bind(&GetwayServer::onServerMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

}

GetwayServer::~GetwayServer()
{
}

void GetwayServer::start()
{
	LOG_INFO << "GetwayServer started ";
	websocketServer_.setThreadNum(2);
	websocketServer_.start();
	tcpServer_.start();
}


void GetwayServer::onClientConnection(const muduo::net::TcpConnectionPtr & conn)
{
	int hash;
	if (conn->connected())
	{
		if (priorIds_.empty())
		{
			//don't need consider maxId oversteped the max int
			hash = maxId_;
			++maxId_;
		}
		else
		{
			hash = priorIds_.front();
			priorIds_.pop();
		}
		{
			muduo::MutexLockGuard lock(mutex_);
			Conections_[hash] = conn;
		}
		conn->setContext(hash);
		LOG_INFO << "User " << conn->getTcpInfoString() << " connected and this hash is " << hash;
	}
	else
	{
		hash = std::any_cast<int>(conn->getContext());
		{
			muduo::MutexLockGuard lock(mutex_);
			Conections_.erase(Conections_.find(hash));
		}
		priorIds_.push(hash);
		conn->send(0);//Fix this, need to range a login server to client
		LOG_INFO << "User " << conn->getTcpInfoString() << " Disconnected and this hash is " << hash;
	}
}

void GetwayServer::onClientMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer * buf, muduo::Timestamp receiveTime)
{
}

void GetwayServer::onServerConnection(const muduo::net::TcpConnectionPtr & conn)
{
	if (conn->connected())
	{
		LOG_INFO << "Server " << conn->getTcpInfoString() << " connected and need register";
	}
	else
	{
		LOG_INFO << "Server " << conn->getTcpInfoString() << " Disconnected and will remove this";
		broadcastServerDisconnected(std::any_cast<int>(conn->getContext()));
		muduo::MutexLockGuard lock(mutex_);
		Conections_.erase(Conections_.find(std::any_cast<int>(conn->getContext())));
	}

}

void GetwayServer::onServerMessage(const muduo::net::TcpConnectionPtr & conn, muduo::net::Buffer * buf, muduo::Timestamp receiveTime)
{
	try
	{
		int hash = std::any_cast<int>(conn->getContext());
		parseServiceMessage(hash);
	}
	catch (const std::bad_any_cast&)
	{//server need register
		int8_t server = buf->peekInt8();
		if (server >= serverType::ErrorServer)
		{
			conn->forceClose();
			LOG_ERROR << "This server can not recognition , type is " << server;
			return;
		}
		buf->retrieve(1);
		for (int hash = server * 10; hash < server * 100; hash++)
		{
			if (Conections_.find(hash) == Conections_.end())
			{//find a unused hash
				{
					muduo::MutexLockGuard lock(mutex_);
					Conections_[hash] = conn;
				}
				conn->setContext(hash);
				broadcastNewServerConnected(hash);
				if (server == serverType::GameLobby)
				{
					sendAllConnectedServer(hash);
				}
				LOG_INFO << "Server " << conn->getTcpInfoString() << " registed and the server type is" << hash / 10;
				break;
			}
		}
		if (buf->readableBytes())
		{
			onServerMessage(conn, buf, receiveTime);
		}
	}

}

void GetwayServer::broadcastNewServerConnected(int hash)
{
}

void GetwayServer::broadcastServerDisconnected(int hash)
{
}

void GetwayServer::sendAllConnectedServer(int hash)
{
}

void GetwayServer::parseClientMessage(int hash)
{
}

void GetwayServer::parseServiceMessage(int hash)
{
}


