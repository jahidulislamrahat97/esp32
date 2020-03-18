#include "GravityTDS.h"
#define TdsSensorPin 36
GravityTDS gravityTds;
float temperature = 25, tds = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(TdsSensorPin, INPUT);

  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //initialization

}

void loop() {
  // put your main code here, to run repeatedly:
  //TDS
  //temperature = readTemperature();  //add your temperature sensor and read it
  gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
  gravityTds.update();  //sample and calculate
  tds = gravityTds.getTdsValue();  // then get the value

  Serial.print(tds, 0);
  Serial.println("ppm");
  delay(2500);
}
