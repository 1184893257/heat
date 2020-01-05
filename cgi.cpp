#include <iostream>
#include <string>
#include "ipc.h"
using namespace std;
using json = nlohmann::json;

int main(int argc, char const *argv[], char const *env[])
{
	const char* endl = "<br>\n";

	cout << "Content-type:text/html\n\n"
		<< "<html>\n"
		<< "<head><title>welcome to c cgi.</title></head>\n<body>";

	/*constexpr int SIZE = 1023;
	char buf[SIZE + 1] = {0,};
	cin.read(buf, SIZE);
	cout << "input:" << buf << endl;*/
	json req = {
		{"hello", "world"}
	};
	json rsp;
	clientCall(Method::hit, req, rsp);
	cout << rsp.dump() << endl;

	cout << "</body></html>\n";
	return 0;
}