#include "ipc.h"
#include <string>
#include <vector>
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
	char json[200];
}Msg;

constexpr int PROJECT_ID = 'h';
constexpr long REQUEST = 1;
constexpr long RESPONSE = 2;

static int initMsg(bool isSever)
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

static bool send(int msgid, long msgType, const json& req, json& rsp)
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

	m.msgType = msgType;
	strcpy(m.json, jsonStr.c_str());
	msgsnd(msgid, &m, sizeof(m) - sizeof(m.msgType), 0);
	return true;
}

static bool recv(int msgid, long msgType, json& req, int msgflg = 0)
{
	Msg rcv;
	int bytes = msgrcv(msgid, &rcv, sizeof(rcv) - sizeof(rcv.msgType), msgType, msgflg);
	if (bytes > 0) {
		req = json::parse(rcv.json);
		return true;
	}
	return false;
}

void clear(vector<json>& v)
{
  int msgid = initMsg(false);
  json tmp;
  while (recv(msgid, 0, tmp, IPC_NOWAIT))
  {
    v.push_back(tmp);
  }
}

void clientCall(const nlohmann::json& req, nlohmann::json& rsp)
{
	int msgid = initMsg(false);
	if (!send(msgid, REQUEST, req, rsp))
		return;
	recv(msgid, RESPONSE, rsp);
}

void serverInit()
{
	initMsg(true);
}

void serverListen(nlohmann::json& req)
{
	int msgid = initMsg(true);
	recv(msgid, REQUEST, req);
}

void serverReply(const nlohmann::json& rsp)
{
	json err;
	int msgid = initMsg(true);
	if (!send(msgid, RESPONSE, rsp, err))
	{
		syslog(LOG_ERR, "message too long %s", err.dump().c_str());
	}
}
#endif