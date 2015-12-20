#include "../include/configuration.h"

#include <SmingCore/SmingCore.h>

MeteoConfig ActiveConfig;

MeteoConfig loadConfig()
{
	DynamicJsonBuffer jsonBuffer;
	MeteoConfig cfg;
	if (fileExist(METEO_CONFIG_FILE))
	{
		int size = fileGetSize(METEO_CONFIG_FILE);
		char* jsonString = new char[size + 1];
		fileGetContent(METEO_CONFIG_FILE, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		JsonObject& network = root["network"];
		cfg.NetworkSSID = String((const char*)network["ssid"]);
		cfg.NetworkPassword = String((const char*)network["password"]);

		JsonObject& correction = root["correction"];
		cfg.AddPRVNIhh = correction["PRVNIhh"];
		cfg.AddPOSLEDNIhh = correction["POSLEDNIhh"];

		delete[] jsonString;
	}
	else
	{
		cfg.NetworkSSID = WIFI_SSID;
		cfg.NetworkPassword = WIFI_PWD;
	}
	return cfg;
}

void saveConfig(MeteoConfig& cfg)
{
	ActiveConfig = cfg;

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& network = jsonBuffer.createObject();
	root["network"] = network;
	network["ssid"] = cfg.NetworkSSID.c_str();
	network["password"] = cfg.NetworkPassword.c_str();

	JsonObject& correction = jsonBuffer.createObject();
	root["correction"] = correction;
	correction["P"] = cfg.AddPRVNIhh;
	correction["POSLEDNIhh"] = cfg.AddPOSLEDNIhh;

	char buf[3048];
	root.prettyPrintTo(buf, sizeof(buf));
	fileSetContent(METEO_CONFIG_FILE, buf);
}
