#include <iostream>
#include <string>
using namespace std;

int main(int argc, char const *argv[], char const *env[])
{
	const char* endl = "<br>\n";

	cout << "Content-type:text/html\n\n"
		<< "<html>\n"
		<< "<head><title>welcome to c cgi.</title></head>\n<body>";

	cout << argc << ":" << endl;

	for (int i = 0; i < argc; ++i) {
		cout << argv[i] << endl;
	}

	cout << "env:" << endl;

	for (int i = 0; env[i]; ++i) {
		cout << env[i] << endl;
	}

	constexpr int SIZE = 1023;
	char buf[SIZE + 1] = {0,};
	cin.read(buf, SIZE);
	cout << "input:" << buf << endl;

	cout << "</body></html>\n";
	return 0;
}