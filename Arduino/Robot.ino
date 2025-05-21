#include <Servo.h>

// === Motor Pins ===
#define enL 10
#define inLA 9
#define inLB 8

#define enR 5
#define inRA 6
#define inRB 7

int motorSpeed = 200;

// === Flame Sensor and Pump ===
#define pumpPin 3
int flameSensors[] = {A0, A1, A2};

// === Servo ===
Servo myServo;
int servoPin = 11;

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

  // Sensor de chama e bomba
  for (int i = 0; i < 3; i++) {
    pinMode(flameSensors[i], INPUT);
  }
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);

  // Servo
  myServo.attach(servoPin);
}

void loop() {
  // === Sensores de chama ===
  bool fireDetected = false;
  for (int i = 0; i < 3; i++) {
    if (digitalRead(flameSensors[i]) == LOW) {
      fireDetected = true;
      break;
    }
  }
  digitalWrite(pumpPin, fireDetected ? LOW : HIGH);

  // === Controle dos motores ===
  moveForward();
  delay(2000);
  stopMotors();
  delay(1000);

  moveBackward();
  delay(2000);
  stopMotors();
  delay(1000);

  turnRight();
  delay(1500);
  stopMotors();
  delay(1000);

  turnLeft();
  delay(1500);
  stopMotors();
  delay(2000);

  // === Movimento do servo ===
  myServo.write(0);
  delay(1000);
  myServo.write(180);
  delay(1000);
  myServo.write(90);
  delay(1000);
}

// === Funções de controle dos motores ===
void moveForward() {
  digitalWrite(inLA, HIGH);
  digitalWrite(inLB, LOW);
  digitalWrite(inRA, HIGH);
  digitalWrite(inRB, LOW);
}

void moveBackward() {
  digitalWrite(inLA, LOW);
  digitalWrite(inLB, HIGH);
  digitalWrite(inRA, LOW);
  digitalWrite(inRB, HIGH);
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

void stopMotors() {
  digitalWrite(inLA, LOW);
  digitalWrite(inLB, LOW);
  digitalWrite(inRA, LOW);
  digitalWrite(inRB, LOW);
}
