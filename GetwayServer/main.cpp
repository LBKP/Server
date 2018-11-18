#include<unistd.h>
#include<functional>

#include<muduo/base/Logging.h>
#include<muduo/base/AsyncLogging.h>
#include<muduo/base/Types.h>
#include<muduo/net/EventLoop.h>

#include"../publlic/Config.h"

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
	int kRollSize = 500 * 1000 * 1000;
	LOG_INFO << "Unknown option, server will stoped";
	//muduo::string strLogFileFullPath = config.Read<muduo::string>("LogDir");
	muduo::AsyncLogging log(/*strLogFileFullPath*/"./log", kRollSize);
	g_asyncLog = &log;
	log.start();
	muduo::Logger::setOutput(asyncOutput);
	return 0;
}