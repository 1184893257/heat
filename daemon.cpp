#include <string>
#include <iostream>
#include "config.h"

using namespace std;
using json = nlohmann::json;

bool ocr_debug = false;

#ifdef __linux__
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
void initSG90();
void handleSignal(int);

int main()
{
	daemon(0, 0);
	initSG90();
	signal(SIGUSR1, handleSignal);
	init_config(true);

	for (;;)
	{
		sleep(10);
	}
}
#else
int main()
{
	json a = {
		{"a", 1}
	};
	cout << a.contains("a") << endl;
	return 0;
}
#endif // __linux__


