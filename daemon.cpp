#include <string>
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

int main()
{
	daemon(0, 0);
	initSG90();
	signal(SIGUSR1, handleSignal);
	init_config(true);

	bool hasError = false;
	for (;;)
	{
		// 出错了5分钟后尝试一次
		//unsigned int sleepTime = (hasError ? 5 : 30) * 60;
		unsigned int sleepTime = 60;
		hasError = false;
		while (sleepTime > 0)
		{
			// 来信号了会被打断，返回值是剩余没睡的时间
			sleepTime = sleep(sleepTime);
		}

		if (config.taskDir.length() > 0)
		{
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
  GlobalConfig a = {
          1,
          2,
          3,
          "20200119/",
          {
                  {
                    "111",
                    "222",
                    "3",
                    "4"
                  }
          }
  };
  // config = a;
  read_struct(path_to_root("config.json"), config);
  // cout << config.daemonPid << endl;

  string sa = "123";
  string sb = sa + "456";
  cout << sa << endl << sb << endl;

  //write_config();
	return 0;
}
#endif // __linux__


