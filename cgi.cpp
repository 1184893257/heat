#include <iostream>
#include <string>
#include <time.h>
#include <vector>
#include "ipc.h"
#include "config.h"
#include "base64.h"
#if defined(__linux__)
#include <sys/types.h>
#include <signal.h>
#include <syslog.h>
#endif

using namespace std;
using json = nlohmann::json;

string urldecode(const string& encd);
void clear(vector<json>& v);

void decodeParam(const string& pair, json& req)
{
	int mid = pair.find('=');
	if (mid > 0)
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
	cout << R"(Content-type:text/html

<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=GBK" />
		<meta name="viewport" content="width=device-idth,initial-scale=1">
		<title>cgi</title>
	</head>
<body>)";

	string cmd = req["cmd"].get<string>();
	json rsp;
	if (cmd == string("loophit"))
	{
		auto snapreq = req["req"].get<string>();
		req["req"] = base64_decode(snapreq);
	}

	if (cmd == string("clear"))
	{
		vector<json> v;
#if defined(__linux__)
		clear(v);
#endif
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
			rsp["startTime"] = getTime(&config.startTime);
			rsp["endTime"] = getTime(&config.endTime);
			
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
		clientCall(req, rsp);
#endif
	}

#if defined(__linux__)
	syslog(LOG_INFO, "cgi remote return, cmd: %s", cmd.c_str());
#endif

	if (cmd == string("snap"))
	{
		string button = R"(
			<form action="/cgi-bin/heat/heat.cgi" method="post">
				<input type="hidden" name="cmd" value ="loophit"/>
				<input type="hidden" name="req" value ="REQ"/>
				<table width="100%" border="1" cellpadding="10">
				  <tr>
					<td align="center">
						<input type="text" name="hour" value="6" /> Сʱ��
						<input type="submit" value="ѭ������" />
					</td>
				  </tr>
				</table>
			</form>)";
		auto reqStr = req.dump();
		auto ucharPtr = (unsigned char const*)(reqStr.c_str());
		string_replace(button, "REQ", base64_encode(ucharPtr, reqStr.length()));
		cout << button;
	}
	
	if (rsp.contains("imgs"))
	{
		for (auto& img : rsp["imgs"])
		{
			// �Դ���˫����
			cout << "<img src=" << img << " alt=\"img\"/>\n";
		}
	}
	cout << rsp.dump() << endl;

	cout << "</body></html>\n";
#if defined(__linux__)
	syslog(LOG_INFO, "cgi finish cmd: %s", cmd.c_str());
#endif
	return 0;
}
