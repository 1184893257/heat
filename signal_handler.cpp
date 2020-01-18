#ifdef __linux__
#include <syslog.h>
#include "json.hpp"
#include "ipc.h"
#include "config.h"
using json = nlohmann::json;

void hit();
void capture(const string& savePath);
void rotate(const string& path, float rot);

void handleSignal(int signum)
{
	json req;
	severListen(req);
	auto cmd = req["cmd"].get<string>();
	syslog(LOG_INFO, "recv cmd: %s", cmd.c_str());
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
		auto rot_str = req["rotate"].get<string>();
		float rot = (float)atof(rot_str.c_str());
		string path_to_heat = "/camera/snap.jpg";
		string abs_path = path_to_root(path_to_heat);
		capture(abs_path);
		rotate(abs_path, rot);
		req["ret"] = "snap ok";
		req["img"] = path_to_webroot(path_to_heat);
		severReply(req);
	}
	else
	{
		req["ret"] = "unknown cmd";
		severReply(req);
	}
}
#endif