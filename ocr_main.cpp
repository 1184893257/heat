#include <string>
#include <sstream>
#include <iostream>
using namespace std;

string ocr(const string&);
string ocr(const string& picturePath, int min, int max);

bool ocr_debug = false;

int main()
{
	if (ocr_debug)
	{
		cout << ocr("snap.jpg", 150, 250) << endl;
	}
	else
	{
#if defined(__linux__) || defined(__APPLE__)
		cout << ocr("ocr/heater.jpg") << endl;
#else
		string dir = "D:\\temp\\MobileFile\\";
		for (int i = 0; i <= 9; ++i)
		{
			stringstream path;
			path << dir << i << ".jpg";

			cout << ocr(path.str()) << endl;
		}
#endif
	}
	return 0;
}
