#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "time.h"
#include <FirebaseESP32.h>

//************TDS and Temperature************//
namespace pin {
const byte tds_sensor = 36;   // Gravity TDS
const byte one_wire_bus = 25; // Dallas Temperature Sensor
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

//************SSID and Password************//
#define WIFI_SSID "EduBlock.co"
#define WIFI_PASSWORD "12345STEMClub"
#define FIREBASE_HOST "led-control-ce1b2.firebaseio.com"
#define FIREBASE_AUTH "NaODGSOCMYngnEJp258i8Crp2Lq0w07Kcg1QX69L"


//----------------------------------------//
WiFiServer server(80);
WiFiClient client;

//----------------------------------------//
String header;    // Variable to store the HTTP request
String PumpState = "off";
String LightState = "off";
String HeaterState = "off";
String FeedState = "off";

//----------------------------------------//
const int Pump = 4;// No need
const int Light = 17;
const int Heater = 16;
const int Oxyzen = 5;
const int K1_Pump_in = 13;
const int K1_Pump_out1 = 12;
const int K1_Pump_out2 = 27;
const int Filter_Pump = 26;
const int Feed = 4;// No need

//----------------------------------------//
RTC_DATA_ATTR int readingID = 0;

//----------------------------------------//
FirebaseJson  json2;
FirebaseData firebaseData;

//----------------------------------------//

String path1 =  "/IOT Fishing/Logging ID" ;
String path3 = "/IOT Fishing/Controlling Data/Pump" ;
String path4 = "/IOT Fishing/Controlling Data/LIGHT" ;
String path5 = "/IOT Fishing/Controlling Data/Heater" ;
String path6 = "/IOT Fishing/Controlling Data/Oxyzen" ;
String path7 = "/IOT Fishing/Controlling Data/Feed" ;
String path8 = "/IOT Fishing/Controlling Data/Filter Pump" ;

//----------------------------------------//
String Incoming_Data = "";

void setup() {
  Serial.begin(115200);
  dallasTemperature.begin();

  //----------------------------------------//
  pinMode(Pump, OUTPUT);
  pinMode(Light, OUTPUT);
  pinMode(Heater, OUTPUT);
  pinMode(Feed, OUTPUT);
  pinMode(Oxyzen, OUTPUT);
  pinMode(K1_Pump_in, OUTPUT);
  pinMode(K1_Pump_out1, OUTPUT);
  pinMode(K1_Pump_out2, OUTPUT);
  pinMode(Filter_Pump, OUTPUT);
  pinMode(SensorPin, INPUT);


  //----------------------------------------//
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //----------------------------------------//
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  //----------------------------------------//
  server.begin();


  //************init and get the time************//
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  //----------------------------------------//
  //  digitalWrite(Pump, HIGH);
  //  digitalWrite(Light, HIGH);
  //  digitalWrite(Heater, HIGH);
  //  digitalWrite(Feed, HIGH);
  //  digitalWrite(Oxyzen, HIGH);
  //  digitalWrite(K1_Pump_in, HIGH);
  //  digitalWrite(K1_Pump_out, HIGH);


  Serial.println("--------------------------------------------------------------------------------");
  Serial.println("Firebase Path Setting Done: ");
  Firebase.setString(firebaseData, path3, Incoming_Data);
  Firebase.setString(firebaseData, path4, Incoming_Data);
  Firebase.setString(firebaseData, path5, Incoming_Data);
  Firebase.setString(firebaseData, path6, Incoming_Data);
  Firebase.setString(firebaseData, path7, Incoming_Data);
  Firebase.setString(firebaseData, path8, Incoming_Data);
  Serial.println(path1);
  Serial.println(path3);
  Serial.println(path4);
  Serial.println(path5);
  Serial.println(path6);
  Serial.println(path7);
  Serial.println(path8);



}

void printLocalTime()
{
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %d %B %Y %H:%M:%S");
}


void getdiv_time() {
  delay(1000);
  second = timeinfo.tm_sec;
  minute = timeinfo.tm_min;
  hour = timeinfo.tm_hour + 5;
  day = timeinfo.tm_mday;
  month = timeinfo.tm_mon + 1;
  year = timeinfo.tm_year + 1900;
  weekday = timeinfo.tm_wday + 1;
  Serial.println(hour);
}


void loop() {
  client = server.available();   // Listen for incoming clients

  //*************TDS*************//
  dallasTemperature.requestTemperatures();
  sensor::waterTemp = dallasTemperature.getTempCByIndex(0);
  float rawEc = analogRead(pin::tds_sensor) * device::aref / 4096.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float temperatureCoefficient = 1.0 + 0.02 * (sensor::waterTemp - 25.0); // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  sensor::ec = (rawEc / temperatureCoefficient) * sensor::ecCalibration; // temperature and calibration compensation
  sensor::tds = (133.42 * pow(sensor::ec, 3) - 255.86 * sensor::ec * sensor::ec + 857.39 * sensor::ec) * 0.5; //convert voltage value to tds value
  Serial.print(F("TDS:")); //Serial.println(sensor::tds);

  float tdsread = sensor::tds ;
  Serial.println(tdsread);
  Serial.print(F("EC:")); Serial.println(sensor::ec, 2);
  float tempread = sensor::waterTemp ;
  Serial.print(F("Temperature:")); //Serial.println(sensor::waterTemp, 2);
  Serial.println(tempread);


  //*************ph*************//
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float voltage;
  float ph;
  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(SensorPin);
    if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
    ph = 3.5 * voltage + Offset;
    samplingTime = millis();
  }
  Serial.print("pH : ");
  Serial.println(ph);


