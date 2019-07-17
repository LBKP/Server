
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>


class GameServer
{
private:
	/* tcp connection */
	void onClientConnection(const muduo::net::TcpConnectionPtr& conn);
	void SendMessage(const muduo::net::TcpConnectionPtr& conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp receiveTime);public:
	GameServer(/* args */);
	~GameServer();
};

GameServer::GameServer(/* args */)
{
}

GameServer::~GameServer()
{
}
