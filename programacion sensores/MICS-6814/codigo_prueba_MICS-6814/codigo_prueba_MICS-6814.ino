// Pines de las bobinas (ajustá según tu conexión)
#define coilA1 2
#define coilA2 3
#define coilB1 4
#define coilB2 5

// Encoder incremental
#define ENCODER_PIN 6

volatile long encoderCount = 0;
long lastStep = 0;

void setup() {
  pinMode(coilA1, OUTPUT);
  pinMode(coilA2, OUTPUT);
  pinMode(coilB1, OUTPUT);
  pinMode(coilB2, OUTPUT);
  
  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), encoderISR, RISING);
  
  Serial.begin(9600);
}

void loop() {
  noInterrupts();
  long currentStep = encoderCount;
  interrupts();

  if (currentStep != lastStep) {
    // Actualizar bobinas
    setCoils(currentStep);
    lastStep = currentStep;
  }
  
  delay(10);
}

void setCoils(long step) {
  // Para simplificar: hay 4 estados de bobinas en secuencia
  
  // El step modulo 4 determina qué bobina activar
  int current = step % 4;
  if (current < 0) current += 4;

  // La bobina anterior es (step - 1) % 4
  int previous = (step - 1) % 4;
  if (previous < 0) previous += 4;

  // Apagar todas las bobinas primero
  digitalWrite(coilA1, LOW);
  digitalWrite(coilA2, LOW);
  digitalWrite(coilB1, LOW);
  digitalWrite(coilB2, LOW);

  // Encender bobina actual y bobina anterior para torque constante
  turnOnCoil(current);
  turnOnCoil(previous);
}

void turnOnCoil(int coil) {
  switch(coil) {
    case 0:
      digitalWrite(coilA1, HIGH);
      break;
    case 1:
      digitalWrite(coilB1, HIGH);
      break;
    case 2:
      digitalWrite(coilA2, HIGH);
      break;
    case 3:
      digitalWrite(coilB2, HIGH);
      break;
  }
}

void encoderISR() {
  encoderCount++;
}
