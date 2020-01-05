#include <stdio.h>

#include "ipc.h"

#ifdef __linux__
#include <unistd.h>
#include <syslog.h>
#endif // __linux__


using json = nlohmann::json;

int main()
{
#ifdef __linux__
	daemon(0, 0);
#endif

	for (;;)
	{
		json req;
		severListen(req);
		severReply(req);
	}
}
