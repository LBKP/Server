#include "./GatewayServer.h"

#include <any>
#include <zlib.h>

using namespace std::placeholders;


int32_t readAInt32(const char* buf)
{
	int32_t be32 = 0;
	::memcpy(&be32, buf, sizeof(be32));
	return muduo::net::sockets::networkToHost32(be32);
}

GatewayServer::GatewayServer(muduo::net::EventLoop *loop,
	const muduo::net::InetAddress websocketAddr,
	const muduo::net::InetAddress TcpAddr,
	muduo::net::ssl::sslAttrivutesPtr sslAttr)
	: websocketServer_(loop, websocketAddr,
		"WebsocketServer",
		muduo::net::TcpServer::kReusePort,
		sslAttr),
	tcpServer_(loop,
		TcpAddr, "TcpServer",
		muduo::net::TcpServer::kReusePort),
	loop_(loop),
	maxHash_(kMinClientHash),
	dispatcher_(std::bind(&GatewayServer::onUnKnownMessage, this, _1, _2, _3, _4)),
	codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3, _4))
{
	websocketServer_->setOpcode(Opcode::BINARY_FRAME);
	websocketServer_.setConnectionCallback(
		std::bind(&GatewayServer::onClientConnection, this, _1));
	websocketServer_.setMessageCallback(
		std::bind(&GatewayServer::onClientMessage, this, _1, _2, _3));
	tcpServer_.setConnectionCallback(
		std::bind(&GatewayServer::onServerConnection, this, _1));
	tcpServer_.setMessageCallback(
		std::bind(&GatewayServer::onServerMessage, this, _1, _2, _3));

	dispatcher_.registerMessageCallback<Gateway::ServerRegister_SG>(
		std::bind(&GatewayServer::onServerRegister, this, _1, _2, _3, _4));

	loop_->runEvery(30,
		std::bind(&GatewayServer::printServerStatus, this));
}

GatewayServer::~GatewayServer()
{
}

void GatewayServer::start()
{
	LOG_INFO << "GatewayServer started ";
	websocketServer_.setThreadNum(2);
	websocketServer_.start();
	tcpServer_.start();
}

void GatewayServer::onClientConnection(const muduo::net::TcpConnectionPtr &conn)
{
	int hash;
	if (conn->connected())
	{
		if (priorIds_.empty())
		{
			//don't need consider maxId oversteped the max int
			hash = maxHash_;
			++maxHash_;
		}
		else
		{
			hash = priorIds_.front();
			priorIds_.pop();
		}
		{
			muduo::MutexLockGuard lock(mutex_);
			Connections_[hash] = conn;
		}
		conn->setContext(hash);
		if (Connections_.find(1) != Connections_.end())
		{
			Gateway::ClientConnected_GS msg;
			msg.set_hash(hash);
			msg.set_connected(true);
			codec_.send(Connections_[1], 0, msg);
			LOG_INFO << "User " << conn->getTcpInfoString() << " connected and this hash is " << hash;
		}
		else
		{
			conn->forceClose();
			LOG_ERROR << "can not found a login server";
		}
	}
	else
	{
		hash = std::any_cast<int>(conn->getContext());
		{
			muduo::MutexLockGuard lock(mutex_);
			Connections_.erase(Connections_.find(hash));
		}
		priorIds_.push(hash);

		Gateway::ClientConnected_GS msg;
		msg.set_hash(hash);
		msg.set_connected(false);
		if (Connections_.find(1) != Connections_.end())
			codec_.send(Connections_[1], 0, msg);
		LOG_INFO << "User " << conn->getTcpInfoString() << " Disconnected and this hash is " << hash;
	}
}

void GatewayServer::onClientMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer *buf, muduo::Timestamp receiveTime)
{
	if (buf->readableBytes() < kMinClientMessageLen)
	{
		return;
	}
	int32_t len = buf->peekInt32();
	if (buf->readableBytes() < static_cast<uint32_t>(len + kHeaderLen))
	{
		return;
	}
	muduo::net::Buffer sendBuf;
	if (buf->readableBytes() == static_cast<uint32_t>(len + kHeaderLen))
	{
		sendBuf.swap(*buf);
	}
	else
	{
		sendBuf.append(buf->peek(), len + kHeaderLen);
		buf->retrieve(len + kHeaderLen);
	}
	const char* buffer = sendBuf.peek();
	int32_t hash = readAInt32(buffer + kHeaderLen);

	int checkSum = adler32(1, reinterpret_cast<const Bytef*>(buffer + kHeaderLen), len - kHeaderLen);
	if (checkSum == readAInt32(buffer + len))
	{

		int32_t newhash = muduo::net::sockets::hostToNetwork32(std::any_cast<int>(conn->getContext()));
		memcpy(const_cast<char*>(buffer + kHeaderLen), &newhash, sizeof(int32_t));
		//don't need checksum 
		int32_t newLen = len - kHeaderLen;
		memcpy(const_cast<char*>(buffer), &newLen, sizeof(int32_t));
		sendBuf.unwrite(sizeof(int32_t));
		muduo::net::TcpConnectionPtr target;
		{
			muduo::MutexLockGuard lock(mutex_);
			if (Connections_.find(hash) != Connections_.end())
				target = Connections_[hash];
		}
		if (target)
		{
			target->send(&sendBuf);
		}
		else
		{
			LOG_ERROR << hash << " This connection is deleated";
		}
	}
	else
	{
		LOG_ERROR << "check sum error, this hash is " << hash;
	}

	if (buf->readableBytes())
	{
		onServerMessage(conn, buf, receiveTime);
	}
}

