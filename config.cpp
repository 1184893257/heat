#include "config.h"
#include <fstream>
#include <iostream>

using namespace std;

#ifdef __linux__
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

static const char* WEB_ROOT = "/var/www/html/";
static const char* HEAT_TO_WEBROOT = "heat/";
static const char* CONFIG_NAME = "config.json";

#else

static const char* WEB_ROOT = "D:/temp/";
static const char* HEAT_TO_WEBROOT = "heat/";
static const char* CONFIG_NAME = "config.json";

#endif

GlobalConfig config;

string path_to_webroot(const string& path_to_heat)
{
	string path = "/";
	path += HEAT_TO_WEBROOT;
	path += path_to_heat;
	return path;
}

string path_to_root(const string& path_to_heat)
{
	string path = WEB_ROOT;
	path += HEAT_TO_WEBROOT;
	path += path_to_heat;
	return path;
}

string readFile(const string& path)
{
	ifstream infile(path);
	string out;
	if (infile.is_open())
	{
		string s;
		while (getline(infile, s))
		{
			out += s;
			out += "\n";
		}
		infile.close();             //关闭文件输入流 
	}
	return out;
}

#ifdef __linux__
int CreateDir(const string& sPathName)
{
	char DirName[256];
	strcpy(DirName, sPathName.c_str());
	int i, len = strlen(DirName);
	for (i = 1; i<len; i++)
	{
		if (DirName[i] == '/')
		{
			DirName[i] = 0;
			if (access(DirName, F_OK) != 0)
			{
				if (mkdir(DirName, 0755) == -1)
				{
					syslog(LOG_ERR, "mkdir fail: %s", DirName);
					return 1;
				}
			}
			DirName[i] = '/';
		}
	}

	return 0;
}
#else
int CreateDir(const string& sPathName)
{
	return 0;
}
#endif

void writeFile(const string& path, const string& text)
{
	if (0 == CreateDir(path.c_str()))
	{
		ofstream outfile(path, ios::trunc);
		outfile << text;
		outfile.close();
	}
}

void to_json(json& j, const HeatResult& p)
{
  j = json{
    { "textBeforeHit", p.textBeforeHit },
    { "textAfterHit", p.textAfterHit},
    { "hitTime", p.hitTime},
    { "status", p.status}
  };
}

void from_json(const json& j, HeatResult& p)
{
  j.at("textBeforeHit").get_to(p.textBeforeHit);
  j.at("textAfterHit").get_to(p.textAfterHit);
  j.at("hitTime").get_to(p.hitTime);
  j.at("status").get_to(p.status);
}

void to_json(json& j, const GlobalConfig& p)
{
	j = json{
		{ "daemonPid", p.daemonPid }
	};
	if (p.taskDir.length() > 0)
	{
		j["taskDir"] = p.taskDir;

		// 拆开存储，因为任务结束 config 任务数据就会清洗
		json detail = {
			{ "startTime", p.startTime },
			{ "endTime", p.endTime },
			{ "results", p.results },
			{ "rotate", p.rotate}
		};
		if (p.split)
		{
			writeFile(path_to_root(p.taskDir + CONFIG_NAME), detail.dump(4));
		}
		else
		{
			j.merge_patch(detail);
		}
	}
}

void from_json(const json& j, GlobalConfig& p)
{
	j.at("daemonPid").get_to(p.daemonPid);
	if (j.contains("taskDir"))
	{
		j.at("taskDir").get_to(p.taskDir);
		string detailStr = readFile(path_to_root(p.taskDir + CONFIG_NAME));
		json detail = json::parse(detailStr);
		detail.at("startTime").get_to(p.startTime);
		detail.at("endTime").get_to(p.endTime);
		detail.at("results").get_to(p.results);
		if (detail.contains("rotate"))
		{
			detail.at("rotate").get_to(p.rotate);
		}
	}
}

void write_config()
{
	string config_path = path_to_root(CONFIG_NAME);
	write_struct(config_path, config);
}

void init_config(bool isDaemon)
{
#ifdef __linux__
	string config_path = path_to_root(CONFIG_NAME);
	if (0 == access(config_path.c_str(), F_OK))
	{
		read_struct(config_path, config);
	}
	if (isDaemon)
	{
		config.daemonPid = getpid();
		write_config();
	}
#endif
}

string getTime(const time_t* timePtr)
{
#ifdef __linux__
	char buf[40];
	time_t now = time(nullptr);
	struct tm *p = localtime(timePtr == nullptr ? &now : timePtr); /*取得当地时间*/
	sprintf(buf, "%d%02d%02d_%02d-%02d-%02d", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	return string(buf);
#else
	return "";
#endif
}