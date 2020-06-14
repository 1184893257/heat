#include <string>
#include <vector>
#include <iostream>
#include "config.h"

using namespace std;
using json = nlohmann::json;

bool ocr_debug = false;

#ifdef __linux__
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
void initSG90();
void handleSignal(int);
bool captureAndHit(HeatResult& result);
void listdir(const char *path, vector<string>& children);
void del_dir(const char *npath);

int main()
{
	daemon(0, 0);
	initSG90();
	signal(SIGUSR1, handleSignal);
	init_config(true);

	syslog(LOG_INFO, "daemon start ok");
	bool hasError = false;
	for (;;)
	{
		// 出错了5分钟后尝试一次
		unsigned int sleepTime = (hasError ? 5 : 30) * 60;
		//unsigned int sleepTime = 60;
		hasError = false;
		while (sleepTime > 0)
		{
			// 来信号了会被打断，返回值是剩余没睡的时间
			sleepTime = sleep(sleepTime);
		}
		
		syslog(LOG_INFO, "heat wake up");

		if (config.taskDir.length() > 0)
		{
			vector<string> dirs;
			string tasksDir = path_to_root("tasks");
			listdir(tasksDir.c_str(), dirs);
			if (dirs.size() > 10)
			{
				for (int i = 0; i < dirs.size() - 10; ++i)
				{
					string task(tasksDir);
					task += "/";
					task += dirs[i];
					del_dir(task.c_str());
				}
			}
			
			// 有循环任务
			time_t now = time(nullptr);
			if (now > config.endTime)
			{
				// 任务已经超时, 清洗数据
				config.taskDir = "";
				config.results.clear();
			}
			else
			{
				HeatResult result;
				hasError = !captureAndHit(result);
				if (result.status.length() == 0)
				{
					result.status = hasError ? "Error" : "Normal";
				}
				config.results.push_back(result);
				
				syslog(LOG_INFO, "captureAndHit result=%s", result.status.c_str());

				if (!hasError)
				{
					string text = result.textAfterHit;
					int hour = text[0] - '0';
					int minute = atoi(text.c_str() + 1);
					int seconds = (hour * 60 + minute) * 60;
					if (now + seconds > config.endTime)
					{
						// 时间已经够了，任务可以停止了
						// 清数据前先保存这最后一次操作
						write_config();
						config.taskDir = "";
						config.results.clear();
					}
				}
			}
			write_config();
		}
	}
}
#else

#include <vector>
int main()
{
	vector<string> v;
	v.push_back("123");
	json j;
	j["a"] = v;
	for (auto& item : j["a"])
	{
		cout << item << endl;
	}
  //write_config();
	return 0;
}
#endif // __linux__


