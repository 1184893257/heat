#include "config.h"
#include <fstream>
#include <iostream>

using namespace std;

#ifdef __linux__
#include <sys/types.h>
#include <unistd.h>
#endif

static const char* WEB_ROOT = "/var/www/html/";
static const char* HEAT_TO_WEBROOT = "/heat/";
static const char* CONFIG_NAME = "config.json";

GlobalConfig config;

string path_to_webroot(const string& path_to_heat)
{
	string path = HEAT_TO_WEBROOT;
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

void writeFile(const string& path, const string& text)
{
	ofstream outfile(path, ios::trunc);
	outfile << text;
	outfile.close();
}

void to_json(json& j, const GlobalConfig& p)
{
	j = json{ { "daemonPid", p.daemonPid } };
}

void from_json(const json& j, GlobalConfig& p)
{
	j.at("daemonPid").get_to(p.daemonPid);
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