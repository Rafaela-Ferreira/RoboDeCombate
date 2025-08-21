como eu adiciono o modulo de camera esp32-cam?

#include <ESP32Servo.h>

Servo meuServo;
int pinoServo = 15;

// Sensores digitais
int sensor1 = 23; // esquerda
int sensor2 = 22; // centro
int sensor3 = 21; // direita

int posAtual = 90; // posição inicial

void setup() {
  Serial.begin(115200);

  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);

  // Define faixa de PWM do servo no ESP32
  meuServo.attach(pinoServo, 500, 2400);

  meuServo.write(posAtual); // começa centralizado
}

void moverServoLento(int alvo) {
  if (posAtual < alvo) posAtual++;
  else if (posAtual > alvo) posAtual--;
  meuServo.write(posAtual);
  delay(5); // controla a velocidade do movimento
}

void loop() {
  int leitura1 = digitalRead(sensor1);
  int leitura2 = digitalRead(sensor2);
  int leitura3 = digitalRead(sensor3);

  // Mostrar status no Serial Monitor
  Serial.print("S1: "); Serial.print(leitura1);
  Serial.print(" | S2: "); Serial.print(leitura2);
  Serial.print(" | S3: "); Serial.println(leitura3);

  // Calcula a média dos ângulos
  int soma = 0;
  int contador = 0;

  if (leitura1 == HIGH) { soma += 0; contador++; }
  if (leitura2 == HIGH) { soma += 90; contador++; }
  if (leitura3 == HIGH) { soma += 180; contador++; }

  int alvo;
  if (contador > 0) {
    alvo = soma / contador; // média das leituras
  } else {
    alvo = 90; // posição central se nada detectado
  }

  // Move servo suavemente até o alvo
  moverServoLento(alvo);
}
