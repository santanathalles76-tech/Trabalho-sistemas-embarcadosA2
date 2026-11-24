/*
 * ===========================================================================
 *  Escritório Inteligente | Automação 8: Nível do reservatório superior e
 *  bomba de recalque
 * ---------------------------------------------------------------------------
 *  Problema: a caixa d'água que abastece banheiros e copa esvazia sem aviso,
 *  ou a bomba fica ligada com a caixa cheia (transbordando) ou trabalhando a
 *  seco quando o sensor falha.
 *
 *  Entrada: sensor ultrassônico HC-SR04 instalado na tampa do reservatório.
 *  Saídas:  relé da bomba de recalque (com LED na carga), barra de 4 LEDs de
 *           nível, buzzer e LCD 16x2 I2C.
 *
 *  Lógica:
 *    - Mede a distância até a água (mediana de 3 leituras) e converte em %:
 *      20 cm da tampa = 100 % e 220 cm = 0 %.
 *    - Bomba liga abaixo de 25 % e só desliga ao chegar a 90 % (histerese,
 *      que evita a bomba ligando e desligando a todo momento).
 *    - Abaixo de 10 %: alarme de reserva crítica (bipe a cada 2 s).
 *    - Se o sensor não responder, a bomba é desligada por segurança e o
 *      visor mostra a falha.
 *
 *  Plataforma: Arduino Uno (ATmega328P), simulado no Wokwi.
 * ===========================================================================
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ----------------------------- Pinos ---------------------------------------
const byte PINO_TRIG   = 12;
const byte PINO_ECHO   = 13;
const byte PINO_BOMBA  = 7;
const byte PINO_BUZZER = 8;
const byte PINOS_BARRA[4] = {2, 3, 4, 5};  // 25 %, 50 %, 75 %, 100 %

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----------------------------- Parâmetros ----------------------------------
const float DIST_CHEIO = 20.0;    // cm do sensor até a água com a caixa cheia
const float DIST_VAZIO = 220.0;   // cm do sensor até o fundo
const int LIGA_BOMBA = 25;        // %
const int DESLIGA_BOMBA = 90;     // %
const int NIVEL_CRITICO = 10;     // %
const unsigned long INTERVALO = 500;
const unsigned long TIMEOUT_ECO = 30000;  // µs (cerca de 5 m de alcance)

// ----------------------------- Estado --------------------------------------
bool bombaLigada = false;
bool sensorComFalha = false;
int nivel = -1;
unsigned long ultimaMedicao = 0;
unsigned long ultimoBipe = 0;

// Dispara o HC-SR04 e devolve a distância em cm (ou -1 se não houve eco)
float medirDistancia() {
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);
  unsigned long duracao = pulseIn(PINO_ECHO, HIGH, TIMEOUT_ECO);
  if (duracao == 0) return -1;
  return duracao / 58.0;  // ida e volta do som a cerca de 343 m/s
}

// Mediana de três medidas: descarta um eco espúrio isolado
float medirDistanciaFiltrada() {
  float a = medirDistancia(), b = medirDistancia(), c = medirDistancia();
  if (a < 0 || b < 0 || c < 0) return -1;
  if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
  if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
  return c;
}

int distanciaParaNivel(float distancia) {
  float pct = (DIST_VAZIO - distancia) * 100.0 / (DIST_VAZIO - DIST_CHEIO);
  return constrain((int)(pct + 0.5), 0, 100);
}

void acionarBomba(bool ligar) {
  if (ligar == bombaLigada) return;
  bombaLigada = ligar;
  digitalWrite(PINO_BOMBA, ligar ? HIGH : LOW);
  Serial.println(ligar ? F("[AGUA] Bomba LIGADA") : F("[AGUA] Bomba DESLIGADA"));
}

void atualizarBarra() {
  for (byte i = 0; i < 4; i++) {
    bool aceso = !sensorComFalha && nivel >= (i + 1) * 25 - 12;  // arredonda ao quarto mais próximo
    digitalWrite(PINOS_BARRA[i], aceso ? HIGH : LOW);
  }
}

void atualizarTela() {
  char linha[17];
  lcd.setCursor(0, 0);
  if (sensorComFalha) snprintf(linha, sizeof(linha), "%-16s", "FALHA NO SENSOR");
  else snprintf(linha, sizeof(linha), "Nivel: %3d%%     ", nivel);
  lcd.print(linha);
  lcd.setCursor(0, 1);
  const char *situacao = sensorComFalha ? "Bomba bloqueada" : bombaLigada ? "Bomba LIGADA" : "Bomba desligada";
  snprintf(linha, sizeof(linha), "%-16s", situacao);
  lcd.print(linha);
}

void setup() {
  Serial.begin(9600);
  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  pinMode(PINO_BOMBA, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  for (byte i = 0; i < 4; i++) pinMode(PINOS_BARRA[i], OUTPUT);
  lcd.init();
  lcd.backlight();
}

void loop() {
  unsigned long agora = millis();
  if (agora - ultimaMedicao < INTERVALO) return;
  ultimaMedicao = agora;

  float distancia = medirDistanciaFiltrada();
  bool falhaAntes = sensorComFalha;
  sensorComFalha = distancia < 0;

  if (sensorComFalha) {
    acionarBomba(false);               // nunca bombear "às cegas"
    if (!falhaAntes) Serial.println(F("[AGUA] Sensor sem resposta"));
  } else {
    nivel = distanciaParaNivel(distancia);
    if (nivel < LIGA_BOMBA) acionarBomba(true);
    else if (nivel >= DESLIGA_BOMBA) acionarBomba(false);
    // entre 25 % e 90 % a bomba mantém o estado anterior (histerese)
  }

  // Alarme de reserva crítica ou de falha: bipe curto a cada 2 s
  bool alarme = sensorComFalha || nivel < NIVEL_CRITICO;
  if (alarme && agora - ultimoBipe >= 2000) {
    ultimoBipe = agora;
    tone(PINO_BUZZER, 1000, 150);
  }

  atualizarBarra();
  atualizarTela();

  static int nivelRegistrado = -1;
  if (!sensorComFalha && nivel != nivelRegistrado) {
    nivelRegistrado = nivel;
    Serial.print(F("[AGUA] Distancia: "));
    Serial.print(distancia, 1);
    Serial.print(F(" cm | Nivel: "));
    Serial.print(nivel);
    Serial.println(F(" %"));
  }
}
