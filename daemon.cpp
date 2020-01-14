#include <string>
#include <iostream>
#include "ipc.h"

#ifdef __linux__
#include <unistd.h>
#include <syslog.h>
#endif // __linux__

const char* WEB_DIR = "/var/www/html/heat";

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
			string relPath = "/heat/camera/snap.jpg";
			string path = WEB_DIR;
			path += relPath;
			capture(path);
			req["ret"] = "loophit ok";
			req["img"] = relPath;
			severReply(req);
		}
		else
		{
			req["ret"] = "unknown cmd";
			severReply(req);
		}
	}
}
