const int uvPin = 25;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 0 - 4095
}

void loop() {
  int raw = analogRead(uvPin);
  float voltage = raw * (3.3 / 4095.0);
  float uvIndex = (voltage - 1.0) * (10.0 / 3.0 - 1.0);
  uvIndex = max(uvIndex, 0.0);

  Serial.print("ADC: ");
  Serial.print(raw);
  Serial.print(" | Volt: ");
  Serial.print(voltage, 2);
  Serial.print(" V | UV Index: ");
  Serial.println(uvIndex, 1);

  delay(1000);
}
