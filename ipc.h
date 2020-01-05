#pragma once
#include "json.hpp"

enum class Method {
	hit,
	loopHit,
	stop
};

void clientCall(const Method method, const nlohmann::json& req, nlohmann::json& rsp);
void severListen(Method& method, nlohmann::json& req);
void severReply(const nlohmann::json& rsp);