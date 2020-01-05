#include "ipc.h"
#include <string>
using namespace std;
using json = nlohmann::json;

#if defined(__linux__)
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>

typedef struct
{
	long msgType;
	Method method;
	char json[100];
}Msg;

constexpr int PROJECT_ID = 'h';
constexpr long REQUEST = 1;
constexpr long RESPONSE = 2;

int initMsg(bool isSever)
{
	static int msgid = 0;
	if (msgid == 0)
	{
		key_t key = ftok("/home", PROJECT_ID);
		msgid = msgget(key, O_RDWR | 0777 | (isSever ? IPC_CREAT : 0));
		if (msgid < 0)
		{
			int err = errno;
			syslog(LOG_ERR, "msgget error %d %d", msgid, err);
			exit(-1);
		}
	}
	return msgid;
}

bool send(int msgid, long msgType, const Method method, const json& req, json& rsp)
{
	Msg m;
	string jsonStr = req.dump();
	if (jsonStr.length() + 1 > sizeof(m.json))
	{
		rsp["err"] = "message too long";
		rsp["jsonLength"] = jsonStr.length();
		rsp["json"] = jsonStr;
		return false;
	}

	m.msgType = REQUEST;
	m.method = method;
	strcpy(m.json, jsonStr.c_str());
	msgsnd(msgid, &m, sizeof(m) - sizeof(m.msgType), 0);
	return true;
}

void recv(int msgid, long msgType, Method* method, json& req)
{
	Msg rcv;
	msgrcv(msgid, &rcv, sizeof(rcv) - sizeof(rcv.msgType), msgType, 0);
	if (method)
	{
		*method = rcv.method;
	}
	req.parse(rcv.json);
}

void clientCall(const Method method, const nlohmann::json& req, nlohmann::json& rsp)
{
	int msgid = initMsg(false);
	if (!send(msgid, REQUEST, method, req, rsp))
		return;
	recv(msgid, RESPONSE, nullptr, rsp);
}

void severListen(Method& method, nlohmann::json& req)
{
	int msgid = initMsg(true);
	recv(msgid, REQUEST, &method, req);
}

void severReply(const nlohmann::json& rsp)
{
	json err;
	int msgid = initMsg(true);
	if (!send(msgid, RESPONSE, Method::hit, rsp, err))
	{
		syslog(LOG_ERR, "message too long %s", err.dump().c_str());
	}
}
#endif