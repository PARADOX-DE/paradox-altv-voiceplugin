#pragma on
#include "ixwebsocket/IXNetSystem.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketServer.h"
#include "CSingleton.h"
#include "json.hpp"

using json = nlohmann::json;

class CWebSocket : public CSingleton<CWebSocket> {
private:
	ix::WebSocketServer* webSocket = nullptr;
public:
	void Init();
	void Disable();

	void Listen(std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg);
};