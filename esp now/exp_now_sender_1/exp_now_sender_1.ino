#include <esp_now.h>
#include <WiFi.h>
#include "DHT.h"

uint8_t broadcastAddress[] = {0xA4, 0xCF, 0x12, 0x75, 0x78, 0x7C};

#define DHTTYPE DHT22
#define DHTPIN 25
DHT dht(DHTPIN, DHTTYPE);
int Temperature;
int Humidity;
typedef struct struct_message {
  char a[32];
  int b;
  int Temperature1;
  int Humidity1;
  float c;
  String d;
  bool e;
} struct_message;

struct_message myData;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

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
  dht.begin();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

}

void loop() {
  Read_Temperature();
  Read_Humidity();
  // Set values to send
  strcpy(myData.a, "THIS IS A CHAR");
  myData.b = random(1, 20);
  myData.c = 1.2;
  myData.Temperature1  = Temperature ;
  myData.Humidity1 = Humidity;
  myData.d = "Hello";
  myData.e = false;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(2000);

}
