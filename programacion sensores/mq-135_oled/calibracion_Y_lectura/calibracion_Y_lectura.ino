#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include <math.h>

#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
#endif

#define MQ135_PIN 36
#define RL_VALUE 10.0    // kOhm
#define CO2_TARGET 400   // Valor deseado en aire limpio

float R0 = 10.0;         // Se calibrará automáticamente

void VextON() {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

float getRS(int analogValue) {
  float voltage = analogValue * (3.3 / 4095.0);
  float rs = ((3.3 - voltage) * RL_VALUE) / voltage;
  return rs;
}

float calcularR0(float rs, float co2_target) {
  float log_rs_r0 = (2.602 - log10(co2_target)) / 2.769;
  float rs_r0 = pow(10, log_rs_r0);
  return rs / rs_r0;
}

float getCO2ppm(float rs_ro_ratio) {
  return pow(10, (-2.769 * log10(rs_ro_ratio) + 2.602));
}

float getNH3ppm(float rs_ro_ratio) {
  return pow(10, (-1.8 * log10(rs_ro_ratio) + 1.5));
}

void calibrarR0() {
  const int samples = 100;
  float rs_total = 0;

  display.clear();
  display.drawString(0, 0, "Calibrando R0...");
  display.display();

  for (int i = 0; i < samples; i++) {
    int analogValue = analogRead(MQ135_PIN);
    float rs = getRS(analogValue);
    rs_total += rs;
    delay(30);
  }

  float rs_avg = rs_total / samples;
  R0 = calcularR0(rs_avg, CO2_TARGET);

  Serial.println("=== Calibración MQ-135 ===");
  Serial.print("RS promedio: ");
  Serial.println(rs_avg, 2);
  Serial.print("CO2 objetivo: ");
  Serial.println(CO2_TARGET);
  Serial.print("R0 calibrado: ");
  Serial.println(R0, 2);
  Serial.println("==========================");

  display.clear();
  display.drawString(0, 0, "RS = " + String(rs_avg, 2));
  display.drawString(0, 16, "CO2 objetivo = " + String(CO2_TARGET) + " ppm");
  display.drawString(0, 32, "R0 = " + String(R0, 2));
  display.display();
  delay(3000);
}

void setup() {
  VextON();
  delay(100);

  Serial.begin(115200);
  analogReadResolution(12);

  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  calibrarR0();  // Se ejecuta solo una vez al iniciar
}

void loop() {
  int analogValue = analogRead(MQ135_PIN);
  float rs = getRS(analogValue);
  float rs_ro_ratio = rs / R0;

  float co2_ppm = getCO2ppm(rs_ro_ratio);
  float nh3_ppm = getNH3ppm(rs_ro_ratio);

  Serial.print("RS: "); Serial.print(rs, 2);
  Serial.print(" | CO2: "); Serial.print(co2_ppm, 0); Serial.print(" ppm");
  Serial.print(" | NH3: "); Serial.print(nh3_ppm, 0); Serial.println(" ppm");

  display.clear();
  display.drawString(0, 0, "CO2: " + String(co2_ppm, 0) + " ppm");
  display.drawString(0, 16, "NH3: " + String(nh3_ppm, 0) + " ppm");
  display.display();

  delay(2000);
}
_