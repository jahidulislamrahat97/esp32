int smoke = 25;
// Your threshold value
//int sensorThres = 400;

void setup() {
  pinMode(smoke, INPUT);
  Serial.begin(9600);
}

void loop() {
  int analogSensor = analogRead(smoke);

  Serial.print("Smoke: ");
  Serial.print(analogSensor);
  Serial.println("%");
  delay(500);
}
