#define Pump 3

int flameSensors[] = {A2, A1, A0}; // [0]=Esquerda, [1]=Centro, [2]=Direita

// === Motor Pins ===
#define enL 10
#define inLA 9
#define inLB 8

#define enR 5
#define inRA 6
#define inRB 7

int motorSpeed = 85;

// Controle da bomba
unsigned long lastFireDetectedTime = 0;
const unsigned long pumpDuration = 2000; // 2 segundos

bool jaAproximouDoFogo = false;

const int fireThreshold = 700;  // Quanto menor o valor, mais forte o fogo. Abaixo disso, considera fogo detectado.

void setup() {
  // Motores
  pinMode(enL, OUTPUT);
  pinMode(inLA, OUTPUT);
  pinMode(inLB, OUTPUT);

  pinMode(enR, OUTPUT);
  pinMode(inRA, OUTPUT);
  pinMode(inRB, OUTPUT);

  analogWrite(enL, motorSpeed);
  analogWrite(enR, motorSpeed);

  // Sensores
  for (int i = 0; i < 3; i++) {
    pinMode(flameSensors[i], INPUT);
  }

  // Bomba
  pinMode(Pump, OUTPUT);
  digitalWrite(Pump, HIGH); // desligada inicialmente

  // Serial para debug
  Serial.begin(9600);
}

void loop() {
  // Leitura analógica dos sensores
  int valLeft = analogRead(flameSensors[0]);
  int valCenter = analogRead(flameSensors[1]);
  int valRight = analogRead(flameSensors[2]);

  Serial.print("Left: "); Serial.print(valLeft);
  Serial.print(" | Center: "); Serial.print(valCenter);
  Serial.print(" | Right: "); Serial.println(valRight);

  bool fireDetected = valLeft < fireThreshold || valCenter < fireThreshold || valRight < fireThreshold;

  if (fireDetected) {
    lastFireDetectedTime = millis();

    // Caso fogo mais intenso esteja no centro
    if (valCenter < valLeft && valCenter < valRight && !jaAproximouDoFogo) {
      moveForward();
      delay(250);
      stopMotors();
      jaAproximouDoFogo = true;
    }
    // Fogo mais intenso à esquerda → desvia para direita
    else if (valLeft < valCenter && valLeft < valRight) {
      moveDiagonalRight();
      delay(250);
      stopMotors();
    }
    // Fogo mais intenso à direita → desvia para esquerda
    else if (valRight < valCenter && valRight < valLeft) {
      moveDiagonalLeft();
      delay(250);
      stopMotors();
    }
    // Fogo muito forte em todos → recua
    else if (valLeft < fireThreshold && valCenter < fireThreshold && valRight < fireThreshold) {
      moveBackward();
      delay(300);
      stopMotors();
    } else {
      stopMotors(); // caso indefinido
    }

  } else {
    jaAproximouDoFogo = false;
    stopMotors();
  }

  // Bomba ativa por 2 segundos após última detecção
  if (millis() - lastFireDetectedTime <= pumpDuration) {
    digitalWrite(Pump, LOW); // ligada
  } else {
    digitalWrite(Pump, HIGH); // desligada
  }

  delay(100);
}

// === Funções de Movimento ===
void moveForward() {
  digitalWrite(inLA, LOW);
  digitalWrite(inLB, HIGH);
  digitalWrite(inRA, LOW);
  digitalWrite(inRB, HIGH);
}

void moveBackward() {
  digitalWrite(inLA, HIGH);
  digitalWrite(inLB, LOW);
  digitalWrite(inRA, HIGH);
  digitalWrite(inRB, LOW);
}

void turnRight() {
  digitalWrite(inLA, HIGH);
  digitalWrite(inLB, LOW);
  digitalWrite(inRA, LOW);
  digitalWrite(inRB, HIGH);
}

void turnLeft() {
  digitalWrite(inLA, LOW);
  digitalWrite(inLB, HIGH);
  digitalWrite(inRA, HIGH);
  digitalWrite(inRB, LOW);
}

void moveDiagonalLeft() {
  analogWrite(enL, 0);
  analogWrite(enR, 100);

  digitalWrite(inLA, LOW);
  digitalWrite(inLB, LOW);
  digitalWrite(inRA, LOW);
  digitalWrite(inRB, HIGH);
}

void moveDiagonalRight() {
  analogWrite(enL, 100);
  analogWrite(enR, 0);

  digitalWrite(inLA, LOW);
  digitalWrite(inLB, HIGH);
  digitalWrite(inRA, LOW);
  digitalWrite(inRB, LOW);
}

void stopMotors() {
  digitalWrite(inLA, LOW);
  digitalWrite(inLB, LOW);
  digitalWrite(inRA, LOW);
  digitalWrite(inRB, LOW);
}
