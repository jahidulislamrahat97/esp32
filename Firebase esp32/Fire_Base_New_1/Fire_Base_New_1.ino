#include <WiFi.h>
#include <FirebaseESP32.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>


#define WIFI_SSID "EduBlock.co"
#define WIFI_PASSWORD "12345STEMClub"
#define FIREBASE_HOST "iot-fishing.firebaseio.com"
#define FIREBASE_AUTH "2E0farRhMF2XXR87EZ1iVzUSqzmMaK3d4zjLBH0X"

FirebaseData firebaseData;

//************TDS and Temperature************//
namespace pin {
const byte tds_sensor = 36;   // Gravity TDS
const byte one_wire_bus = 14; // Dallas Temperature Sensor
}

namespace device {
float aref = 4.3;
}

namespace sensor {
float ec = 0;
unsigned int tds = 0;
float waterTemp = 0;
float ecCalibration = 1;
}

OneWire oneWire(pin::one_wire_bus);
DallasTemperature dallasTemperature(&oneWire);


//************pH************//
#define SensorPin 39// ph 
#define Offset 0.00            //deviation compensate
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex = 0;


// Variable for Control data path to Control load
String DB_Light ;
String DB_Heater ;
String DB_K1_In_Pump_1 ;
String DB_K1_Out_Pump_1 ;
String DB_K1_Out_Pump_2 ;

// Variable for live data path to show live data
float Temperature_data ;
float TDS_data ;
float pH_data ;

String Light_State ;
String Heater_State ;
String K1_In_Pump_1_State ;
String K1_Out_Pump_1_State ;
String K1_Out_Pump_2_State ;


//************Time************//
const char* ntpServer = "Bd.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
int second;
int minute;
int hour;
int day;
int month;
int year;
int weekday;
long current;
struct tm timeinfo;
String Date;
String Time;

void printLocalTime()
{
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  //Serial.println(&timeinfo, "%A, %d %B %Y %H:%M:%S");
}

void getdiv_time() {
  delay(1000);
  second = timeinfo.tm_sec;
  minute = timeinfo.tm_min;
  hour = timeinfo.tm_hour + 4;
  day = timeinfo.tm_mday;
  month = timeinfo.tm_mon + 1;
  year = timeinfo.tm_year + 1900;

  Date = (String(day) + "-" + String(month) + "-" + String(year));
  Serial.println(Date);

  if (1 <= hour && 12 >= hour) {
    Time = (String(hour) + "AM" + "_" + String(minute));
    Serial.println(Time);
  }
  else {
    hour = hour - 12;
    Time = (String(hour) + "PM" + "_" + String(minute));
    Serial.println(Time);
  }
}


String UUID_Path = "/XYZ" ;

String path1 = "/IOT Fishing" + UUID_Path + "/Live Data/Temperature" ;
String path2 = "/IOT Fishing" + UUID_Path + "/Live Data/pH" ;
String path3 = "/IOT Fishing" + UUID_Path + "/Live Data/TDS" ;
String path4 = "/IOT Fishing" + UUID_Path + "/Live Data/Light State" ;
String path5 = "/IOT Fishing" + UUID_Path + "/Live Data/Heater State" ;
String path6 = "/IOT Fishing" + UUID_Path + "/Live Data/K1 In Pump 1 State" ;
String path7 = "/IOT Fishing" + UUID_Path + "/Live Data/K1 Out Pump 1 State" ;
String path8 = "/IOT Fishing" + UUID_Path + "/Live Data/K1 Out Pump 2 State" ;

String path9  = "/IOT Fishing" + UUID_Path + "/Control Data/Light" ;
String path10 = "/IOT Fishing" + UUID_Path + "/Control Data/Heater" ;
String path11 = "/IOT Fishing" + UUID_Path + "/Control Data/K1 In Pump 1" ;
String path12 = "/IOT Fishing" + UUID_Path + "/Control Data/K1 Out Pump 1" ;
String path13 = "/IOT Fishing" + UUID_Path + "/Control Data/K1 Out Pump 2" ;






