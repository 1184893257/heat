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

void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst)
{
    std::string::size_type pos = 0;
    std::string::size_type srclen = strsrc.size();
    std::string::size_type dstlen = strdst.size();

    while ((pos = strBig.find(strsrc, pos)) != std::string::npos)
    {
        strBig.replace(pos, srclen, strdst);
        pos += dstlen;
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

	string cmd = req["cmd"].get<string>();
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
		rsp.erase("results");
		
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

	if (cmd == string("snap"))
	{
		string button = R"(
			<form action="/cgi-bin/heat/heat.cgi" method="post">
				<input type="hidden" name="cmd" value ="loophit"/>
				<input type="hidden" name="rotate" value ="ROTATE"/>
				<table width="100%" border="1" cellpadding="10">
				  <tr>
					<td align="center">
						<input type="text" name="hour" value="6" /> 小时内
						<input type="submit" value="循环触发" />
					</td>
				  </tr>
				</table>
			</form>)";
		string_replace(button, "ROTATE", req["rotate"].get<string>());
		cout << button;
	}
	
	if (rsp.contains("img"))
	{
		// img 自带了双引号
		cout << "<img src=" << rsp["img"] << " alt=\"img\"/>\n";
	}
	cout << rsp.dump() << endl;

	cout << "</body></html>\n";
	return 0;
}