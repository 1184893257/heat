#pragma once
#include "json.hpp"

void clientCall(const nlohmann::json& req, nlohmann::json& rsp);
void serverInit();
void serverListen(nlohmann::json& req);
void serverReply(const nlohmann::json& rsp);
