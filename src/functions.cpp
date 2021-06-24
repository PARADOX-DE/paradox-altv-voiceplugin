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
		this->SendServerCallback("error", "VOICECHANNEL_NOT_FOUND"); // würde das lieber mit enums etc. reworkn

		this->ts3functions.printMessageToCurrentTab("[color=white][PARADOX-VOICE] Es konnte kein Ingame Channel gefunden werden!");
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
			this->SendServerCallback("error", "ALREADY_IN_VOICE");

			this->ts3functions.printMessageToCurrentTab("[color=white][PARADOX-VOICE] Du befindest dich nun im Sprachchannel.");
		}

		if (!this->Changename(username)) return false;
	}

	this->SendServerCallback("success", "CONNECTED");

	this->ts3functions.freeMemory(Results);
	return true;
}

bool CFunctions::IsClientInVoice()
{
	if (this->serverHandle == -1) return false;
	if (this->speechChannel == -1) return false;

	return true;
}

bool CFunctions::ConnectedToServer(uint64 serverHandle)
{
	anyID Client;
	if (this->ts3functions.getClientID(this->serverHandle, &Client) != ERROR_ok) return false;

	this->serverHandle = serverHandle;
	
	int isMuted = 0;
	if (this->ts3functions.getClientVariableAsInt(this->serverHandle, Client, CLIENT_IS_MUTED, &isMuted) != ERROR_ok) return false;
	this->microphoneMuted = isMuted;

	//thats useless
	this->ts3functions.printMessageToCurrentTab("[color=white][PARADOX-VOICE] Du hast sich zu einem Server verbunden.");

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

bool CFunctions::SetClientPosition(TS3_VECTOR Position)
{
	if (this->serverHandle == -1) return false;

	this->ts3functions.systemset3DListenerAttributes(this->serverHandle, &Position, NULL, NULL);
	return true;
}

bool CFunctions::SetTargetPositions(json jsonData) {
	if (this->serverHandle == -1) return false;

	std::vector<std::string> client_username;
	std::vector<float> client_x;
	std::vector<float> client_y;
	std::vector<float> client_z;

	for (auto& element : jsonData["data"])
	{
		client_username.push_back(element["username"]);

		client_x.push_back(std::atof(element["x"].get<std::string>().c_str()));
		client_y.push_back(std::atof(element["y"].get<std::string>().c_str()));
		client_z.push_back(std::atof(element["z"].get<std::string>().c_str()));
	}

	anyID* Clients = NULL;
	if (this->ts3functions.getChannelClientList(this->serverHandle, this->GetCurrentChannelId(), &Clients) != ERROR_ok) return false;

	for (int i = 0; i < this->GetServerClientCount(); i++)
	{
		char* tempUsername = NULL;
		if (this->ts3functions.getClientVariableAsString(this->serverHandle, Clients[i], CLIENT_NICKNAME, &tempUsername) != ERROR_ok) return NULL;

		bool isMuted = true;
		for (int e = 0; e < jsonData["data"].size(); e++)
		{
			if (client_username[e].find(tempUsername) != std::string::npos)
			{
				isMuted = false;

				TS3_VECTOR Position;
				Position.x = client_x[e];
				Position.y = client_y[e];
				Position.z = client_z[e];

				this->ts3functions.channelset3DAttributes(this->serverHandle, Clients[i], &Position);
			}
		}

		this->SetClientMuteState(Clients[i], isMuted);
		this->ts3functions.freeMemory(tempUsername);
	}
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

	char tempo[128];
	sprintf_s(tempo, 128, "Count: %i\n", Count);
	this->ts3functions.printMessageToCurrentTab(tempo);

	for (int i = 0; i < Count; i++)
	{
		char* tempUsername = NULL;
		if (this->ts3functions.getClientVariableAsString(this->serverHandle, Clients[i], CLIENT_NICKNAME, &tempUsername) != ERROR_ok) return NULL;

		if (!strcmp(username, tempUsername))
		{
			returnClient = Clients[i];
			break;
		}
	}

	return returnClient;
}

int CFunctions::GetServerClientCount()
{
	int returnValue = 0;
	if (this->ts3functions.getServerVariableAsInt(this->serverHandle, VIRTUALSERVER_CLIENTS_ONLINE, &returnValue) != ERROR_ok) return NULL;

	return returnValue;
}
