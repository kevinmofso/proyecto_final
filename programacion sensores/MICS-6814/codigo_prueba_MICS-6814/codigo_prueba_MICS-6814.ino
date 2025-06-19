// Pines donde conectaste cada salida analógica del sensor
const int pinNH3 = 2;  // Amoniaco
const int pinCO  = 0;  // Monóxido de carbono
const int pinNO2 = 13;  // Dióxido de nitrógeno

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // Resolución del ADC de ESP32 (0-4095)
}

void loop() {
  int valorNH3 = analogRead(pinNH3);
  int valorCO  = analogRead(pinCO);
  int valorNO2 = analogRead(pinNO2);

  // Mostrar valores en el monitor serie
  Serial.println("Lecturas del MICS-6814:");
  Serial.print("NH3 (Amoniaco): ");
  Serial.println(valorNH3);
  Serial.print("CO  (Monóxido de carbono): ");
  Serial.println(valorCO);
  Serial.print("NO2 (Dióxido de nitrógeno): ");
  Serial.println(valorNO2);
  Serial.println("-----------------------------");

  delay(2000); // Esperar 2 segundos antes de la próxima lectura
}
