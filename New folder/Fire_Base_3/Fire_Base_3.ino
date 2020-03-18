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

int Led = 2 ;

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

  pinMode(Led, OUTPUT);

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
  String ST_path_3 = "Controlling Data";

  String path2 = "/" + ST_path_1 + "/" + ST_path_2 + String(readingID);
  Serial.println(path2);
  String path3 = "/" + ST_path_1 + "/" + ST_path_3 ;
  Serial.println(path3);

  String x ;
  Firebase.getString(firebaseData, path3, x);
  Serial.println("get Sting is : ");
  Serial.println(x);
  if (x == "ON") {
    digitalWrite(Led, HIGH);
  }
  else if (x == "OFF") {
    digitalWrite(Led, LOW);
  }


  Read_Temperature();
  Read_Humidity();
  json2.set("/Temperature", Temperature);
  json2.set("/Humidity", Humidity);
  Firebase.set(firebaseData, path2, json2);

  delay(1000);
}
