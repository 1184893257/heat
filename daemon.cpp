#include <unistd.h>
#include <stdio.h>
#include <syslog.h>
#include "ipc.h"

using json = nlohmann::json;

int main()
{
	daemon(0, 0);
	for (;;)
	{
		Method method;
		json req;
		severListen(method, req);
		severReply(req);
	}
}
