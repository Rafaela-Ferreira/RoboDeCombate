#include <Arduino.h>
#include <ESP32Servo.h>

// ====== CONFIGURAÇÕES ======
#define SENSOR_ACTIVE_LOW 1

// Motores
#define enL 17
#define inLA 32
#define inLB 33

#define enR 16
#define inRA 25
#define inRB 26

// PWM
#define PWM_CH_L 0
#define PWM_CH_R 1
#define PWM_FREQ 5000
#define PWM_RES  8

// Velocidades
const int MOTOR_MIN   = 150;   // mínimo pra conseguir mover
int motorSpeed        = 200;   // velocidade normal
int TURN_SPEED        = 180;   // velocidade de giro
int APPROACH_SPEED    = 180;   // velocidade reduzida ao se aproximar da chama
const int STEP_TIME   = 400;   // ms que anda pra frente antes de parar e reavaliar

// Servo
Servo meuServo;
const int pinoServo = 5;
int posAtual = 90;
const int SERVO_CENTER = 90;

// Sensores
const int sensor1 = 23;
const int sensor2 = 22;
const int sensor3 = 21;

// Servo não-bloqueante
unsigned long lastServoStep = 0;
const unsigned long SERVO_STEP_MS = 10;

// ===== Helpers =====
inline bool sensorAtivo(int pin) {
  int v = digitalRead(pin);
  return SENSOR_ACTIVE_LOW ? (v == LOW) : (v == HIGH);
}

int clampSpeed(int v) {
  return (v < MOTOR_MIN) ? MOTOR_MIN : v;
}

void setMotorPWM(uint8_t leftVal, uint8_t rightVal) {
  ledcWrite(PWM_CH_L, clampSpeed(leftVal));
  ledcWrite(PWM_CH_R, clampSpeed(rightVal));
}

void stopMotors() {
  digitalWrite(inLA, LOW);
  digitalWrite(inLB, LOW);
  digitalWrite(inRA, LOW);
  digitalWrite(inRB, LOW);
  setMotorPWM(0, 0);
}

void moveForward(int speed = -1) {
  if (speed < 0) speed = motorSpeed;
  digitalWrite(inLA, HIGH); digitalWrite(inLB, LOW);
  digitalWrite(inRA, HIGH); digitalWrite(inRB, LOW);
  setMotorPWM(speed, speed);
}

void turnLeft(int speed = -1) {
  if (speed < 0) speed = TURN_SPEED;
  digitalWrite(inLA, LOW);  digitalWrite(inLB, HIGH);
  digitalWrite(inRA, HIGH); digitalWrite(inRB, LOW);
  setMotorPWM(speed, speed);
}

void turnRight(int speed = -1) {
  if (speed < 0) speed = TURN_SPEED;
  digitalWrite(inLA, HIGH); digitalWrite(inLB, LOW);
  digitalWrite(inRA, LOW);  digitalWrite(inRB, HIGH);
  setMotorPWM(speed, speed);
}

// Servo não-bloqueante
void atualizarServoPara(int alvo) {
  unsigned long now = millis();
  if (now - lastServoStep < SERVO_STEP_MS) return;
  lastServoStep = now;

  if (abs(alvo - posAtual) > 1) {
    posAtual += (alvo > posAtual) ? 1 : -1;
    meuServo.write(posAtual);
  }
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);

  pinMode(inLA, OUTPUT);
  pinMode(inLB, OUTPUT);
  pinMode(inRA, OUTPUT);
  pinMode(inRB, OUTPUT);
  stopMotors();

  ledcSetup(PWM_CH_L, PWM_FREQ, PWM_RES);
  ledcAttachPin(enL, PWM_CH_L);
  ledcSetup(PWM_CH_R, PWM_FREQ, PWM_RES);
  ledcAttachPin(enR, PWM_CH_R);
  setMotorPWM(0, 0);

  if (SENSOR_ACTIVE_LOW) {
    pinMode(sensor1, INPUT_PULLUP);
    pinMode(sensor2, INPUT_PULLUP);
    pinMode(sensor3, INPUT_PULLUP);
  } else {
    pinMode(sensor1, INPUT);
    pinMode(sensor2, INPUT);
    pinMode(sensor3, INPUT);
  }

  meuServo.attach(pinoServo, 500, 2400);
  posAtual = SERVO_CENTER;
  meuServo.write(posAtual);

  delay(200);
  Serial.println("Setup completo");
}

// ===== Loop =====
void loop() {
  bool s1 = sensorAtivo(sensor1);
  bool s2 = sensorAtivo(sensor2);
  bool s3 = sensorAtivo(sensor3);

  Serial.print("S1: "); Serial.print(s1);
  Serial.print(" | S2: "); Serial.print(s2);
  Serial.print(" | S3: "); Serial.print(s3);

  bool chamaDetectada = (s1 || s2 || s3);

  int soma = 0, contador = 0;
  if (s1) { soma += 0;   contador++; }
  if (s2) { soma += 90;  contador++; }
  if (s3) { soma += 180; contador++; }
  int alvoAngle = (contador > 0) ? (soma / contador) : SERVO_CENTER;

  atualizarServoPara(alvoAngle);

  const int FRONT_MIN = 60;
  const int FRONT_MAX = 120;

  if (chamaDetectada) {
    if (alvoAngle >= FRONT_MIN && alvoAngle <= FRONT_MAX) {
      // frente → avança pouco e reavalia
      moveForward(APPROACH_SPEED);
      Serial.println(" | Ação: AVANÇA POUCO");
      delay(STEP_TIME);
      stopMotors();
    } else if (alvoAngle < FRONT_MIN) {
      turnLeft();
      Serial.println(" | Ação: GIRANDO ESQUERDA");
    } else {
      turnRight();
      Serial.println(" | Ação: GIRANDO DIREITA");
    }
  } else {
    stopMotors();
    Serial.println(" | Ação: PARADO");
  }

  delay(50);
}
