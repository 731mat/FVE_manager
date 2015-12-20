#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "NtpClientDelegateDemo.h"


void onNtpReceive(NtpClient& client, time_t timestamp);

HttpServer server;

void onPrintSystemTime() {
	Serial.println(SystemClock.getSystemTimeString());
}

void onNtpReceive(NtpClient& client, time_t timestamp) {
	SystemClock.setTime(timestamp);

	Serial.print("Time synchronized: ");
	Serial.println(SystemClock.getSystemTimeString());
}
