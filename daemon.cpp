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

#include <vector>
int main()
{
  GlobalConfig a = {
          1,
          2,
          3,
          "4",
          {
                  {
                    "111",
                    "222",
                    "3",
                    "4"
                  }
          }
  };
  json j = a;
  cout << j.dump(4) << endl;
  GlobalConfig b = j.get<GlobalConfig>();
	return 0;
}
#endif // __linux__


