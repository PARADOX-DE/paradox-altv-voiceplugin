#include "websocket.hpp"
#include "functions.hpp"

bool CFunctions::JoinChannel(const char* channelname, const char* password, const char* username)
{
	if (this->serverHandle == -1) return false;

	uint64* Results = NULL;
	this->ts3functions.getChannelList(this->serverHandle, &Results);

	for (int i = 0; i < 1024; i++)
	{
		if (Results[i] == NULL) break;

		char* Name = NULL;
		this->ts3functions.getChannelVariableAsString(this->serverHandle, Results[i], CHANNEL_NAME, &Name);

		if (!strcmp(Name, channelname))
		{
			this->password = password;
			this->speechChannel = Results[i];
			this->ts3functions.freeMemory(Name);

			break;
		}
	}

	if (this->speechChannel == -1)
	{
		this->ts3functions.printMessageToCurrentTab("[color=blue][PARADOX Voice] Es konnte kein Ingame Channel gefunden werden!");
		return true;
	}
	else
	{
		anyID Client;
		if (this->ts3functions.getClientID(this->serverHandle, &Client) != ERROR_ok) return false;

		if (this->GetCurrentChannelId() != this->speechChannel)
		{
			this->lastChannel = this->GetCurrentChannelId();
			if (this->ts3functions.requestClientMove(this->serverHandle, Client, this->speechChannel, this->password, NULL) != ERROR_ok) return true;

			//ResetListenerPosition();
			this->ts3functions.systemset3DSettings(this->serverHandle, 2.0f, 6.0f);
			this->ts3functions.printMessageToCurrentTab("[color=blue][PARADOX Voice] Du befindest dich nun im Sprachchannel.");
		}

		if (!this->Changename(username)) return true;
	}

	this->SendServerCallback("success", "CONNECTED");

	this->ts3functions.freeMemory(Results);
	return true;
}

bool CFunctions::ResetListenerPosition() {
	TS3_VECTOR position;
	position.x = 0;
	position.y = 0;
	position.z = 0;

	TS3_VECTOR forward;
	forward.x = 0;
	forward.y = 0;
	forward.z = 0;

	TS3_VECTOR up;
	up.x = 0;
	up.y = 0;
	up.z = 1;

	if (this->ts3functions.systemset3DListenerAttributes(this->serverHandle, &position, &forward, &up) != ERROR_ok) {
		printf("[PARADOX][VOICE] Unable to reset 3D system settings \n");
		return false;
	}

	return true;
}

bool CFunctions::ConnectedToServer(uint64 serverHandle)
{
	this->serverHandle = serverHandle;
	this->ts3functions.printMessageToCurrentTab("[color=blue][PARADOX Voice] Du hast sich zu einem Server verbunden.");

	json data;
	data["method"] = "Connected";

	CWebSocket::Instance().Send(data.dump());
	return true;
}

bool CFunctions::Changename(const char* username)
{
	if (this->serverHandle == -1) return false;

	char* currentName = NULL;
	anyID Client = NULL;

	if (this->ts3functions.getClientID(this->serverHandle, &Client) != ERROR_ok) return false;
	if (this->ts3functions.getClientVariableAsString(this->serverHandle, Client, CLIENT_NICKNAME, &currentName) != ERROR_ok) return false;

	std::string currentTemp = currentName;

	this->ts3functions.freeMemory(currentName);
	if (currentTemp.find(username) != std::string::npos) return false;

	snprintf(this->Name, sizeof(this->Name), "%s", currentTemp.c_str());

	if (this->ts3functions.setClientSelfVariableAsString(this->serverHandle, CLIENT_NICKNAME, username) != ERROR_ok) return false;
	if (this->ts3functions.flushClientSelfUpdates(this->serverHandle, NULL) != ERROR_ok) return false;

	return true;
}

bool CFunctions::SetClientPosition(TS3_VECTOR Position) {
	this->ts3functions.systemset3DListenerAttributes(this->serverHandle, &Position, NULL, NULL);
	return true;
}

int GetClientsCount(anyID* clients) {
	int index = 0;
	anyID id = clients[index];

	while (id != 0) {
		index++;
		id = clients[index];
	}

	return index;
}

bool CFunctions::SetTargetPositions(json jsonData) {
	anyID* Clients = NULL;
	if (this->ts3functions.getChannelClientList(this->serverHandle, this->GetCurrentChannelId(), &Clients) != ERROR_ok) return false;

	cachedPlayers.clear();

	int length = GetClientsCount(Clients);
	for (int i = 0; i < length; i++)
	{
		if (Clients[i] == NULL) return false;

		char* tempUsername = NULL;
		if (this->ts3functions.getClientVariableAsString(this->serverHandle, Clients[i], CLIENT_NICKNAME, &tempUsername) != ERROR_ok) return NULL;

		bool isMuted = true;
		for (auto& target : jsonData["data"]) {
			auto name = target["name"].get<std::string>();
			auto posX = target["x"].get<int>();
			auto posY = target["y"].get<int>();
			auto posZ = target["z"].get<int>();
			auto distance = target["distance"].get<float>();
			auto voiceRange = target["voiceRange"].get<int>();
			auto volumeModifier = target["volumeModifier"].get<float>();

			if (name.find(tempUsername) != std::string::npos) {
				isMuted = false;

				TS3_VECTOR Position;
				Position.x = (float)posX;
				Position.y = (float)posY;
				Position.z = (float)posZ;

				this->ts3functions.channelset3DAttributes(this->serverHandle, Clients[i], &Position);

				CachedPlayer player{ name, Clients[i], Position, distance, voiceRange, volumeModifier };
				cachedPlayers.emplace_back(player);
			}
		}

		this->SetClientMuteState(Clients[i], isMuted);
		this->ts3functions.freeMemory(tempUsername);
	}

	this->ts3functions.freeMemory(Clients);
	return true;
}

