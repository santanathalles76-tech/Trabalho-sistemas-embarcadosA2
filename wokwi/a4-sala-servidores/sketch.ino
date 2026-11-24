/*
 * ===========================================================================
 *  Escritório Inteligente | Automação 4: Monitoramento da sala de servidores
 * ---------------------------------------------------------------------------
 *  Problema: se o ar da sala de servidores falha, a temperatura sobe em
 *  minutos e os equipamentos desligam ou se danificam, muitas vezes sem que
 *  ninguém perceba a tempo.
 *
 *  Entradas: sensor DHT22 (temperatura e umidade) e botão "Silenciar".
 *  Saídas:   LEDs verde, amarelo e vermelho, buzzer, relé da ventilação de
 *            emergência (com LED na carga) e LCD 16x2 I2C.
 *
 *  Lógica (três níveis, com base na faixa recomendada pela ASHRAE para
 *  data centers, de 18 a 27 °C):
 *    NORMAL  : 18 °C <= T <= 27 °C e umidade entre 20 % e 80 %
 *    ATENÇÃO : T fora da faixa normal, umidade fora da faixa ou falha de
 *              leitura do sensor (LED amarelo)
 *    CRÍTICO : T > 32 °C (LED vermelho, buzzer intermitente e ventilação)
 *  A ventilação de emergência desliga só quando T < 30 °C (histerese).
 *  O botão silencia o buzzer do alarme atual. Se a situação piorar de novo
 *  (voltar ao nível crítico depois de sair dele), o buzzer volta a tocar.
 *
 *  Plataforma: Arduino Uno (ATmega328P), simulado no Wokwi.
 * ===========================================================================
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ----------------------------- Pinos ---------------------------------------
const byte PINO_DHT          = 2;
const byte PINO_BOTAO        = 3;   // silenciar (INPUT_PULLUP)
const byte PINO_RELE         = 7;   // ventilação de emergência
const byte PINO_BUZZER       = 8;
const byte PINO_LED_VERDE    = 9;
const byte PINO_LED_AMARELO  = 10;
const byte PINO_LED_VERMELHO = 11;

DHT dht(PINO_DHT, DHT22);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----------------------------- Parâmetros ----------------------------------
const float T_MIN_NORMAL = 18.0;
const float T_MAX_NORMAL = 27.0;
const float T_CRITICA = 32.0;
const float T_DESLIGA_VENTILACAO = 30.0;
const float UR_MIN = 20.0, UR_MAX = 80.0;
const unsigned long INTERVALO_LEITURA = 2000;  // o DHT22 aceita 1 leitura a cada 2 s
const unsigned long PERIODO_BIPE = 1000;       // ms do ciclo liga/desliga do buzzer
const unsigned long DEBOUNCE = 50;

// ----------------------------- Estado --------------------------------------
enum Nivel { NORMAL, ATENCAO, CRITICO };
Nivel nivel = NORMAL;
bool silenciado = false;
bool ventilacaoLigada = false;
float temperatura = NAN, umidade = NAN;
unsigned long ultimaLeitura = 0;
bool primeiraLeitura = true;
bool buzzerTocando = false;

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

// Classifica a leitura atual em um dos três níveis
Nivel classificar(float t, float ur) {
  if (isnan(t) || isnan(ur)) return ATENCAO;           // sensor sem resposta
  if (t > T_CRITICA) return CRITICO;
  if (t < T_MIN_NORMAL || t > T_MAX_NORMAL) return ATENCAO;
  if (ur < UR_MIN || ur > UR_MAX) return ATENCAO;
  return NORMAL;
}

const char *nomeNivel(Nivel n) {
  return n == NORMAL ? "NORMAL" : n == ATENCAO ? "ATENCAO" : "CRITICO";
}

void atualizarSaidas() {
  digitalWrite(PINO_LED_VERDE, nivel == NORMAL ? HIGH : LOW);
  digitalWrite(PINO_LED_AMARELO, nivel == ATENCAO ? HIGH : LOW);
  digitalWrite(PINO_LED_VERMELHO, nivel == CRITICO ? HIGH : LOW);

  // Ventilação: liga no nível crítico e só desliga abaixo de 30 °C
  if (nivel == CRITICO) ventilacaoLigada = true;
  else if (!isnan(temperatura) && temperatura < T_DESLIGA_VENTILACAO) ventilacaoLigada = false;
  digitalWrite(PINO_RELE, ventilacaoLigada ? HIGH : LOW);
}

void atualizarTela() {
  char linha[17];
  lcd.setCursor(0, 0);
  if (isnan(temperatura)) {
    lcd.print("Falha no sensor ");
  } else {
    lcd.print("T:");
    lcd.print(temperatura, 1);
    lcd.print("C UR:");
    lcd.print(umidade, 0);
    lcd.print("%   ");
  }
  snprintf(linha, sizeof(linha), "%-8s%s", nomeNivel(nivel),
           ventilacaoLigada ? "VENT ON " : (silenciado ? "MUDO    " : "        "));
  lcd.setCursor(0, 1);
  lcd.print(linha);
}

void lerSensor() {
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();
  Nivel novo = classificar(temperatura, umidade);
  if (novo != nivel || primeiraLeitura) {
    // Ao sair do nível crítico, o silêncio perde efeito para o próximo alarme
    if (novo != CRITICO) silenciado = false;
    Serial.print(F("[SERVIDORES] Nivel: "));
    Serial.println(nomeNivel(novo));
    nivel = novo;
    primeiraLeitura = false;
  }
  atualizarSaidas();
  atualizarTela();
}

void setup() {
  Serial.begin(9600);
  pinMode(PINO_BOTAO, INPUT_PULLUP);
  pinMode(PINO_RELE, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_AMARELO, OUTPUT);
  pinMode(PINO_LED_VERMELHO, OUTPUT);
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.print("Iniciando...");
  ultimaLeitura = millis() - INTERVALO_LEITURA;  // lê logo no primeiro ciclo
}

void loop() {
  unsigned long agora = millis();

  if (agora - ultimaLeitura >= INTERVALO_LEITURA) {
    ultimaLeitura = agora;
    lerSensor();
  }

  if (botaoApertado() && nivel == CRITICO && !silenciado) {
    silenciado = true;
    Serial.println(F("[SERVIDORES] Alarme silenciado pelo operador"));
    atualizarTela();
  }

  // Buzzer intermitente no nível crítico, enquanto não for silenciado
  bool deveTocar = nivel == CRITICO && !silenciado && ((agora / (PERIODO_BIPE / 2)) % 2 == 0);
  if (deveTocar != buzzerTocando) {   // só mexe no buzzer quando o estado muda
    buzzerTocando = deveTocar;
    if (deveTocar) tone(PINO_BUZZER, 2500);
    else noTone(PINO_BUZZER);
  }
}
