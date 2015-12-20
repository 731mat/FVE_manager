#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <user_config.h>
#include <SmingCore/SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "MAT-J" // Put you SSID and Password here
	#define WIFI_PWD ""
#endif


// Pin for trigger control output
//#define CONTROL_PIN 3 // UART0 RX pin
#define CONTROL_PIN 15

#define METEO_CONFIG_FILE ".meteo.conf" // leading point for security reasons :)

enum TriggerType
{
	eTT_None = 0,
	eTT_Temperature,
	eTT_Humidity
};

struct MeteoConfig
{
	MeteoConfig()
	{
		AddPRVNIhh = 0;
		AddPOSLEDNIhh = 0;
	}

	String NetworkSSID;
	String NetworkPassword;

	int AddPRVNIhh = 0;
	int AddPOSLEDNIhh = 0;

};

MeteoConfig loadConfig();
void saveConfig(MeteoConfig& cfg);

extern MeteoConfig ActiveConfig;

#endif /* INCLUDE_CONFIGURATION_H_ */
