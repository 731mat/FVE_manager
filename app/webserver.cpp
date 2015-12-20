#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include "../include/configuration.h"

bool serverStarted = false;
HttpServer server;
extern int vykon;

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	vars["T"] = String(vykon);
	vars["TIME"] = String(SystemClock.getSystemTimeString());
	response.sendTemplate(tmpl);
}

void onConfiguration(HttpRequest &request, HttpResponse &response)
{
	MeteoConfig cfg = loadConfig();
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		debugf("Update config");
		// Update config
		if (request.getPostParameter("SSID").length() > 0) // Network
		{
			cfg.NetworkSSID = request.getPostParameter("SSID");
			cfg.NetworkPassword = request.getPostParameter("Password");
		}
		if (request.getPostParameter("PRVNIhh").length() > 4 && request.getPostParameter("POSLEDNIhh").length() < 23 ) // Correction
		{
			cfg.AddPRVNIhh = request.getPostParameter("PRVNIhh").toInt();
			cfg.AddPOSLEDNIhh = request.getPostParameter("POSLEDNIhh").toInt();
		}
		saveConfig(cfg);
		response.redirect();
	}

	debugf("Send template");
	TemplateFileStream *tmpl = new TemplateFileStream("config.html");
	auto &vars = tmpl->variables();
	vars["SSID"] = cfg.NetworkSSID;
	vars["PRVNIhh"] = String(cfg.AddPRVNIhh, 2);
	vars["POSLEDNIhh"] = String(cfg.AddPOSLEDNIhh, 2);
	response.sendTemplate(tmpl);
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

/// API ///

void onApiDoc(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("api.html");
	auto &vars = tmpl->variables();
	vars["IP"] = (WifiStation.isConnected() ? WifiStation.getIP() : WifiAccessPoint.getIP()).toString();
	response.sendTemplate(tmpl);
}

void onApiSensors(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	json["status"] = (bool)true;
	JsonObject& sensors = json.createNestedObject("sensors");
	sensors["temperature"] = "a";
	sensors["humidity"] = "b";
	response.sendJsonObject(stream);
}

void onApiOutput(HttpRequest &request, HttpResponse &response)
{
	int val = request.getQueryParameter("control", "-1").toInt();
	if (val == 0 || val == 1)
		digitalWrite(CONTROL_PIN, val == 1);
	else
		val = -1;

	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	json["status"] = val != -1;
	if (val == -1) json["error"] = "Wrong control parameter value, please use: ?control=0|1";
	response.sendJsonObject(stream);
}

void startWebServer()
{
	if (serverStarted) return;

	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/api", onApiDoc);
	server.addPath("/api/sensors", onApiSensors);
	server.addPath("/api/output", onApiOutput);
	server.addPath("/config", onConfiguration);
	server.setDefaultHandler(onFile);
	serverStarted = true;

	if (WifiStation.isEnabled())
		debugf("STA: %s", WifiStation.getIP().toString().c_str());
	if (WifiAccessPoint.isEnabled())
		debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}

/// FileSystem Initialization ///

Timer downloadTimer;
HttpClient downloadClient;
int dowfid = 0;
void downloadContentFiles()
{
	if (!downloadTimer.isStarted())
	{
		downloadTimer.initializeMs(3000, downloadContentFiles).start();
	}

	if (downloadClient.isProcessing()) return; // Please, wait.
	debugf("DownloadContentFiles");

	if (downloadClient.isSuccessful())
		dowfid++; // Success. Go to next file!
	downloadClient.reset(); // Reset current download status

	if (dowfid == 0)
		downloadClient.downloadFile("http://10.10.1.14/MeteoControl/MeteoControl.html", "index.html");
	else if (dowfid == 1)
		downloadClient.downloadFile("http://10.10.1.14/MeteoControl/MeteoConfig.html", "config.html");
	else if (dowfid == 2)
		downloadClient.downloadFile("http://10.10.1.14/MeteoControl/MeteoAPI.html", "api.html");
	else if (dowfid == 3)
		downloadClient.downloadFile("http://10.10.1.14/MeteoControl/bootstrap.css.gz");
	else if (dowfid == 4)
		downloadClient.downloadFile("http://10.10.1.14/MeteoControl/jquery.js.gz");
	else
	{
		// Content download was completed
		downloadTimer.stop();
		startWebServer();
	}
}
