#include <WiFi.h>
#include <FirebaseESP32.h>
#include "DHT.h"

#define WIFI_SSID "EduBlock.co"
#define WIFI_PASSWORD "12345STEMClub"
#define FIREBASE_HOST "led-control-ce1b2.firebaseio.com"
#define FIREBASE_AUTH "NaODGSOCMYngnEJp258i8Crp2Lq0w07Kcg1QX69L"

RTC_DATA_ATTR int readingID = 0;


FirebaseJson  json2;
FirebaseData firebaseData;

#define DHTTYPE DHT22
#define DHTPIN 25
int Temperature;
int Humidity;
DHT dht(DHTPIN, DHTTYPE);

void Read_Temperature() {
  Temperature = dht.readTemperature();
  Serial.print("Temperature: ");
  Serial.println(Temperature);
}

void Read_Humidity() {
  Humidity = dht.readHumidity();
  Serial.print("Humidity: ");
  Serial.println(Humidity);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

}

void loop() {


  readingID++;

  String ST_path_1 = "IOT";
  String ST_path_2 = "Sensor Data";
  String path2 = "/" + ST_path_1 + "/" + ST_path_2 + String(readingID);
  String path3 = "/IOT/Sensor Data";
  Serial.println(path2);
  Read_Temperature();
  Read_Humidity();
  json2.set("/Temperature", Temperature);
  json2.set("/Humidity", Humidity);
  Firebase.set(firebaseData, path2, json2);

  delay(1000);
}
