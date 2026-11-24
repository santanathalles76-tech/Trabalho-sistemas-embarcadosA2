/*
 * ===========================================================================
 *  Escritório Inteligente | Automação 3: Ar-condicionado por ocupação e
 *  temperatura
 * ---------------------------------------------------------------------------
 *  Problema: o ar-condicionado das salas fica ligado com a sala vazia, ou liga
 *  e desliga sem parar perto da temperatura desejada, o que gasta energia e
 *  desgasta o compressor.
 *
 *  Entradas: termistor NTC (temperatura), potenciômetro (temperatura
 *            desejada, de 18 a 28 °C) e sensor de presença PIR.
 *  Saídas:   relé do compressor (com LED azul na carga) e LCD 16x2 I2C.
 *
 *  Lógica:
 *    - A sala é "ocupada" se houve movimento nos últimos 20 s
 *      (na prática, 15 min).
 *    - Liga se ocupada E temperatura >= desejada + 1,0 °C.
 *    - Desliga se temperatura <= desejada - 1,0 °C OU a sala ficou vazia.
 *    - A faixa de 2 °C entre ligar e desligar é a histerese, que evita o
 *      liga e desliga contínuo.
 *    - Entre duas mudanças do relé há um intervalo mínimo de 5 s
 *      (na prática, 3 min) para proteger o compressor.
 *
 *  Plataforma: Arduino Uno (ATmega328P), simulado no Wokwi.
 * ===========================================================================
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

// ----------------------------- Pinos ---------------------------------------
const byte PINO_NTC   = A0;
const byte PINO_POT   = A1;
const byte PINO_PIR   = 2;
const byte PINO_RELE  = 7;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----------------------------- Parâmetros ----------------------------------
const float BETA_NTC = 3950;                       // coeficiente do termistor do Wokwi
const float HISTERESE = 1.0;                       // °C acima e abaixo da desejada
const float DESEJADA_MIN = 18.0, DESEJADA_MAX = 28.0;
const unsigned long TEMPO_OCUPACAO = 20000;        // ms de validade do último movimento
const unsigned long INTERVALO_MINIMO_RELE = 5000;  // ms de proteção do compressor
const unsigned long INTERVALO_TELA = 500;          // ms entre atualizações do LCD

// ----------------------------- Estado --------------------------------------
bool compressorLigado = false;
unsigned long ultimaMudancaRele = 0;
unsigned long ultimoMovimento = 0;
bool houveMovimento = false;          // evita "ocupada" logo ao ligar a placa
unsigned long ultimaTela = 0;

// Converte a leitura analógica do NTC em graus Celsius (equação Beta)
float lerTemperatura() {
  int leitura = analogRead(PINO_NTC);
  leitura = constrain(leitura, 1, 1022);  // evita divisão por zero nos extremos
  return 1.0 / (log(1.0 / (1023.0 / leitura - 1.0)) / BETA_NTC + 1.0 / 298.15) - 273.15;
}

// Converte a posição do potenciômetro na temperatura desejada (passo de 0,5 °C)
float lerTemperaturaDesejada() {
  int leitura = analogRead(PINO_POT);
  float valor = DESEJADA_MIN + (DESEJADA_MAX - DESEJADA_MIN) * leitura / 1023.0;
  return round(valor * 2.0) / 2.0;
}

void acionarCompressor(bool ligar) {
  if (ligar == compressorLigado) return;
  compressorLigado = ligar;
  ultimaMudancaRele = millis();
  digitalWrite(PINO_RELE, ligar ? HIGH : LOW);
  Serial.println(ligar ? F("[AR] Compressor LIGADO") : F("[AR] Compressor DESLIGADO"));
}

void atualizarTela(float temperatura, float desejada, bool ocupada) {
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperatura, 1);
  lcd.print(" Alvo:");
  lcd.print(desejada, 1);
  lcd.print("   ");
  lcd.setCursor(0, 1);
  lcd.print(ocupada ? "Ocupada " : "Vazia   ");
  lcd.print(compressorLigado ? "AR LIG  " : "AR DESL ");
}

void setup() {
  Serial.begin(9600);
  pinMode(PINO_PIR, INPUT);
  pinMode(PINO_RELE, OUTPUT);
  digitalWrite(PINO_RELE, LOW);
  lcd.init();
  lcd.backlight();
  // Permite o primeiro acionamento sem esperar o intervalo de proteção
  ultimaMudancaRele = millis() - INTERVALO_MINIMO_RELE;
}

void loop() {
  unsigned long agora = millis();

  if (digitalRead(PINO_PIR) == HIGH) {
    ultimoMovimento = agora;
    houveMovimento = true;
  }
  bool ocupada = houveMovimento && (agora - ultimoMovimento < TEMPO_OCUPACAO);

  float temperatura = lerTemperatura();
  float desejada = lerTemperaturaDesejada();

  // Decisão com histerese
  bool deveLigar = compressorLigado;
  if (!ocupada) deveLigar = false;
  else if (temperatura >= desejada + HISTERESE) deveLigar = true;
  else if (temperatura <= desejada - HISTERESE) deveLigar = false;

  // Proteção do compressor: só muda o relé depois do intervalo mínimo
  if (deveLigar != compressorLigado && agora - ultimaMudancaRele >= INTERVALO_MINIMO_RELE) {
    acionarCompressor(deveLigar);
  }

  if (agora - ultimaTela >= INTERVALO_TELA) {
    ultimaTela = agora;
    atualizarTela(temperatura, desejada, ocupada);
  }
}
