#include <WiFi.h>
#include "time.h"

const char* ssid     = "EduBlock.co";
const char* password = "12345STEMClub";

const char* ntpServer = "asia.pool.ntp.org";
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
/*
  struct tm
  {
  int    tm_sec;   //   Seconds [0,60].
  int    tm_min;   //   Minutes [0,59].
  int    tm_hour;  //   Hour [0,23].
  int    tm_mday;  //   Day of month [1,31].
  int    tm_mon;   //   Month of year [0,11].
  int    tm_year;  //   Years since 1900.
  int    tm_wday;  //   Day of week [0,6] (Sunday =0).
  int    tm_yday;  //   Day of year [0,365].
  int    tm_isdst; //   Daylight Savings flag.
  }
*/

void printLocalTime()
{

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  // Serial.println(&timeinfo, "%A, %d %B %Y %H:%M:%S");
}

void setup()
{
  Serial.begin(115200);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop()
{
  delay(5000);

  printLocalTime();
  second = timeinfo.tm_sec;
  minute = timeinfo.tm_min;
  hour = timeinfo.tm_hour + 4;
  day = timeinfo.tm_mday;
  month = timeinfo.tm_mon + 1;
  year = timeinfo.tm_year + 1900;
  weekday = timeinfo.tm_wday + 1;
  Serial.print("Time from variables:  ");
  Serial.print(day);
  Serial.print(".");
  Serial.print(month);
  Serial.print(".");
  Serial.print(year);
  Serial.print(" --- ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);
}
