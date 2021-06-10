#include "websocket.hpp"

void CWebSocket::Init() {
	this->webSocket = new ix::WebSocketServer(3005);

	webSocket->setOnClientMessageCallback([&](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& ws, const ix::WebSocketMessagePtr& msg) {
		Listen(connectionState, ws, msg);
	});

	auto res = webSocket->listen();
	if (!res.first) {
		printf("[PARADOX-Voice][WEBSOCKET] Failed to start...\n");
		return;
	}

	webSocket->disablePerMessageDeflate();
	webSocket->start();

	printf("[PARADOX-Voice][WEBSOCKET] Started. Waiting for stop...\n");
	webSocket->wait();
	printf("[PARADOX-Voice][WEBSOCKET] Stopped. Closing thread...\n");
	Sleep(2000);

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

}