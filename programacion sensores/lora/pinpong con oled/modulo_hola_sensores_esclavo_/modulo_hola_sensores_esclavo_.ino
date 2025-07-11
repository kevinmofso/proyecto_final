////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define RF_FREQUENCY 915000000
#define TX_OUTPUT_POWER 5
#define LORA_BANDWIDTH 0
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BUFFER_SIZE 64

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
  ESTADO_BAJO_CONSUMO,
  ESTADO_RECIBIR,
  ESTADO_TRANSMITIR
} States_t;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int16_t txNumber = 0;
int16_t rxNumber = 0;
States_t state;
int16_t Rssi, rxSize;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t ultimo_envio = 0;
const uint32_t intervalo = 10000; // 10 segundos
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
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

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  state = ESTADO_RECIBIR;

  VextON();
  delay(100);
  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Iniciado");
  display.display();
  display.setContrast(255);

  ultimo_envio = millis(); 
}

void loop() {
  //  si pasaron 10 segundos, transmitir "hola"
  if ((millis() - ultimo_envio) >= intervalo) {
    strcpy(txpacket, "hola");


    display.clear();
    display.drawString(0, 0, "Modo: auto tx");
    display.drawString(0, 16, txpacket);
    display.display();
delay(2000);
    Radio.Send((uint8_t *)txpacket, strlen(txpacket));
    ultimo_envio = millis();
    state = ESTADO_BAJO_CONSUMO;
  }

  switch (state) {
    case ESTADO_TRANSMITIR:
      // Esto se usa si hay una transmisión manual, por ejemplo por recibir "chau"

      txNumber++;
sprintf(txpacket, "respuesta %d", txNumber);  // Crea el mensaje "hello <número>"

      display.clear();
    
      display.drawString(0, 0, "Respondiendo a CHAU");
      display.drawString(0, 16, txpacket);
      display.display();
delay(2000);
      Radio.Send((uint8_t *)txpacket, strlen(txpacket));
      state = ESTADO_BAJO_CONSUMO;
      break;

    case ESTADO_RECIBIR:
      display.clear();
      display.drawString(0, 0, "escuchando...");
      display.display();

      Radio.Rx(0);
      state = ESTADO_BAJO_CONSUMO;
      break;

    case ESTADO_BAJO_CONSUMO:
      Radio.IrqProcess();//espera a que pase algo, si pasa algo lo mandamos a el OnRxDone
      break;

    default:
      break;
  }
}

void OnTxDone(void) {
  //cada vez que transmite algo entra aca
  state = ESTADO_RECIBIR;
}

void OnTxTimeout(void) {
  Radio.Sleep();
  Serial.println("TX Timeout");

  display.clear();
  display.drawString(0, 0, "TX Timeout");
  display.display();

  state = ESTADO_TRANSMITIR;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
Rssi = rssi;
rxNumber++;
Radio.Sleep();
String mensaje = String((char*)payload);//paso a string
display.clear();
display.drawString(0, 0, "modo: recepcion");
display.drawString(0, 16, String("mensaje recibido #") + rxNumber);
display.drawString(0, 32, mensaje);
display.display();
delay(2000);
if (mensaje == "chau") {
  state = ESTADO_TRANSMITIR;
} else {
  state = ESTADO_RECIBIR;
}
}

void VextON(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}
