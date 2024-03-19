#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* api = "YOUR_API_KEY";
const char* city = "YOUR_CITY";
const char* ntpServer = "YOUR_NTP_SERVER";
const String url = (String) "http://api.openweathermap.org/data/2.5/weather?q="+city+",IT&appid="+api+"&units=metric";
const long  gmtOffset_sec = -3 * 3600;
const unsigned int daylightOffset_sec = 0;

char formattedDate[12];
char formattedTime[9];
char formattedTemp[7];
unsigned long epochTime;
struct tm *timeInfo;
int lastHour = -1;

WiFiUDP ntpUDP;
NTPClient client(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);
LiquidCrystal_I2C LCD(0x3F, 16, 2);

byte grau[] = {
  B00111,
  B00101,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

float obterTemperatura() {
  HTTPClient http;

  
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
  
    String payload = http.getString();

    int startIndex = payload.indexOf("\"temp\":") + 7;
    int endIndex = payload.indexOf(",", startIndex);

    String temperaturaStr = payload.substring(startIndex, endIndex);
    float temperatura = temperaturaStr.toFloat();
    
    return temperatura;

  } else {
    return 28.3;
  }
  
  http.end();
}

void setup() {
  LCD.init();
  LCD.createChar(0, grau);

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
    epochTime = client.getEpochTime();
    timeInfo = localtime((time_t *)&epochTime);

    if (timeInfo->tm_hour != lastHour) {
        lastHour = timeInfo->tm_hour;
        sprintf(formattedTemp, "%.1f", obterTemperatura());
        LCD.setCursor(10, 0);
        LCD.print(formattedTemp);
        LCD.write(byte(0));
        LCD.print("C"); 
    }

    sprintf(formattedTime, "%02d:%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
    sprintf(formattedDate, "%02d/%02d/%04d", timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900);
    LCD.setCursor(0, 0);
    LCD.print(formattedTime);
    LCD.setCursor(3, 1);
    LCD.print(formattedDate);
    
    if(formattedTime == "00:00:00"){
      LCD.clear();
    }

    delay(990);

}
