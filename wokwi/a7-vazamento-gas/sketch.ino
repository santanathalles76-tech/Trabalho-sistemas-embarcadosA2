/*
 * ===========================================================================
 *  Escritório Inteligente | Automação 7: Detecção de vazamento de gás na copa
 * ---------------------------------------------------------------------------
 *  Problema: a copa do escritório tem fogão ou aquecedor a gás. Um vazamento
 *  fora do horário de uso pode passar despercebido e provocar incêndio.
 *
 *  Entradas: sensor de gás MQ-2 (saídas analógica AOUT e digital DOUT) e
 *            botão "Rearmar".
 *  Saídas:   servo da válvula de gás, relé do exaustor (com LED na carga),
 *            LED vermelho e buzzer.
 *
 *  Lógica:
 *    - Nos primeiros 3 s o sistema mede o ar limpo (linha de base).
 *    - Alarme se a leitura analógica passar da linha de base + 150
 *      OU se a saída digital do módulo for a nível baixo (redundância).
 *    - No alarme: fecha a válvula, liga o exaustor, LED pisca e sirene toca.
 *    - O alarme fica travado (latch). Só é rearmado pelo botão e somente
 *      quando o gás já voltou a um nível seguro (linha de base + 75).
 *
 *  Plataforma: Arduino Uno (ATmega328P), simulado no Wokwi.
 * ===========================================================================
 */
#include <Servo.h>

// ----------------------------- Pinos ---------------------------------------
const byte PINO_GAS_ANALOG  = A0;
const byte PINO_GAS_DIGITAL = 4;
const byte PINO_BOTAO       = 3;   // rearmar (INPUT_PULLUP)
const byte PINO_VALVULA     = 6;
const byte PINO_EXAUSTOR    = 7;
const byte PINO_BUZZER      = 8;
const byte PINO_LED_ALARME  = 9;

Servo valvula;

// ----------------------------- Parâmetros ----------------------------------
const unsigned long TEMPO_CALIBRACAO = 3000;  // ms medindo o ar limpo
const int MARGEM_ALARME = 150;                // acima da linha de base
const int MARGEM_SEGURA = 75;                 // nível para permitir o rearme
const int ANGULO_ABERTA = 0, ANGULO_FECHADA = 90;
const unsigned long DEBOUNCE = 50;

// ----------------------------- Estado --------------------------------------
enum Estado { CALIBRANDO, MONITORANDO, ALARME };
Estado estado = CALIBRANDO;
unsigned long inicioCalibracao = 0;
long somaCalibracao = 0;
int amostrasCalibracao = 0;
int linhaDeBase = 0;
unsigned long ultimoLog = 0;
int faseSirene = -1;   // -1 força a primeira troca de tom ao entrar em alarme

int leituraAnterior = HIGH, estadoEstavel = HIGH;
unsigned long ultimaMudanca = 0;

bool botaoApertado() {
  int leitura = digitalRead(PINO_BOTAO);
  bool apertou = false;
  if (leitura != leituraAnterior) ultimaMudanca = millis();
  if (millis() - ultimaMudanca > DEBOUNCE && leitura != estadoEstavel) {
    estadoEstavel = leitura;
    if (estadoEstavel == LOW) apertou = true;
  }
  leituraAnterior = leitura;
  return apertou;
}

void dispararAlarme(int leitura) {
  estado = ALARME;
  faseSirene = -1;
  valvula.write(ANGULO_FECHADA);
  digitalWrite(PINO_EXAUSTOR, HIGH);
  Serial.print(F("[GAS] ALARME! Leitura "));
  Serial.print(leitura);
  Serial.print(F(" (base "));
  Serial.print(linhaDeBase);
  Serial.println(F("). Valvula fechada e exaustor ligado"));
}

void rearmar() {
  estado = MONITORANDO;
  valvula.write(ANGULO_ABERTA);
  digitalWrite(PINO_EXAUSTOR, LOW);
  digitalWrite(PINO_LED_ALARME, LOW);
  noTone(PINO_BUZZER);
  Serial.println(F("[GAS] Sistema rearmado: valvula aberta"));
}

void setup() {
  Serial.begin(9600);
  pinMode(PINO_GAS_DIGITAL, INPUT);
  pinMode(PINO_BOTAO, INPUT_PULLUP);
  pinMode(PINO_EXAUSTOR, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_LED_ALARME, OUTPUT);
  valvula.attach(PINO_VALVULA);
  valvula.write(ANGULO_ABERTA);
  inicioCalibracao = millis();
  Serial.println(F("[GAS] Calibrando com ar limpo..."));
}

void loop() {
  unsigned long agora = millis();
  int leitura = analogRead(PINO_GAS_ANALOG);
  bool digitalAcusou = digitalRead(PINO_GAS_DIGITAL) == LOW;
  bool apertou = botaoApertado();

  switch (estado) {
    case CALIBRANDO:
      somaCalibracao += leitura;
      amostrasCalibracao++;
      digitalWrite(PINO_LED_ALARME, (agora / 250) % 2);  // pisca rápido calibrando
      if (agora - inicioCalibracao >= TEMPO_CALIBRACAO) {
        linhaDeBase = somaCalibracao / amostrasCalibracao;
        estado = MONITORANDO;
        digitalWrite(PINO_LED_ALARME, LOW);
        Serial.print(F("[GAS] Linha de base: "));
        Serial.println(linhaDeBase);
      }
      break;

    case MONITORANDO:
      if (leitura > linhaDeBase + MARGEM_ALARME || digitalAcusou) dispararAlarme(leitura);
      break;

    case ALARME: {
      // Sirene de dois tons e LED piscando, alternando a cada 300 ms
      int fase = (agora / 300) % 2;
      if (fase != faseSirene) {          // troca o tom só na virada da fase
        faseSirene = fase;
        digitalWrite(PINO_LED_ALARME, fase ? HIGH : LOW);
        tone(PINO_BUZZER, fase ? 2000 : 1400);
      }
      if (apertou) {
        if (leitura <= linhaDeBase + MARGEM_SEGURA && !digitalAcusou) rearmar();
        else Serial.println(F("[GAS] Rearme recusado: gas ainda acima do nivel seguro"));
      }
      break;
    }
  }

  if (estado != CALIBRANDO && agora - ultimoLog >= 2000) {  // registro periódico
    ultimoLog = agora;
    Serial.print(F("[GAS] Leitura: "));
    Serial.println(leitura);
  }
}
