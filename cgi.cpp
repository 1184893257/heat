#include <iostream>
#include <string>
#include <time.h>
#include <vector>
#include "ipc.h"
#include "config.h"
#if defined(__linux__)
#include <sys/types.h>
#include <signal.h>
#endif

using namespace std;
using json = nlohmann::json;

string urldecode(const string& encd);
void clear(vector<json>& v);

void decodeParam(const string& pair, json& req)
{
	int mid = pair.find('=');
	if (mid > 0 && mid < pair.length() - 1)
	{
		string key = urldecode(pair.substr(0, mid));
		string value = urldecode(pair.substr(mid + 1));
		req[key] = value;
	}
}

int main(int argc, char const *argv[], char const *env[])
{
	constexpr int SIZE = 1023;
	char buf[SIZE + 1] = { 0, };
	cin.read(buf, SIZE);

	string params = buf;
	json req;
	for (int start = 0; start < params.length();)
	{
		int end = params.find('&', start);
		if (end >= 0)
		{
			string pair = params.substr(start, end - start);
			decodeParam(pair, req);
			start = end + 1;
		}
		else
		{
			string pair = params.substr(start);
			decodeParam(pair, req);
			break;
		}
	}
	
	const char* endl = "<br>\n";
	cout << "Content-type:text/html\n\n"
		<< "<html>\n"
		<< "<head><title>welcome to c cgi.</title></head>\n<body>\n";

	string cmd = req["cmd"];
	json rsp;
	if (cmd == string("clear"))
	{
		vector<json> v;
		clear(v);
		rsp["msgs"] = v;
	}
	else if (cmd == string("status"))
	{
		init_config(false);
		GlobalConfig copy = config;
		copy.split = false;
		rsp = copy;
		
		if (config.taskDir.length() > 0)
		{
			cout << "<a href=\""
			  << path_to_webroot(config.taskDir)
			  << "\" target=\"_blank\">" << config.taskDir << "</a>" << endl;
		}
	}
	else
	{
		init_config(false);

#if defined(__linux__)
		kill(config.daemonPid, SIGUSR1);
#endif

		clientCall(req, rsp);
	}

	if (rsp.contains("img"))
	{
		// img �Դ���˫����
		cout << "<img src=" << rsp["img"] << " alt=\"img\"/>\n";
	}
	cout << rsp.dump(4) << endl;

	cout << "</body></html>\n";
	return 0;
}