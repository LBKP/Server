#include<unistd.h>
#include<dirent.h>
#include <sys/stat.h>


#include<functional>

#include<muduo/base/Logging.h>
#include<muduo/base/AsyncLogging.h>
#include<muduo/base/Types.h>
#include<muduo/net/EventLoop.h>

#include"../publlic/Config.h"
#include"Server/GetwayServer.h"

using namespace std;
#define DEBUG
muduo::AsyncLogging* g_asyncLog = nullptr;
void asyncOutput(const char* msg, int len)
{
	g_asyncLog->append(msg, len);
#ifdef DEBUG
	fwrite(msg, 1, len, stdout);
#endif // DEBUG

}
int main(int argc, char** argv)
{
	bool isDaemon = false;
	int ch;
	while ((ch = getopt(argc, argv, "dv")) != -1)
	{
		switch (ch)
		{
		case 'd':
			isDaemon = true;
			LOG_INFO << "Server will run in backeground";
			break;
		case 'v':
			LOG_INFO << "This server version is 0.0.0";
			return 1;
			break;
		default:
			return 2;
			LOG_INFO << "Unknown option, server will stoped";
			break;
		}
	}

	if (isDaemon)
		daemon(1, 0);

	Config config("./Getway.cfg");

	//init the logger
	muduo::string strLogFileFullPath = config.Read<muduo::string>("LogDir");
	auto dir = opendir(strLogFileFullPath.c_str());
	if (dir == nullptr)
	{
		if (mkdir(strLogFileFullPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)//chmod 777
		{
			LOG_FATAL << "make dir error error no" << errno << "  " << strerror(errno);
		}
	}
	closedir(dir);
	strLogFileFullPath += config.Read<muduo::string>("LogName");
	int kRollSize = 500 * 1000 * 1000;
	muduo::AsyncLogging log(strLogFileFullPath, kRollSize);
	g_asyncLog = &log;
	log.start();
	muduo::Logger::setOutput(asyncOutput);

	//init ssl attrivutes
	muduo::net::ssl::sslAttrivutesPtr ssl(new muduo::net::ssl::SslServerAttributes);
	ssl->certificatePath = config.Read<string>("certificatePath");
	ssl->keyPath = config.Read<string>("keyPath");
	ssl->certificateType = config.Read<int>("certificateType");
	ssl->keyType = config.Read<int>("keyType");

	//start GetWay server
	muduo::net::InetAddress webSocketAddr(config.Read<uint16_t>("CliPort"));
	muduo::net::InetAddress TcpSocketAddr(config.Read<uint16_t>("SerPort"));

	muduo::net::EventLoop loop;
	GetwayServer server(&loop, webSocketAddr, TcpSocketAddr, ssl);
	server.start();
	loop.loop();
	return 0;
}