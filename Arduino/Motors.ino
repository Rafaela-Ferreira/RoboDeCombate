// === Motor Pins ===
#define enL 10
#define inLA 9
#define inLB 8

#define enR 5
#define inRA 6
#define inRB 7

int motorSpeed = 200;

void setup() {
  pinMode(enL, OUTPUT);
  pinMode(inLA, OUTPUT);
  pinMode(inLB, OUTPUT);

  pinMode(enR, OUTPUT);
  pinMode(inRA, OUTPUT);
  pinMode(inRB, OUTPUT);

  analogWrite(enL, motorSpeed);
  analogWrite(enR, motorSpeed);
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
  digitalWrite(inLeftA, HIGH);
  digitalWrite(inLB, LOW);
  digitalWrite(inRightA, HIGH);
  digitalWrite(inRB, LOW);
}

void moveBackward() {
  digitalWrite(inLeftA, LOW);
  digitalWrite(inLB, HIGH);
  digitalWrite(inRightA, LOW);
  digitalWrite(inRB, HIGH);
}

void turnRight() {
  digitalWrite(inLeftA, HIGH);
  digitalWrite(inLB, LOW);
  digitalWrite(inRightA, LOW);
  digitalWrite(inRB, HIGH);
}

void turnLeft() {
  digitalWrite(inLeftA, LOW);
  digitalWrite(inLB, HIGH);
  digitalWrite(inRightA, HIGH);
  digitalWrite(inRB, LOW);
}

void stopMotors() {
  digitalWrite(inLeftA, LOW);
  digitalWrite(inLB, LOW);
  digitalWrite(inRightA, LOW);
  digitalWrite(inRB, LOW);
}
