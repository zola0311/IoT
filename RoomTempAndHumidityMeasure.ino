#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>

//Wifi elérés definiálása
#define WIFI_SSID "******"
#define WIFI_PASS "*******"
//askSensors api key megadása és az írási sebesség megadása milisec-ben
const char* apiKeyIn = "*******";
const unsigned int writeInterval = 25000;
//askSensors host megadása
String host = "http://api.asksensors.com";
String port = "8080";
 
// DHT11 pin és típus definiálás
#define DHTPIN D4 
#define DHTTYPE DHT11

// serial config
#define     RX    10
#define     TX    11
SoftwareSerial AT(RX,TX); 

int AT_cmd_time; 
boolean AT_cmd_result = false; 
 
DHT dht(DHTPIN, DHTTYPE);
 
void setup() 
{
  //Serial port configurálás és wifi csatlakozás elkezdése
  Serial.begin(115200);
  Serial.println();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  dht.begin();


  Serial.print("Csatlakozás: ");
  Serial.print(WIFI_SSID);
  //Folyamatos loop mindaddig, ameddig nem csatlakozik wifihez
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  //Wifi csatlakozás után üzenet amiben az ip-címet kiírjuk  
  Serial.println();
  Serial.print("Csatlakozva! Ip cím: ");
  Serial.println(WiFi.localIP());

  AT.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
}
 
void loop() 
{
  //Hőmérséklet és páratartalom mérés
  delay(2000);
  digitalWrite(LED_BUILTIN, LOW); 
 
  float h = dht.readHumidity();
  float t = dht.readTemperature();
 
  // Mérés ellenőrzése, ha szélsőértéken kívül van vagy hibás a mérés led jelzés
  if (isnan(h) || isnan(t) || h < 20.0 || h > 50.0 || t < 20.0 || t > 35.0) 
  {
    Serial.println("Sikertelen mérés vagy szélsőértéken kívüli mérés");
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);   
                                     
    delay(1000);                       
    digitalWrite(LED_BUILTIN, LOW);    
                                     
    delay(1000);  
    return;
  }
  //url készítés
  String url = "GET /write/";
  url += apiKeyIn;
  url += "?module1=";
  url += ((int) h);
    url += "?module2=";
  url += ((int) t);
  Serial.println("*****************************************************");
  Serial.println("********** TCP kapcsolódás nyitása ");
  sendATcmd("AT+CIPMUX=1", 10, "OK");
  sendATcmd("AT+CIPSTART=0, \"TCP\",\"" + host +"\"," + port, 20, "OK");
  sendATcmd("AT+CIPSEND=0," + String(url.length() + 4), 10, ">");
  
  Serial.print("********** URL: ");
  Serial.println(url);
  AT.println(url);
  delay(2000);
  sendATcmd("AT+CIPCLOSE=0", 10, "OK");
  
  Serial.println("********** TCP kapcsolódás zárása ");
  Serial.println("*****************************************************");
  
  delay(writeInterval);   // delay
 
  Serial.print("Páratartalom: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Hőmérséklet: ");
  Serial.print(t);
  Serial.print(" *C ");
 
}

void sendATcmd(String AT_cmd, int AT_cmd_maxTime, char readReplay[]) {
  Serial.print("AT parancs:");
  Serial.println(AT_cmd);

  while(AT_cmd_time < (AT_cmd_maxTime)) {
    AT.println(AT_cmd);
    if(AT.find(readReplay)) {
      AT_cmd_result = true;
      break;
    }
  
    AT_cmd_time++;
  }
  Serial.print("...Eredmény:");
  if(AT_cmd_result == true) {
    Serial.println("KÉSZ");
    AT_cmd_time = 0;
  }
  
  if(AT_cmd_result == false) {
    Serial.println("HIBA");
    AT_cmd_time = 0;
  }
  
  AT_cmd_result = false;
 }