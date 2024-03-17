#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* ntpServer = "a.ntp.br";
const long  gmtOffset_sec = -3 * 3600;
const unsigned int daylightOffset_sec = 0;

WiFiUDP ntpUDP;
NTPClient client(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);
LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x3F, 16, 2);

void setup() {
  LCD.init();

  LCD.setCursor(0, 0);
  LCD.print("Connecting to");
  LCD.setCursor(0, 1);
  LCD.print("WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED);

  LCD.clear();
  client.begin();
  client.update();
}

void loop() {
    unsigned long epochTime = client.getEpochTime();
    struct tm *timeInfo;
    timeInfo = localtime((time_t *)&epochTime);

    char formattedDate[12];
    char formattedTime[9];
    sprintf(formattedDate, "%2d/%02d/%4d", timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900);
    sprintf(formattedTime, "%02d:%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);

    LCD.setCursor(4, 0);
    LCD.print(formattedTime);
    LCD.setCursor(3, 1);
    LCD.print(formattedDate);
    
    delay(990);
}
