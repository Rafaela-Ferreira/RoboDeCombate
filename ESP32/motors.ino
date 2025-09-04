// === Motor Pins ===
#define enL 17
#define inLA 32
#define inLB 33

#define enR 16
#define inRA 25
#define inRB 26

int motorSpeed = 200; // 0–255
#define PWM_CH_L 0
#define PWM_CH_R 1

void setup() {
  pinMode(inLA, OUTPUT);
  pinMode(inLB, OUTPUT);
  pinMode(inRA, OUTPUT);
  pinMode(inRB, OUTPUT);

  // Configura PWM (canal, freq, resolução)
  ledcSetup(PWM_CH_L, 5000, 8);
  ledcAttachPin(enL, PWM_CH_L);

  ledcSetup(PWM_CH_R, 5000, 8);
  ledcAttachPin(enR, PWM_CH_R);

  // Define velocidade inicial
  ledcWrite(PWM_CH_L, motorSpeed);
  ledcWrite(PWM_CH_R, motorSpeed);
}

void loop() {
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
}

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
