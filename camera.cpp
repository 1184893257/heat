#include <string>
#include <sstream>
#include <stdio.h>
using namespace std;

#if defined(__linux__)
#include <syslog.h>

void capture(const string& savePath)
{
	FILE *fp = NULL;
	char buff[128] = { 0 };
	stringstream cmd;
	cmd << "vgrabbj -q 100 -i vga -o jpg -f " << savePath << " -e -d /dev/video0 2>&1";
	fp = popen(cmd.str().c_str(), "r");
	fread(buff, 1, 127, fp);// ×èÈû¶Á
	syslog(LOG_INFO, "capture out: %s", buff);
	pclose(fp);
}

#endif