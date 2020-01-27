#pragma once

#include "json.hpp"
#include <string>
#include <vector>
#include <time.h>

using namespace std;
using json = nlohmann::json;

#if defined(__linux__)
#include <sys/types.h>
#include <unistd.h>
#else
using pid_t = int;
#endif

string path_to_webroot(const string& path_to_heat);
string path_to_root(const string& path_to_heat);
string readFile(const string& path);
void writeFile(const string& path, const string& text);

template<class T>
void read_struct(const string& path, T& t)
{
	string content = readFile(path);
	json j = json::parse(content);
	t = j.get<T>();
}

template<class T>
void write_struct(const string& path, const T& t)
{
	json j = t;
	writeFile(path, j.dump(4));
}

typedef struct {
  string textBeforeHit;
  string textAfterHit;
  string hitTime;
  string status;
}HeatResult;

typedef struct {
	bool split = true;
	pid_t daemonPid;
	float rotate;
	time_t startTime;
    time_t endTime;
	string taskDir;
	vector<HeatResult> results;
}GlobalConfig;

extern GlobalConfig config;

void to_json(json& j, const HeatResult& p);

void from_json(const json& j, HeatResult& p);

void to_json(json& j, const GlobalConfig& p);

void from_json(const json& j, GlobalConfig& p);

void init_config(bool isDaemon);

void write_config();

string getTime(time_t* timePtr = nullptr);

int CreateDir(const string& sPathName);