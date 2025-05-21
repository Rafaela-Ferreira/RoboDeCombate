#define Pump 3

int flameSensors[] = {A0, A1, A2};

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(flameSensors[i], INPUT);
  }
  pinMode(Pump, OUTPUT);
  digitalWrite(Pump, LOW);
}

void loop() {
  bool fireDetected = false;

  for (int i = 0; i < 3; i++) {
    if (digitalRead(flameSensors[i]) == LOW) {
      fireDetected = true;
      break;
    }
  }

  digitalWrite(Pump, fireDetected ? LOW : HIGH);

  delay(100);
}
