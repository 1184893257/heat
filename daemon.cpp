#include <string>
#include <iostream>
#include "ipc.h"

#ifdef __linux__
#include <unistd.h>
#include <syslog.h>
#endif // __linux__

using namespace std;
using json = nlohmann::json;

void initSG90();
void hit();
void capture(const string& savePath);

int main()
{
#ifdef __linux__
	daemon(0, 0);
#endif

	initSG90();

	for (;;)
	{
		json req;
		severListen(req);
		auto cmd = req["cmd"].get<string>();
		if (cmd == string("hit"))
		{
			hit();
			req["ret"] = "hit ok";
			severReply(req);
		}
		else if (cmd == string("loophit"))
		{
			req["ret"] = "loophit ok";
			severReply(req);
		}
		else if (cmd == string("snap"))
		{
			string path = "/heat/camera/snap.jpg";
			capture(path);
			req["ret"] = "loophit ok";
			req["img"] = path;
			severReply(req);
		}
		else
		{
			req["ret"] = "unknown cmd";
			severReply(req);
		}
	}
}
