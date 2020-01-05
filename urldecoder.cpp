#include <string>
using namespace std;

//解url编码实现 
string urldecode(const string& encd)
{
	int buf;
	unsigned char* p = (unsigned char*)(&buf);

	string decd;
	for (unsigned int i = 0; i < encd.length(); i++)
	{
		buf = 0;
		if (encd[i] != '%')
		{
			unsigned char ch = encd[i];
			if (ch == '+')
			{
				ch = ' ';
			}
			decd += ch;
			continue;
		}

		p[0] = encd[++i];
		p[1] = encd[++i];

		p[0] = p[0] - 48 - ((p[0] >= 'A') ? 7 : 0) - ((p[0] >= 'a') ? 32 : 0);
		p[1] = p[1] - 48 - ((p[1] >= 'A') ? 7 : 0) - ((p[1] >= 'a') ? 32 : 0);
		decd += (unsigned char)(p[0] * 16 + p[1]);
	}

	return decd;
}