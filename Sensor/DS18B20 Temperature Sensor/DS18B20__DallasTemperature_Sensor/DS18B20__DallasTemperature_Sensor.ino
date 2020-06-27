#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 14

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

void setup()
{
  Serial.begin(9600);
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  int tempread = sensors.getTempCByIndex(0) ;
  //int temp = tempread + 7300;
  Serial.print("temperature: ");
  Serial.print(tempread);
  Serial.println("°C");
  delay(1000);
}