void GatewayServer::onServerConnection(const muduo::net::TcpConnectionPtr &conn)
{
	if (conn->connected())
	{
		LOG_INFO << "Server " << conn->getTcpInfoString() << " connected and need register";
	}
	else
	{
		LOG_INFO << "Server " << conn->getTcpInfoString() << " Disconnected and will remove this";
		try
		{
			int hash = std::any_cast<int>(conn->getContext());
			broadcastNewServerConnected(hash);
			muduo::MutexLockGuard lock(mutex_);
			Connections_.erase(Connections_.find(std::any_cast<int>(conn->getContext())));
		}
		catch (const std::bad_any_cast&)
		{
			LOG_INFO << "Server connection has't hash";
		}

	}
}

void GatewayServer::onServerMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp receiveTime)
{
	if (buf->readableBytes() < kMinServerMessageLen)
	{
		return;
	}
	int32_t len = buf->peekInt32();
	if (buf->readableBytes() < static_cast<uint32_t>(len))
	{
		return;
	}

	int hash = readAInt32(buf->peek() + kHeaderLen);
	if (hash == 0)
	{
		codec_.onMessage(conn, buf, receiveTime);
	}
	else
	{
		muduo::net::Buffer sendBuf;
		if (buf->readableBytes() == static_cast<uint32_t>(len + kHeaderLen))
		{
			sendBuf.swap(*buf);
		}
		else
		{
			sendBuf.append(buf->peek(), len + kHeaderLen);
			buf->retrieve(len + kHeaderLen);
		}
		int32_t checkSum = adler32(1, reinterpret_cast<const Bytef*>(buf->peek() + kHeaderLen), len);
		sendBuf.appendInt32(checkSum);
		//checksum 4 bytes
		len += 4;
		memcpy(const_cast<char*>(sendBuf.peek()), &len, sizeof(len));

		//change hash
		try
		{
			int32_t newHahs = std::any_cast<int>(conn->getContext());
			memcpy(const_cast<char*>(sendBuf.peek() + kHeaderLen), &newHahs, sizeof(newHahs));
		}
		catch (std::bad_any_cast&)
		{
			LOG_ERROR << "This server haven't register" << conn->getTcpInfoString();
			conn->shutdown();
		}

		//send
		muduo::net::TcpConnectionPtr target;
		{
			muduo::MutexLockGuard lock(mutex_);
			if (Connections_.find(hash) != Connections_.end())
				target = Connections_[hash];
		}
		if (target)
		{
			target->send(&sendBuf);
		}
		else
		{
			LOG_ERROR << hash << " This client connection is deleated";
		}
	}

	if (buf->readableBytes())
	{
		onServerMessage(conn, buf, receiveTime);
	}
}

void GatewayServer::broadcastNewServerConnected(int hash)
{
	Gateway::ServerConnected_GS msg;
	auto server = msg.mutable_server();
	server->set_hash(hash);
	server->set_type(Gateway::ServerType(hash / 10));
	if (Connections_.find(hash) != Connections_.end())
		msg.set_connected(true);
	else
		msg.set_connected(false);
	for (auto conn : Connections_)
	{
		if (conn.first >= 100)
			break;
		codec_.send(conn.second, 0, msg);
	}
}

void GatewayServer::sendAllConnectedServer(int hash)
{
	Gateway::AllConnectedServer_GS msg;
	for (auto conn : Connections_)
	{
		if (conn.first >= 100)
			break;
		auto server = msg.add_server();
		server->set_hash(conn.first);
		server->set_type(Gateway::ServerType(conn.first / 10));
	}
	codec_.send(Connections_[hash], 0, msg);
}


void GatewayServer::onServerRegister(const muduo::net::TcpConnectionPtr& conn, int hash, const std::shared_ptr<Gateway::ServerRegister_SG>& message, muduo::Timestamp)
{
	int serverType = message->server().type();
	for (int hash = serverType * 10 + 1; hash < serverType * 10 + 10; hash++)
	{
		//find a unused hash
		if (Connections_.find(hash) == Connections_.end())
		{
			broadcastNewServerConnected(hash);

			{
				muduo::MutexLockGuard lock(mutex_);
				Connections_[hash] = conn;
			}
			conn->setContext(hash);
			//if (serverType == Gateway::LOBBY)
			//{
			sendAllConnectedServer(hash);
			//}
			LOG_INFO << "Server " << conn->getTcpInfoString() << " registed and the server type is" << hash / 10;
			break;
		}
	}
}

void GatewayServer::onUnKnownMessage(const muduo::net::TcpConnectionPtr& conn, int hash, const MessagePtr& message, muduo::Timestamp)
{
	LOG_ERROR << "received a unknown message from " << conn->getTcpInfoString() << " hash is " << hash;
}

void GatewayServer::printServerStatus()
{
	//haven't lock mutex because don't need Exact value
	LOG_INFO << "\n/************************************************************************/ \n"
		"				USER_COUNT:" << maxHash_ - 100 - priorIds_.size() << "\n"
		"				MAX_HASH/PRIOR_SIZE:" << maxHash_ << "/" << priorIds_.size() << "\n"
		"				SERVER_COUNT:" << Connections_.size() + 100 + priorIds_.size() - maxHash_
		<< "\n/************************************************************************/";
}
