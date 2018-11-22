#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <functional>

#include <muduo/base/Logging.h>
#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Types.h>
#include <muduo/net/EventLoop.h>

#include "../publlic/Config.h"
#include "Server/GetwayServer.h"

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
	//set just start once
	int lock_file = open("/tmp/GetwayServer.lock", O_CREAT | O_RDWR, 0666);
	int rc = flock(lock_file, LOCK_EX | LOCK_NB);
	if(rc)
	{
		if(EWOULDBLOCK == errno)
		{
			LOG_FATAL << "This program has started";
		}
	}
	else
	{
		char buffer[64];
		sprintf(buffer, "pid:%d\n", getpid());
		write(lock_file, buffer, strlen(buffer));
		close(lock_file); 
	}

	//check option
	bool isDaemon = false;
	int ch;
	while((ch = getopt(argc, argv, "dv")) != -1)
	{
		switch(ch)
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

	if(isDaemon)
		daemon(1, 0);//1 current dir is work dir;0 fd 0, 1, 2to /dev/null

	Config config("./Getway.cfg");

	//init the logger
	muduo::string strLogFileFullPath = config.Read<muduo::string>("LogDir");
	auto dir = opendir(strLogFileFullPath.c_str());
	if(dir == nullptr)
	{
		if(mkdir(strLogFileFullPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)//chmod 777
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