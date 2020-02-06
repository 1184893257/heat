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

static time_t heat_time = 4 * 3600;

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
	}
	else if (cmd == string("loophit"))
	{
		heat_time = 4 * 3600;
		config.startTime = time(nullptr);
		auto hourStr = req["hour"].get<string>();
		int seconds = atoi(hourStr.c_str()) * 60 * 60;
		config.endTime = config.startTime + seconds;
		auto rot = req["rotate"].get<string>();
		config.rotate = atof(rot.c_str());
		config.results.clear();

		string now = getTime();
		string taskDir = "tasks/";
		taskDir += now;
		taskDir += "/";

		config.taskDir = taskDir;
		write_config();
		req["ret"] = "loophit ok";
	}
	else if (cmd == string("snap"))
	{
		auto rot_str = req["rotate"].get<string>();
		float rot = (float)atof(rot_str.c_str());
		string path_to_heat = "camera/snap.jpg";
		string abs_path = path_to_root(path_to_heat);
		CreateDir(abs_path);
		capture(abs_path);
		if (0 == access(abs_path.c_str(), F_OK))
		{
			rotate(abs_path, rot);
		}
		req["ret"] = "snap ok";
		req["img"] = path_to_webroot(path_to_heat);
	}
	else
	{
		req["ret"] = "unknown cmd";
	}
	severReply(req);
}

#ifndef NOTPI

bool captureAndHit(HeatResult& result)
{
	string now = getTime();
	result.hitTime = now;

	string captureDir = config.taskDir + now;
	captureDir += "/";
	captureDir = path_to_root(captureDir);
	if (0 != CreateDir(captureDir))
	{
		result.status = captureDir + " create fail";
		return false;
	}

	string beforeHitPicture = captureDir + "before.jpg";
	capture(beforeHitPicture);
	rotate(beforeHitPicture, config.rotate);
	if (0 != access(beforeHitPicture.c_str(), F_OK))
	{
		result.status = beforeHitPicture + " capture fail";
		return false;
	}

	result.textBeforeHit = ocr(beforeHitPicture);
	int textLength = result.textBeforeHit.length();
	if (textLength != 3)
	{
		return false;
	}

	hit();

	string afterHitPicture = captureDir + "after.jpg";
	capture(afterHitPicture);
	rotate(afterHitPicture, config.rotate);
	if (0 != access(afterHitPicture.c_str(), F_OK))
	{
		result.status = afterHitPicture + " capture fail";
		return false;
	}

	result.textAfterHit = ocr(afterHitPicture);
	textLength = result.textAfterHit.length();
	if (textLength != 3)
	{
		return false;
	}

	return true;
}

#else
	

string fake_ocr()
{
	int left = (int)(config.startTime + heat_time - time(nullptr));
	if (left < 0)
		return "";
	left /= 60;
	string result;
	result += (left / 60) + '0';
	left = left % 60;
	result += (left / 10) + '0';
	result += (left % 10) + '0';
	return result;
}

bool captureAndHit(HeatResult& result)
{
	string now = getTime();
	result.hitTime = now;

	string captureDir = config.taskDir + now;
	captureDir += "/";
	captureDir = path_to_root(captureDir);
	if (0 != CreateDir(captureDir))
	{
		result.status = captureDir + " create fail";
		return false;
	}

	result.textBeforeHit = fake_ocr();
	int textLength = result.textBeforeHit.length();
	if (textLength != 3)
	{
		return false;
	}

	heat_time += 1800;

	result.textAfterHit = fake_ocr();
	textLength = result.textAfterHit.length();
	if (textLength != 3)
	{
		return false;
	}

	return true;
}

#endif

#endif