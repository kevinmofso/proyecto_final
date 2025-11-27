#include <DHT.h>

#define DHTPIN 2      // Pin digital donde está conectado el DHT11
#define DHTTYPE DHT11 // Definimos que usamos el DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  // Esperar 2 segundos entre lecturas (recomendado por el sensor)
  delay(2000);

  float humedad = dht.readHumidity();
  float temperatura = dht.readTemperature(); // Celsius por defecto

  // Verificar si alguna lectura falló y salir del loop si es así
  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error al leer del sensor DHT11");
    return;
  }

  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");
}
