#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "NtpClientDelegateDemo.h"
#include <Libraries/LiquidCrystal/LiquidCrystal.h>
///////////////////////////////////////////////////////////////////
// Výchozí nastavení SSID a hesla
#include "../include/configuration.h"
///////////////////////////////////////////////////////////////////

#include "special_chars.h"
#include "webserver.h"
#include "cas.h"

#define LED_PIN 15 // GPIO number
#define INT_PIN 0   // GPIO0
#define INT_LED_PIN 13


bool state = true;
int vykon = 0;
int preruseni = 0;

ntpClientDemo *demo;
Timer CasovacWykon;
LiquidCrystal lcd(12, 14, 16, 5, 4, 2);


void onDataSent(HttpClient& client, bool successful)
{
	String response = client.getResponseString();
	Serial.println("Pokus o odeslani byl "+ response);
}


HttpClient herokuSend;
void IRAM_ATTR interruptHandler()
{
	preruseni++;
	digitalWrite(INT_LED_PIN, true);
	delay(100);
	digitalWrite(INT_LED_PIN, false);
}


void vypocetWykonu(){
	if (preruseni*6*60 > 0 && preruseni*6*60 < 6000){
			vykon = preruseni*6*60;
			herokuSend.downloadString("http://fve.herokuapp.com/vstupDat?x=" + String(vykon), onDataSent);
	}
	preruseni = 0;
}


void connectOk();
void connectFail();

void init()
{
   lcd.begin(16, 2);
   lcd.print("hello, world!");



	spiffs_mount(); // Mount file system, in order to work with files
	attachInterrupt(INT_PIN, interruptHandler, RISING); // interrupt
	SystemClock.setTimeZone(1);
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial

	ActiveConfig = loadConfig();

	// Select control line
	pinMode(CONTROL_PIN, OUTPUT);

	WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start

	CasovacWykon.initializeMs(10000, vypocetWykonu);

}


void connectOk()
{
	demo = new ntpClientDemo();
	debugf("connected");
	WifiAccessPoint.enable(false);

	// At first run we will download web server content
	if (!fileExist("index.html") || !fileExist("config.html") || !fileExist("api.html") || !fileExist("bootstrap.css.gz") || !fileExist("jquery.js.gz"))
		downloadContentFiles();
	else{
		startWebServer();
		CasovacWykon.start();
	}
}


void connectFail()
{
	CasovacWykon.stop();
	debugf("connection FAILED");
	WifiAccessPoint.config("FVE_manager01", "", AUTH_OPEN);
	WifiAccessPoint.enable(true);

	Serial.println(WifiAccessPoint.getIP());
	startWebServer();
	WifiStation.waitConnection(connectOk); // Wait connection
}
