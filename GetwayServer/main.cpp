#include<unistd.h>
#include<string>

#include<muduo\base\Logging.h>
#include<muduo\base\AsyncLogging.h>

#include"../publlic/Config.h"

using namespace std;

int main(int argc, char** argv)
{
	bool isDaemon = false;
	char ch;
	while (ch = getopt(argc, argv, "dv") != -1)
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
	Config config("./Getway.config");
	int kRollSize = 500 * 1000 * 1000;
	string strLogFileFullPath = config.Read<string>("LogDir");
	muduo::AsyncLogging log(strLogFileFullPath, kRollSize);
	log.start();

	return 0;
}