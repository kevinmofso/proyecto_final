#include <DHTesp.h>
#include "LoRaWan_APP.h"
#include "Arduino.h"

// MQ-135
#define MQ135_PIN 36
#define s12_pin 25
#define RL_VALUE 10.0
#define CO2_TARGET 400
float R0 = 10.0;


// DHT22 Configuration
#define DHT_PIN 4
DHTesp dht;



// LoRa Configuration
#define RF_FREQUENCY                915000000   // Hz
#define TX_OUTPUT_POWER             14          // dBm
#define LORA_BANDWIDTH              0           // 125 kHz
#define LORA_SPREADING_FACTOR       7           // SF7
#define LORA_CODINGRATE             1           // 4/5
#define LORA_PREAMBLE_LENGTH        8
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON        false

#define BUFFER_SIZE 100
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

bool lora_idle = true;
unsigned long lastSendTime = 0;
const unsigned long interval = 10000;  // 10 segundos

unsigned long messageCount = 0;

// LoRa event callbacks
static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

void setup() {
  Serial.begin(115200);
pinMode(s12_pin,INPUT);   
  // Init DHT22
  dht.setup(DHT_PIN, DHTesp::DHT22);
  Serial.println("DHT22 Initialized");
  calibrarR0();
  // Init LoRa
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
  Radio.Rx(0);  // Start listening
}

void loop() {
  Radio.IrqProcess();  // Handle radio events

  unsigned long currentTime = millis();

  if (lora_idle && (currentTime - lastSendTime >= interval)) {
    lastSendTime = currentTime;
    messageCount++;
//mq 135
  int analogValue = analogRead(MQ135_PIN);
  float rs = getRS(analogValue);
  float rs_ro_ratio = rs / R0;

  float co2 = getCO2ppm(rs_ro_ratio);
  float nh3 = getNH3ppm(co2);
int sensorValue = analogRead(s12_pin);  // lectura cruda
float sensorVoltage = sensorValue * (3.3 / 4095.0);  // conversión a voltaje el ADCdele sp32 tiene12 tine bits
float uv_index = sensorVoltage * 10.0;  // índice UV
  //dht22
    TempAndHumidity data = dht.getTempAndHumidity();
    if (dht.getStatus() != DHTesp::ERROR_NONE) {
      Serial.println("DHT22 read failed: " + String(dht.getStatusString()));
      return;
    }

snprintf(txpacket, BUFFER_SIZE,
         "{\"id\":%lu,\"t\":%.1f,\"h\":%.1f,\"co2\":%.1f,\"nh3\":%.1f,\"uv\":%.2f}",
         messageCount, data.temperature, data.humidity, co2, nh3, uv_index);

    Serial.printf("Sending (%lu): %s\n", messageCount, txpacket);
    Radio.Send((uint8_t *)txpacket, strlen(txpacket));
    lora_idle = false;
  }
}

// LoRa TX successful
void OnTxDone(void) {
  Serial.println("LoRa TX done");
  lora_idle = true;
  Radio.Rx(0);  // Resume listening
}

// LoRa TX timeout
void OnTxTimeout(void) {
  Serial.println("LoRa TX timeout");
  Radio.Sleep();
  lora_idle = true;
  Radio.Rx(0);
}

// LoRa RX callback
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  memcpy(rxpacket, payload, size);
  rxpacket[size] = '\0';  // Null-terminate

  Serial.printf("Received: %s | RSSI: %d, SNR: %d\n", rxpacket, rssi, snr);

  if (String(rxpacket) == "chau") {
    Serial.println("Received 'chau', responding...");
    snprintf(txpacket, BUFFER_SIZE, "{\"reply\":\"chau recibido\", \"id\":%lu}", messageCount);
    Radio.Send((uint8_t *)txpacket, strlen(txpacket));
    lora_idle = false;
  } else {
    Radio.Rx(0);  // Keep listening
  }
}

float getRS(int analogValue) {
  float voltage = analogValue * (3.3 / 4095.0);
  return ((3.3 - voltage) * RL_VALUE) / voltage;
}

float calcularR0(float rs, float co2_target) {
  float log_rs_ro = (2.602 - log10(co2_target)) / 2.769;
  float rs_ro = pow(10, log_rs_ro);
  return rs / rs_ro;
}

float getCO2ppm(float rs_ro_ratio) {
  return pow(10, (-2.769 * log10(rs_ro_ratio) + 2.602));
}

float getNH3ppm(float co2) {
  float log_rs_ro = (2.602 - log10(co2)) / 2.769;
  return (pow(10, (-1.8 * log_rs_ro + 1.5))*0.02);
} 

void calibrarR0() {
  const int muestras = 100;
  float rs_total = 0;


  for (int i = 0; i < muestras; i++) {
    int analogValue = analogRead(MQ135_PIN);
    rs_total += getRS(analogValue);
    delay(30);
  }

  float rs_avg = rs_total / muestras;
  R0 = calcularR0(rs_avg, CO2_TARGET);

  Serial.println("=== Calibracion MQ-135 ===");
  Serial.print("RS promedio: "); Serial.println(rs_avg, 2);
  Serial.print("R0 calibrado: "); Serial.println(R0, 2);

}
