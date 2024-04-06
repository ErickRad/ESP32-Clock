#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <BluetoothSerial.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* api = "YOUR_OPENWHEATERMAP_API_KEY";
const char* city = "YOUR_COUNTRY";
const char* ntpServer = "YOUT_NTP_SERVER";
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
int idx = 0;

float temperature = 0.0;

bool telaLigada = true;

String weather;
String leitura = "";
String leituraAnterior = "";

struct Lembrete {
    String hora;
    String data;
    String desc;
};

std::vector<Lembrete> lembretes;

WiFiUDP ntpUDP;
NTPClient client(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);
HTTPClient http;
LiquidCrystal_I2C LCD(0x3F, 16, 2);
BluetoothSerial BT;


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

String filtro(String texto){
    String textoFinal = "";
    for(int i = 0; i < texto.length(); i++){
        if(isDigit(texto[i]) or isAlphaNumeric(texto[i]) or texto[i] == ' ' or texto[i] == ':' or texto[i] == '/' or texto[i] == '*' or texto[i] == '#' or texto[i] == '-'){
            textoFinal += texto[i];
        }
    }
    return textoFinal;
}

Lembrete criarLembrete(String texto){
    Lembrete lembrete;

    int posHora = texto.indexOf(' ');
    int posData = texto.indexOf(' ', posHora + 1);

    if (posHora != -1 && posData != -1) {
        lembrete.hora = texto.substring(0, posHora);
        lembrete.data = texto.substring(posHora + 1, posData);
        lembrete.desc = texto.substring(posData + 1);

        lembretes.push_back(lembrete);
        return lembrete;

    } else {
       throw;
       BT.print("Formato inválido");
    } 
}

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
  BT.begin("ESP32 Clock");

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

    if (BT.available() > 0) {
        leitura = BT.readStringUntil('\n');

        if(leitura != leituraAnterior){
            Lembrete novoLembrete = criarLembrete(filtro(leitura));
            leituraAnterior = leitura;

            for(idx; idx < lembretes.size(); idx++){
                String horario = lembretes[idx].hora.c_str();
                String dia = lembretes[idx].data.c_str();
                String descricao = lembretes[idx].desc.c_str();

                if(horario == formattedTime){
                    if(dia == formattedDate){
                        LCD.clear();
                        LCD.setCursor(5, 0);
                        LCD.print("ALERTA");

                        delay(1000);

                        LCD.clear();
                        LCD.setCursor(0, 0);
                        LCD.print(horario);
                        LCD.setCursor(12, 0);
                        LCD.print(dia);
                        LCD.setCursor(0, 1);
                        LCD.print(descricao);

                        delay(60000);
                    }
                }
            }
            idx = 0;
        } 
    }

    LCD.clear();

  }else{

    LCD.clear();

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