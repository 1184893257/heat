#ifdef __linux__
#include <syslog.h>
#include <time.h>
#include "json.hpp"
#include "ipc.h"
#include "config.h"
using json = nlohmann::json;

void hit();
void capture(const string& savePath);
void rotate(const string& path, float rot);
string ocr(const string& picturePath);

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
		config.startTime = time(nullptr);
		auto hourStr = req["hour"].get<string>();
		int seconds = atoi(hourStr) * 60 * 60;
		config.endTime = config.startTime + seconds;
		config.results.clear();

		string now = getTime();
		string taskDir = "tasks/";
		taskDir += now;
		taskDir += "/";

		config.taskDir = taskDir;
		write_config();
		req["ret"] = "loophit ok";
		severReply(req);
	}
	else if (cmd == string("snap"))
	{
		auto rot_str = req["rotate"].get<string>();
		float rot = (float)atof(rot_str.c_str());
		string path_to_heat = "camera/snap.jpg";
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

bool captureAndHit(HeatResult& result)
{
	string now = getTime();
	result.hitTime = now;

	string captureDir = config.taskDir + now;
	captureDir += "/";
	if (0 != CreateDir(captureDir))
	{
		result.status = captureDir + " create fail";
		return false;
	}

	string beforeHitPicture = captureDir + "before.jpg";
	capture(beforeHitPicture);
	if (0 != access(beforeHitPicture.c_str(), F_OK))
	{
		result.status = beforeHitPicture + " capture fail";
		return false;
	}

	config.textBeforeHit = ocr(beforeHitPicture);
	int textLength = config.textBeforeHit.length();
	if (textLength != 3)
	{
		return false;
	}

	hit();

	string afterHitPicture = captureDir + "after.jpg";
	if (0 != access(afterHitPicture.c_str(), F_OK))
	{
		result.status = afterHitPicture + " capture fail";
		return false;
	}

	config.textAfterHit = ocr(afterHitPicture);
	textLength = config.textAfterHit.length();
	if (textLength != 3)
	{
		return false;
	}

	return true;
}

#endif