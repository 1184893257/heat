#include <string>
#include <sstream>
#include <iostream>
#include "ocr.h"
using namespace std;

bool ocr_debug = false;

int main()
{
	auto builder = createOCRBuilder();
	if (ocr_debug)
	{
		cout << builder->setPath("snap.jpg")->setGrayRange(150, 250)->ocr() << endl;
	}
	else
	{
#if defined(__linux__) || defined(__APPLE__)
		cout << builder->setPath("ocr/heater.jpg")->ocr() << endl;
#else
		string dir = "D:\\temp\\MobileFile\\";
		for (int i = 0; i <= 9; ++i)
		{
			stringstream path;
			path << dir << i << ".jpg";

			cout << builder->setPath(path.str())->ocr() << endl;
		}
#endif
	}
	return 0;
}
