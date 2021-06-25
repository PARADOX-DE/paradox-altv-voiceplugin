#include "websocket.hpp"
#include "json.hpp"
#include "functions.hpp"

void CWebSocket::Init() {
	ix::initNetSystem();
	this->webSocket = new ix::WebSocketServer(3005);

	webSocket->setOnClientMessageCallback([&](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& ws, const ix::WebSocketMessagePtr& msg) {
		Listen(connectionState, ws, msg);
	});

	auto res = webSocket->listen();
	if (!res.first) {
		printf("[PARADOX-Voice][WEBSOCKET] Failed to start: %s\n", res.second.c_str());
		return;
	}

	webSocket->disablePerMessageDeflate();
	webSocket->disablePong();
	webSocket->start();

	printf("[PARADOX-Voice][WEBSOCKET] Started. Waiting for stop...\n");
	webSocket->wait();
	printf("[PARADOX-Voice][WEBSOCKET] Stopped. Closing thread...\n");

	ix::uninitNetSystem();
	ExitThread(0);
}

void CWebSocket::Disable() {
	if (webSocket == nullptr) {
		printf("[PARADOX-Voice][WEBSOCKET] Failed to stop...\n");
		return;
	}

	webSocket->stop();
}

void CWebSocket::Listen(std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg) {
	if (msg->type == ix::WebSocketMessageType::Open) {

	}
	else if (msg->type == ix::WebSocketMessageType::Message) {
		if (!json::accept(msg->str)) {
			printf("[PARADOX-Voice][WEBSOCKET] Failed to parse received message.\n");
			return;
		}

		auto msgJson = json::parse(msg->str);
		auto method = msgJson["method"].get<std::string>();

		if (method.find("joinChannel") != std::string::npos) {
			if (msgJson["data"]["channel"].is_null()) return;
			if (msgJson["data"]["password"].is_null()) return;
			if (msgJson["data"]["username"].is_null()) return;

			auto channelName = msgJson["data"]["channel"].get<std::string>();
			auto password = msgJson["data"]["password"].get<std::string>();
			auto username = msgJson["data"]["username"].get<std::string>();

			if (!CFunctions::Instance().JoinChannel(channelName.c_str(), password.c_str(), username.c_str()))
			{
				CFunctions::Instance().ts3functions.printMessageToCurrentTab("[color=blue][PARADOX Voice] Stell sicher das du dich auf dem richtigen Teamspeak befindest![/color]");
				return;
			}

			CFunctions::Instance().ResetListenerPosition();
		}
		else if (method.find("setLocalPosition") != std::string::npos) {
			if (CFunctions::Instance().speechChannel == -1) return;
			if (CFunctions::Instance().serverHandle == -1) return;

			auto posX = msgJson["data"]["x"].get<int>();
			auto posY = msgJson["data"]["y"].get<int>();
			auto posZ = msgJson["data"]["z"].get<int>();

			TS3_VECTOR Position;
			Position.x = (float)posX;
			Position.y = (float)posY;
			Position.z = (float)posZ;

			if (!CFunctions::Instance().SetClientPosition(Position)) {
				printf("[PARADOX][VOICE] failed to set local position!");
				return;
			}
		}
		else if (method.find("updateTargetPositions") != std::string::npos) {
			if (CFunctions::Instance().speechChannel == -1) return;
			if (CFunctions::Instance().serverHandle == -1) return;

			if (!CFunctions::Instance().SetTargetPositions(msgJson)) {
				printf("[PARADOX][VOICE] failed to set target positions!");
				return;
			}
		}
		else if (method.find("isInVoice") != std::string::npos) 
			CFunctions::Instance().SendServerCallback("isInVoice", CFunctions::Instance().IsClientInVoice() ? "true" : "false");
	}
}

void CWebSocket::Send(std::string& data) {
	for (auto& client : webSocket->getClients()) client->send(data);
}