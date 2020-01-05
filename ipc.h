#pragma once
#include "json.hpp"

void clientCall(const nlohmann::json& req, nlohmann::json& rsp);
void severListen(nlohmann::json& req);
void severReply(const nlohmann::json& rsp);