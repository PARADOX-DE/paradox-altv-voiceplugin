#include "main.hpp"
#include "injector.hpp"

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 23

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

static char* pluginID = NULL;

#ifdef _WIN32
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if(WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif

const char* ts3plugin_name() {
#ifdef _WIN32
	static char* result = NULL;
	if(!result) {
		const wchar_t* name = L"PARADOX Voice";
		if(wcharToUtf8(name, &result) == -1) result = "PARADOX Voice";
	}

	return result;
#else
	return "PARADOX Voice";
#endif
}

const char* ts3plugin_version() {
    return "0.1";
}

int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

const char* ts3plugin_author() {
    return "PARADOX - Nova_ and Captcha.";
}

const char* ts3plugin_description() {
    return "GTA V - Voice plugin for alt:V Multiplayer (made by altMP Team)";
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}

int ts3plugin_init() {
    char appPath[PATH_BUFSIZE];
    char resourcesPath[PATH_BUFSIZE];
    char configPath[PATH_BUFSIZE];
	char pluginPath[PATH_BUFSIZE];

    printf("PLUGIN: init\n");

    ts3Functions.getAppPath(appPath, PATH_BUFSIZE);
    ts3Functions.getResourcesPath(resourcesPath, PATH_BUFSIZE);
    ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);
	ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE, pluginID);

	printf("PLUGIN: App path: %s\nResources path: %s\nConfig path: %s\nPlugin path: %s\n", appPath, resourcesPath, configPath, pluginPath);

    return 0;
}

void ts3plugin_shutdown() {
    printf("PLUGIN: shutdown\n");

	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);

	printf("PLUGIN: registerPluginID: %s\n", pluginID);
}

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {
	if (newStatus == STATUS_CONNECTION_ESTABLISHED) {
		char* clientLibVersion;

		if (ts3Functions.getClientLibVersion(&clientLibVersion) == ERROR_ok) {
			printf("PLUGIN: Client lib version: %s\n", clientLibVersion);
			ts3Functions.freeMemory(clientLibVersion);

			std::ifstream File(std::string("C:\\PARADOX\\client.dll").c_str(), std::ios::binary | std::ios::ate);
			if (File.fail()) {
				printf("PLUGIN: Error reading file C:/PARADOX/client.dll\n");
				//ts3Functions.logMessage("Error reading file C:/PARADOX/client.dll", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}

			DWORD srcSize = File.tellg();
			if (srcSize < 0x1000) {
				printf("PLUGIN: Invalid size of client.dll\n");
				//ts3Functions.logMessage("Invalid size of client.dll", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}

			BYTE* srcData = new BYTE[(UINT_PTR)srcSize];
			if (!srcData) {
				printf("PLUGIN: Error allocating client.dll\n");
				//ts3Functions.logMessage("Error allocating client.dll", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}

			File.seekg(0, std::ios::beg);
			File.read(reinterpret_cast<char*>(srcData), srcSize);
			File.close();

			auto gtaProcId = CInjector::GetProcId(L"GTA5.exe");
			if (gtaProcId == 0) {
				printf("PLUGIN: Error getting GTA5.exe ...\n");
				//ts3Functions.logMessage("Error getting GTA5.exe ...", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}

			auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, gtaProcId);
			if (!CInjector::ManualMap(hProcess, srcData)) {
				printf("PLUGIN: Error injecting client.dll into GTA V...\n");
				//ts3Functions.logMessage("Error injecting client.dll into GTA V...", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}
			else {
				printf("PLUGIN: Successful injecting client.dll into GTA V.\n");
				//ts3Functions.logMessage("Successful injecting client.dll into GTA V.", LogLevel_INFO, "Plugin", serverConnectionHandlerID);
			}
		}
		else {
			ts3Functions.logMessage("Error querying client lib version", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			return;
		}
	}
}