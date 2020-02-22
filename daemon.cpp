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

	bool hasError = false;
	for (;;)
	{
		// ������5���Ӻ���һ��
		unsigned int sleepTime = (hasError ? 5 : 30) * 60;
		//unsigned int sleepTime = 60;
		hasError = false;
		while (sleepTime > 0)
		{
			// ���ź��˻ᱻ��ϣ�����ֵ��ʣ��û˯��ʱ��
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
			
			// ��ѭ������
			time_t now = time(nullptr);
			if (now > config.endTime)
			{
				// �����Ѿ���ʱ, ��ϴ����
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
						// ʱ���Ѿ����ˣ��������ֹͣ��
						// ������ǰ�ȱ��������һ�β���
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


