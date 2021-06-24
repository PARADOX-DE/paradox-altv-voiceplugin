#pragma once
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"

#include "CSingleton.h"
#include "json.hpp"

using json = nlohmann::json;

class CFunctions : public CSingleton<CFunctions> {
private:
	char Name[128];
	bool clientsMuted = false;

public:
	uint64 serverHandle = -1;

	uint64 speechChannel = -1;
	uint64 lastChannel = -1;
	const char* password = "";

	bool ConnectedToServer(uint64 serverHandle);
	bool JoinChannel(const char* channelname, const char* password, const char* username);
	bool Changename(const char* username);
	bool SetClientPosition(TS3_VECTOR Position);
	bool SetTargetPositions(json jsonData);
	bool SetClientMuteState(anyID clientId, bool state);

	uint64 GetCurrentChannelId();
	anyID GetIdByName(const char* username);
	int GetServerClientCount();

	TS3Functions ts3functions;
};