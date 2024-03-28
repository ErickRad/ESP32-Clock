#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_PASSWORD";
const char* api = "YOUR_API_KEY";
const char* city = "TOUR_CITY";
const char* ntpServer = "a.ntp.br";
const String url = (String) "http://api.openweathermap.org/data/2.5/weather?q="+city+",IT&appid="+api+"&units=metric";
const long  gmtOffset_sec = -3 * 3600;
const unsigned int daylightOffset_sec = 0;

char formattedDate[6];
char formattedTime[6];
char formattedTemp[7];
unsigned long epochTime;
struct tm *timeInfo;
int lastHour = -1;
int secs = 0;
float temperature = 0;
bool telaLigada = true;
String weather;

WiFiUDP ntpUDP;
NTPClient client(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);
HTTPClient http;
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

byte e[] = {
  B00001,
  B00010,
  B01110,
  B10001,
  B11111,
  B10000,
  B01110,
  B00000
};

void backlightOn() {
  Wire.beginTransmission(0x3F);
  Wire.write(0x08);
  Wire.endTransmission();
}

void backlightOff() {
  Wire.beginTransmission(0x3F);
  Wire.write(0x00);
  Wire.endTransmission();
}

void get_wheather() {
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    int tempStartIndex = payload.indexOf("\"temp\":") + 7;
    int tempEndIndex = payload.indexOf(",", tempStartIndex);
    temperature = payload.substring(tempStartIndex, tempEndIndex).toFloat();

    int weatherDescriptionStartIndex = payload.indexOf("\"main\":\"") + 8;
    int weatherDescriptionEndIndex = payload.indexOf("\"", weatherDescriptionStartIndex);
    weather = payload.substring(weatherDescriptionStartIndex, weatherDescriptionEndIndex);
    
    if (weather == "Clouds"){
      weather = " Nuvens";

    }else if (weather == "Rain"){
      weather = " Chuva";

    }else if (weather == "Drizzle"){
      weather = " Chuvisco";

    }else if(weather == "Thunderstorm"){
      weather = " Raios";

    }else if(weather == "Squall"){
      weather = " Tempest.";

    }else if(weather == "Snow"){
      weather = " Granizo";
    }
    http.end();
  }
}


void printTime(bool blink) {
  if(blink){
    sprintf(formattedTime, "%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min);
  }else{
    sprintf(formattedTime, "%02d %02d", timeInfo->tm_hour, timeInfo->tm_min);
  }
  
  LCD.setCursor(1, 0);
  LCD.print(formattedTime);
}

void printDate() {
  sprintf(formattedDate, "%02d/%02d", timeInfo->tm_mday, timeInfo->tm_mon + 1);
  LCD.setCursor(1, 1);
  LCD.print(formattedDate);
}

void printTemp() {
  sprintf(formattedTemp, "%.1f", temperature);
  LCD.setCursor(8, 0);
  LCD.print(formattedTemp);
  LCD.write(byte(0));
  LCD.print("C");

  LCD.setCursor(7, 1);
  if(weather == "Clear"){
    LCD.print("C");
    LCD.write(byte(1));
    LCD.print("u limpo");
  }else{
    LCD.print(weather);
  }
  
}



void setup() {
  LCD.init();
  Wire.begin();

  LCD.createChar(0, grau);
  LCD.createChar(1, e);
  
  LCD.setCursor(0, 0);
  LCD.print("Connecting to");
  LCD.setCursor(0, 1);
  LCD.print("WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED);
  
  client.begin();
  client.forceUpdate();
  get_wheather();

  LCD.clear();
}




void loop() {
  if(WiFi.status() == WL_CONNECTED){
    epochTime = client.getEpochTime();
    timeInfo = localtime((time_t *)&epochTime);
    
    printTemp();
    printDate();

    for(secs = timeInfo->tm_sec; secs < 120; secs++){
      if(secs % 2 == 0){
        printTime(true);
      }else{
        printTime(false);
      }
      delay(500);
      
    }

    if (lastHour != timeInfo->tm_hour) {
      get_wheather();
      lastHour = timeInfo->tm_hour;
    }
    
    LCD.clear();

  }else{
    
    LCD.setCursor(0, 0);
    LCD.print("Reconnecting");
    LCD.setCursor(0, 1);
    LCD.print("to WiFi...");

    WiFi.begin(ssid, password);
    while(WiFi.status() == WL_CONNECTED);

    client.begin();
    client.forceUpdate();
    get_wheather();
    LCD.clear();

  }
}
