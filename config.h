#pragma once

#include "json.hpp"
#include <string>

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

struct GlobalConfig {
	pid_t daemonPid;
};

extern GlobalConfig config;

void to_json(json& j, const GlobalConfig& p);

void from_json(const json& j, GlobalConfig& p);

void init_config(bool isDaemon);