void setup() {
  Serial.begin(115200);
  dallasTemperature.begin();

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


  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //****Setting Live data path to send data from sensor and load state to show live data
  Firebase.setFloat(firebaseData, path1, Temperature_data);
  Firebase.setFloat(firebaseData, path2, pH_data);
  Firebase.setFloat(firebaseData, path3, TDS_data);
  Firebase.setString(firebaseData, path4, Light_State);
  Firebase.setString(firebaseData, path5, Heater_State);
  Firebase.setString(firebaseData, path6, K1_In_Pump_1_State);
  Firebase.setString(firebaseData, path7, K1_Out_Pump_1_State);
  Firebase.setString(firebaseData, path8, K1_Out_Pump_2_State);
  //****Setting Live data path to Receive data from Firebase to control load state
  Firebase.setString(firebaseData, path9, DB_Light);
  Firebase.setString(firebaseData, path10, DB_Heater);
  Firebase.setString(firebaseData, path11, DB_K1_In_Pump_1);
  Firebase.setString(firebaseData, path12, DB_K1_Out_Pump_1);
  Firebase.setString(firebaseData, path13, DB_K1_Out_Pump_2);




}


void Temperature_TDS_read() {

}

void ph_read() {


}

void loop() {
  printLocalTime();
  getdiv_time();

  //*************TDS*************//
  dallasTemperature.requestTemperatures();
  sensor::waterTemp = dallasTemperature.getTempCByIndex(0);
  float rawEc = analogRead(pin::tds_sensor) * device::aref / 4096.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float temperatureCoefficient = 1.0 + 0.02 * (sensor::waterTemp - 25.0); // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  sensor::ec = (rawEc / temperatureCoefficient) * sensor::ecCalibration; // temperature and calibration compensation
  sensor::tds = (133.42 * pow(sensor::ec, 3) - 255.86 * sensor::ec * sensor::ec + 857.39 * sensor::ec) * 0.5; //convert voltage value to tds value
  Serial.print(F("TDS:")); //Serial.println(sensor::tds);

  int TDS = sensor::tds ;
  if (TDS <= 0) {
    TDS_data = TDS;
  }

  Serial.println(TDS_data);
  Serial.print(F("EC:")); Serial.println(sensor::ec, 2);
  Temperature_data = sensor::waterTemp ;
  Serial.print(F("Temperature:")); //Serial.println(sensor::waterTemp, 2);
  Serial.println(Temperature_data);



  //*************ph*************//
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float voltage;

  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(SensorPin);
    if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
    pH_data = 3.5 * voltage + Offset;
    samplingTime = millis();
  }
  Serial.print("pH : ");
  Serial.println(pH_data);


  String Date_Path = ("/Date: " + Date + "" ) ;
  String Time_Path = ("/Time: " + Time + "") ;
  String path14 = "/IOT Fishing" + UUID_Path + "/Save Data" + Date_Path + Time_Path + "/Temperature" ;
  String path15 = "/IOT Fishing" + UUID_Path + "/Save Data" + Date_Path + Time_Path + "/pH" ;
  String path16 = "/IOT Fishing" + UUID_Path + "/Save Data" + Date_Path + Time_Path + "/TDS" ;
  String path17 = "/IOT Fishing" + UUID_Path + "/Save Data" + Date_Path + Time_Path + "/Light" ;




  Serial.println("--------------------------------------------------------------------------------");
  Serial.println("Firebase Path Setting Done: ");
  Firebase.setString(firebaseData, path14, String(Temperature_data));
  Firebase.setString(firebaseData, path15, String(pH_data));
  Firebase.setString(firebaseData, path16, String(TDS_data));
  Firebase.setString(firebaseData, path16, Light_State);
  Serial.println("Firebase Path Setting Done: ");

  Firebase.setFloat(firebaseData, path1, Temperature_data);
  Firebase.setFloat(firebaseData, path2, pH_data);
  Firebase.setFloat(firebaseData, path3, TDS_data);

}


double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount = 0;

  if (number < 5) { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  } else {
    if (arr[0] < arr[1]) {
      min = arr[0]; max = arr[1];
    }
    else {
      min = arr[1]; max = arr[0];
    }
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {
        amount += min;      //arr<min
        min = arr[i];
      } else {
        if (arr[i] > max) {
          amount += max;  //arr>max
          max = arr[i];
        } else {
          amount += arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount / (number - 2);
  }//if
  return avg;
}