uint64 CFunctions::GetCurrentChannelId()
{
	if (this->serverHandle == -1) return NULL;

	uint64 returnValue = NULL;
	anyID Client;

	if (this->ts3functions.getClientID(this->serverHandle, &Client) != ERROR_ok) return NULL;
	if (this->ts3functions.getChannelOfClient(this->serverHandle, Client, &returnValue) != ERROR_ok) return NULL;

	return returnValue;
}

bool CFunctions::SetClientMuteState(anyID clientId, bool state)
{
	if (this->serverHandle == -1) return false;

	int isMuted;

	anyID Client[2];
	Client[0] = clientId;
	Client[1] = -1;

	if (this->ts3functions.getClientVariableAsInt(this->serverHandle, clientId, CLIENT_IS_MUTED, &isMuted) != ERROR_ok) return false;
	this->microphoneMuted = isMuted;

	if (state)
	{
		if (!isMuted)
		{
			if (this->ts3functions.requestMuteClients(this->serverHandle, Client, NULL) != ERROR_ok) return false;
			else return true;
		}
	}
	else
	{
		if (isMuted)
		{
			if (this->ts3functions.requestUnmuteClients(this->serverHandle, Client, NULL) != ERROR_ok) return false;
			else return true;
		}
	}

	return true;
}

void CFunctions::SendServerCallback(std::string method, std::string callback)
{
	json data;
	data["method"] = method;
	data["data"]["callback"] = callback;

	CWebSocket::Instance().Send(data.dump());
}

anyID CFunctions::GetIdByName(const char* username)
{
	if (this->serverHandle == -1) return NULL;

	anyID returnClient = NULL;
	int Count = NULL;
	anyID* Clients = NULL;

	if (this->ts3functions.getClientList(this->serverHandle, &Clients) != ERROR_ok) return NULL;
	if (this->ts3functions.getServerVariableAsInt(this->serverHandle, VIRTUALSERVER_CLIENTS_ONLINE, &Count) != ERROR_ok) return NULL;

	for (int i = 0; i < Count; i++)
	{
		char* tempUsername = NULL;
		if (this->ts3functions.getClientVariableAsString(this->serverHandle, Clients[i], CLIENT_NICKNAME, &tempUsername) != ERROR_ok) return NULL;

		if (!strcmp(username, tempUsername))
		{
			returnClient = Clients[i];
			this->ts3functions.freeMemory(tempUsername);
			break;
		}

		this->ts3functions.freeMemory(tempUsername);
	}

	return returnClient;
}

int CFunctions::GetServerClientCount()
{
	int returnValue = 0;
	if (this->ts3functions.getServerVariableAsInt(this->serverHandle, VIRTUALSERVER_CLIENTS_ONLINE, &returnValue) != ERROR_ok) return NULL;

	return returnValue;
}

CachedPlayer* CFunctions::GetCachedPlayerById(anyID id) {
	for (auto& player : cachedPlayers) if (player.clientId == id) return &player;

	return nullptr;
}

void CFunctions::Reset() {
	auto oldName = this->Name;
	auto oldChannel = this->lastChannel;

	anyID Client;
	if (this->ts3functions.getClientID(this->serverHandle, &Client) != ERROR_ok) return;

	anyID* Clients = NULL;
	if (this->ts3functions.getChannelClientList(this->serverHandle, this->GetCurrentChannelId(), &Clients) != ERROR_ok) return;

	int length = GetClientsCount(Clients);
	for (int i = 0; i < length; i++) {
		TS3_VECTOR Position;

		Position.x = (float)0;
		Position.y = (float)0;
		Position.z = (float)0;

		this->ts3functions.channelset3DAttributes(this->serverHandle, Clients[i], &Position);
		this->SetClientMuteState(Clients[i], false);
	}

	if (this->ts3functions.setClientSelfVariableAsString(this->serverHandle, CLIENT_NICKNAME, oldName) != ERROR_ok) return;
	if (this->ts3functions.flushClientSelfUpdates(this->serverHandle, NULL) != ERROR_ok) return;
	if (this->ts3functions.requestClientMove(this->serverHandle, Client, oldChannel, "", NULL) != ERROR_ok) return;

	this->ResetListenerPosition();
}