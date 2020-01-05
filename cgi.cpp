#include <iostream>
#include <string>
#include <time.h>
#include "ipc.h"
using namespace std;
using json = nlohmann::json;

string urldecode(const string& encd);

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

	json rsp;
	clientCall(req, rsp);

	const char* endl = "<br>\n";
	cout << "Content-type:text/html\n\n"
		<< "<html>\n"
		<< "<head><title>welcome to c cgi.</title></head>\n<body>\n";

	cout << rsp.dump() << endl;

	cout << "</body></html>\n";
	return 0;
}