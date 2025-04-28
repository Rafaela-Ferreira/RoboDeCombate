// Definições dos pinos do driver L298 para controle dos motores
#define habilitaMotor1 10  // Habilita o Motor 1
#define motor1Frente 9     // Motor 1 - sentido frente
#define motor1Tras 8       // Motor 1 - sentido trás
#define motor2Tras 7       // Motor 2 - sentido trás
#define motor2Frente 6     // Motor 2 - sentido frente
#define habilitaMotor2 5   // Habilita o Motor 2

// Sensores e atuadores
#define sensorDir A0       // Sensor infravermelho direito
#define sensorFrente A1    // Sensor infravermelho frontal
#define sensorEsq A2       // Sensor infravermelho esquerdo
#define pinoServo A4       // Servo motor
#define bombaAgua A5       // Bomba de água

int velocidade = 160; // Velocidade dos motores (0 a 255)
int leituraSensorDir, leituraSensorFrente, leituraSensorEsq; // Leituras dos sensores

void setup() {
  Serial.begin(9600); 

  // Pinos de entrada
  pinMode(sensorDir, INPUT);
  pinMode(sensorFrente, INPUT);
  pinMode(sensorEsq, INPUT);

  // Pinos de saída
  pinMode(habilitaMotor1, OUTPUT);
  pinMode(motor1Frente, OUTPUT);
  pinMode(motor1Tras, OUTPUT);
  pinMode(motor2Tras, OUTPUT);
  pinMode(motor2Frente, OUTPUT);
  pinMode(habilitaMotor2, OUTPUT);

  // Pinos do servo e da bomba
  pinMode(pinoServo, OUTPUT);
  pinMode(bombaAgua, OUTPUT);

  // Movimento inicial do servo (varredura)
  for (int angulo = 90; angulo <= 140; angulo += 5) {
    servoPulso(pinoServo, angulo);
  }
  for (int angulo = 140; angulo >= 40; angulo -= 5) {
    servoPulso(pinoServo, angulo);
  }
  for (int angulo = 40; angulo <= 95; angulo += 5) {
    servoPulso(pinoServo, angulo);
  }

  // Define a velocidade dos motores
  analogWrite(habilitaMotor1, velocidade);
  analogWrite(habilitaMotor2, velocidade);
  delay(500);
}

void loop() {
  // Leitura dos sensores
  leituraSensorDir = analogRead(sensorDir);
  leituraSensorFrente = analogRead(sensorFrente);
  leituraSensorEsq = analogRead(sensorEsq);

  // Impressão dos valores no monitor serial
  Serial.print(leituraSensorDir);
  Serial.print("\t");
  Serial.print(leituraSensorFrente);
  Serial.print("\t");
  Serial.println(leituraSensorEsq);
  delay(50);

  // Lógica de combate ao fogo
  if (leituraSensorDir < 250) {
    Parar();
    digitalWrite(bombaAgua, HIGH);

    for (int angulo = 90; angulo >= 40; angulo -= 3) {
      servoPulso(pinoServo, angulo);
    }
    for (int angulo = 40; angulo <= 90; angulo += 3) {
      servoPulso(pinoServo, angulo);
    }

  } else if (leituraSensorFrente < 350) {
    Parar();
    digitalWrite(bombaAgua, HIGH);

    for (int angulo = 90; angulo <= 140; angulo += 3) {
      servoPulso(pinoServo, angulo);
    }
    for (int angulo = 140; angulo >= 40; angulo -= 3) {
      servoPulso(pinoServo, angulo);
    }
    for (int angulo = 40; angulo <= 90; angulo += 3) {
      servoPulso(pinoServo, angulo);
    }

  } else if (leituraSensorEsq < 250) {
    Parar();
    digitalWrite(bombaAgua, HIGH);
    for (int angulo = 90; angulo <= 140; angulo += 3) {
      servoPulso(pinoServo, angulo);
    }
    for (int angulo = 140; angulo >= 90; angulo -= 3) {
      servoPulso(pinoServo, angulo);
    }

  } else if (leituraSensorDir >= 251 && leituraSensorDir <= 700) {
    digitalWrite(bombaAgua, LOW);
    Recuar();
    delay(100);
    VirarDireita();
    delay(200);

  } else if (leituraSensorFrente >= 251 && leituraSensorFrente <= 800) {
    digitalWrite(bombaAgua, LOW);
    Avancar();

  } else if (leituraSensorEsq >= 251 && leituraSensorEsq <= 700) {
    digitalWrite(bombaAgua, LOW);
    Recuar();
    delay(100);
    VirarEsquerda();
    delay(200);

  } else {
    digitalWrite(bombaAgua, LOW);
    Parar();
  }

  delay(10);
}

// Envia pulso ao servo motor para ajustar o ângulo
void servoPulso(int pino, int angulo) {
  int pwm = (angulo * 11) + 500; // Converte ângulo em microssegundos
  digitalWrite(pino, HIGH);
  delayMicroseconds(pwm);
  digitalWrite(pino, LOW);
  delay(50); 
}

void Avancar() {
  digitalWrite(motor1Frente, HIGH);
  digitalWrite(motor1Tras, LOW);
  digitalWrite(motor2Tras, LOW);
  digitalWrite(motor2Frente, HIGH);
}

void Recuar() {
  digitalWrite(motor1Frente, LOW);
  digitalWrite(motor1Tras, HIGH);
  digitalWrite(motor2Tras, HIGH);
  digitalWrite(motor2Frente, LOW);
}

void VirarDireita() {
  digitalWrite(motor1Frente, LOW);
  digitalWrite(motor1Tras, HIGH);
  digitalWrite(motor2Tras, LOW);
  digitalWrite(motor2Frente, HIGH);
}

void VirarEsquerda() {
  digitalWrite(motor1Frente, HIGH);
  digitalWrite(motor1Tras, LOW);
  digitalWrite(motor2Tras, HIGH);
  digitalWrite(motor2Frente, LOW);
}

void Parar() {
  digitalWrite(motor1Frente, LOW);
  digitalWrite(motor1Tras, LOW);
  digitalWrite(motor2Tras, LOW);
  digitalWrite(motor2Frente, LOW);
}
