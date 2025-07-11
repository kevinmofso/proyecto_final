#define MQ135_PIN A0

float R0 = 10.0; // Se calibrará automáticamente
bool calibrated = false;
unsigned long calibrationTime = 30000; // 30 segundos
unsigned long startTime;

void setup() {
  Serial.begin(9600);
  startTime = millis();
  Serial.println("Calibrando sensor... Mantén en aire limpio");
}

void loop() {
  if (!calibrated) {
    if (millis() - startTime < calibrationTime) {
      // Calibración: Promedio de R0
      int sensorValue = analogRead(MQ135_PIN);
      float voltage = sensorValue * (5.0 / 1023.0);
      float Rs = (5.0 - voltage) / voltage;
      R0 = Rs / 3.6; // Rs/R0 ≈ 3.6 en aire limpio
      Serial.print("Calibrando... R0 estimado: ");
      Serial.println(R0);
      delay(1000);
    } else {
      calibrated = true;
      Serial.print("✅ Calibración completa. R0 final: ");
      Serial.println(R0);
      delay(1000);
    }
    return;
  }

  // Lectura normal después de la calibración
  int sensorValue = analogRead(MQ135_PIN);
  float voltage = sensorValue * (5.0 / 1023.0);
  float Rs = (5.0 - voltage) / voltage;
  float ratio = Rs / R0;

  // Estimar CO2 usando fórmula logarítmica
  float ppmCO2 = pow(10, ((log10(ratio) - 0.4) / -0.42)); // curva estimada CO2

  // Estimar NH3 usando fórmula logarítmica
  float ppmNH3 = pow(10, ((log10(ratio) - 1.3) / -1.5)); // curva estimada NH3

  Serial.print("CO2 estimado: ");
  Serial.print(ppmCO2);
  Serial.print(" ppm | NH3 estimado: ");
  Serial.print(ppmNH3);
  Serial.println(" ppm");

  delay(2000);
}