  //*************feed & light controll*************//
  {
    printLocalTime();
    getdiv_time();

    if ((hour >= 6) && (hour <= 18)) {
      digitalWrite(Light, LOW);
      Serial.println("LIGHT ON");
      LightState = "on";
    }
    else {
      digitalWrite(Light, HIGH);
      Serial.println("LIGHT OFF");
      LightState = "off";
    }
    if ((hour == 9) || (hour == 15)) {
      Serial.println("Feeding Done");
      // Feed.write(60);
      //  delay(1000);
      // Feed.write(90);
    }
  }

  //*************Heater control*************//
  {
    if (tempread < 22) {
      digitalWrite(Heater, LOW);
      HeaterState = "on";
    }
    else if (tempread >= 26) {
      digitalWrite(Heater, HIGH);
      HeaterState = "off";
    }
  }

  //*************  *************//
  digitalWrite(Oxyzen, LOW);
  digitalWrite(K1_Pump_in, LOW);
  digitalWrite(K1_Pump_out1, LOW);
  digitalWrite(K1_Pump_out2, LOW);
  digitalWrite(Filter_Pump, LOW);


  //----------------------------------------//
  readingID++;
  String path2 = path1 + String(readingID);




  //----------------------------------------//
  // Control from Firebase
  Firebase.getString(firebaseData, path3, Incoming_Data);
  Firebase.getString(firebaseData, path4, Incoming_Data);
  Firebase.getString(firebaseData, path5, Incoming_Data);
  Firebase.getString(firebaseData, path6, Incoming_Data);
  Firebase.getString(firebaseData, path7, Incoming_Data);
  Firebase.getString(firebaseData, path8, Incoming_Data);
  Serial.println("Data from path3 : ");
  Serial.println(Incoming_Data);
  if (Incoming_Data == "Pump ON") {
    digitalWrite(Pump, HIGH);
  }
  else if (Incoming_Data == "Pump OFF") {
    digitalWrite(Pump, LOW);
  }
  if (Incoming_Data == "Light ON") {
    digitalWrite(Light, HIGH);
  }
  else if (Incoming_Data == "Light OFF") {
    digitalWrite(Light, LOW);
  }
  if (Incoming_Data == "Feed ON") {
    digitalWrite(Feed, HIGH);
  }
  else if (Incoming_Data == "Feed OFF") {
    digitalWrite(Feed, LOW);
  }
  if (Incoming_Data == "Heater ON") {
    digitalWrite(Heater, HIGH);
  }
  else if (Incoming_Data == "Heater OFF") {
    digitalWrite(Heater, LOW);
  }



  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected

      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              PumpState = "on";
              digitalWrite(Pump, LOW);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              PumpState = "off";
              digitalWrite(Pump, HIGH);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              HeaterState = "on";
              digitalWrite(Heater, LOW);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              HeaterState = "off";
              digitalWrite(Heater, HIGH);
            } else if (header.indexOf("GET /33/on") >= 0) {
              Serial.println("GPIO 33 on");
              LightState = "on";
              digitalWrite(Light, LOW);
            } else if (header.indexOf("GET /33/off") >= 0) {
              Serial.println("GPIO 33 off");
              LightState = "off";
              digitalWrite(Light, HIGH);
            } else if (header.indexOf("GET /25/on") >= 0) {
              Serial.println("Feeding done");
              FeedState = "on";
              digitalWrite(Feed, LOW);
              //Feed.write(90);
              // delay(1000);
              //Feed.write(90);
            }
            else if (header.indexOf("GET /25/off") >= 0) {
              Serial.println("GPIO 25 off");
              FeedState = "off";
              digitalWrite(Feed, HIGH);
            }

            // Display the HTML web page

            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<body>");
            client.println("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\">");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js\">");
            client.println("</script>");
            client.println(" <script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\">");
            client.println("</script>");


            client.print("<div class=\"container\">");
            client.print("<div class=\"row\">");
            client.print("<div class=\"  col-xs-8 col-sm-12\">");
            client.print("<div class=\"panel panel-primary\">");
            client.println(" <div class=\"panel-heading\">");
            client.println(" <h3 class=\"panel-title text-center\">IOT FISHING</h3>");
            client.println(" </div>");
            client.println(" <div class=\"panel-body\">");

            // client.println(" Liight");
            client.println(" <div class=\"row\">");
            client.print("<div class=\" col-sm-3\">");
            client.print("<h3 class=\"alert alert-info text-center\" role=\"alert\">PUMP</h3>");
            if (PumpState == "off") {
              client.print("<span class=\"label label-danger\">Status Off</span>");
            }
            else {
              client.print("<span class=\"label label-success\">Status On</span>");
            }

            client.println("<br><br>");
            client.println("<a href=\"/26/on\"\"><button type=\"button\"class=\"btn btn-success\">Turn On </button></a>");
            client.println("<a href=\"/26/off\"\"><button type=\"button\"class=\"btn btn-danger\">Turn Off </button></a>");




            client.println("</div>");

            client.print("<div class=\"  col-sm-3\">");

            client.print("<h3 class=\"alert alert-info text-center\" role=\"alert\">LIGHT</h3>");
            if (LightState == "off") {
              client.print("<span class=\"label label-danger\">Status Off</span>");
            }
            else {
              client.print("<span class=\"label label-success\">Status On</span>");
            }
            client.println("<br><br>");
            client.println("<a href=\"/27/on\"\"><button type=\"button\"class=\"btn btn-success\">Turn On </button></a>");
            client.println("<a href=\"/27/off\"\"><button type=\"button\"class=\"btn btn-danger\">Turn Off </button></a>");


            client.println("</div>");

            client.print("<div class=\"  col-sm-3\">");

            client.print("<h3 class=\"alert alert-info text-center\" role=\"alert\">Heater</h3>");
            if (HeaterState == "off") {
              client.print("<span class=\"label label-danger\">Status Off</span>");
            }
            else {
              client.print("<span class=\"label label-success\">Status On</span>");
            }
            client.println("<br><br>");
            client.println("<a href=\"/33/on\"\"><button type=\"button\"class=\"btn btn-success\">Turn On </button></a>");
            client.println("<a href=\"/33/off\"\"><button type=\"button\"class=\"btn btn-danger\">Turn Off </button></a>");



            // client.print("<h3 class=\"alert alert-info text-center\" role=\"alert\">LIGHT</h3>");
            client.println("<br><br>");


            client.println("</div>");
            // client.println("</div>");
            //  client.println("</div>");


            //client.println("</div>");
            client.print("<div class=\"  col-sm-3\">");
            client.print("<h3 class=\"alert alert-info text-center\" role=\"alert\">FEED</h3>");
            if (FeedState == "off") {
              client.print("<span class=\"label label-danger\">Status Off</span>");
            }
            else {
              client.print("<span class=\"label label-success\">Status On</span>");
            }
            client.println("<br><br>");
            client.println("<a href=\"/25/on\"\"><button type=\"button\"class=\"btn btn-success\">Turn On </button></a>");
            client.println("<a href=\"/25/off\"\"><button type=\"button\"class=\"btn btn-danger\">Turn Off </button></a>");



            // client.print("<h3 class=\"alert alert-info text-center\" role=\"alert\">LIGHT</h3>");
            client.println("<br><br>");


            client.println("</div>");
            // client.println("</div>");
            //  client.println("</div>");
            client.println("<br><br>");


            client.println("</div>");


            client.print("<div class=\"  col-sm-3\">");

            client.println("<br><br>");


            client.println("</div>");
            client.println("</div>");
            //client.println("</div>");



            client.println(" <div class=\"row\">");
            client.print("<div class=\" col-sm-3\">");
            client.print("<h3 class=\"alert alert-info text-center\" role=\"alert\">Temperature</h3>");
            client.print("<h4 class=\" text-center\"  style=\"color:Red\">");
            client.print(sensor::waterTemp);
            client.print("C");
            client.print("</h4>");
            client.println("</div>");




            client.print("<div class=\" col-sm-3\">");
            client.print("<h3 class=\"alert alert-info text-center\" role=\"alert\">TDS</h3>");
            client.print("<h4 class=\" text-center\"  style=\"color:Red\">");
            client.print(tdsread, 0);
            client.print("ppm");
            client.print("</h4>");
            client.println("</div>");




            client.print("<div class=\" col-sm-3\">");
            client.print("<h3 class=\"alert alert-info text-center\" role=\"alert\">PH</h3>");
            client.print("<h4 class=\" text-center\"  style=\"color:Red\">");
            client.print(ph, 2);
            client.print("</h4>");
            client.println("</div>");




            client.print("<div class=\" col-sm-3\">");
            client.print("<h3 class=\"alert alert-info text-center\" role=\"alert\">Moisture</h3>");
            client.print("<h4 class=\" text-center\"  style=\"color:Red\">");
            //client.print(analogValue);
            client.print("</h4>");
            client.println("</div>");

            client.println("<br><br>");
            client.println("</div>");
            client.println("</div>");
            client.println("</div>");


            client.println("</body>");
            client.println("</html>");



            delay(1);
            Serial.println("Client disonnected");
            Serial.println("");

            client.println("</div>");
            client.println("</div>");





            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }



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
