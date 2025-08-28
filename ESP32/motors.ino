// === Pinos do Motor (adaptados para ESP32) ===
#define enL 4    // PWM canal esquerdo
#define inLA 16
#define inLB 17

#define enR 5    // PWM canal direito
#define inRA 18
#define inRB 19

int motorSpeed = 200; // valor de 0 a 255

// Canais e resolução do PWM
#define PWM_FREQ     1000
#define PWM_CHANNEL_L  0
#define PWM_CHANNEL_R  1
#define PWM_RESOLUTION 8  // 8 bits (0-255)

void setup() {
  Serial.begin(115200);

  // Configura pinos de direção
  pinMode(inLA, OUTPUT);
  pinMode(inLB, OUTPUT);
  pinMode(inRA, OUTPUT);
  pinMode(inRB, OUTPUT);

  // Configura PWM (ledc) para os pinos de enable
  ledcSetup(PWM_CHANNEL_L, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(enL, PWM_CHANNEL_L);

  ledcSetup(PWM_CHANNEL_R, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(enR, PWM_CHANNEL_R);

  // Define velocidade inicial
  ledcWrite(PWM_CHANNEL_L, motorSpeed);
  ledcWrite(PWM_CHANNEL_R, motorSpeed);
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
