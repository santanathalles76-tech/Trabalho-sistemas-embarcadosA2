/*
 * ===========================================================================
 *  Escritório Inteligente | Automação 5: Contador de pessoas e controle de
 *  lotação do auditório
 * ---------------------------------------------------------------------------
 *  Problema: em treinamentos e eventos internos o auditório passa do limite
 *  de pessoas definido pelo projeto de segurança contra incêndio, e ninguém
 *  sabe quantas pessoas estão lá dentro em caso de evacuação.
 *
 *  Entradas: botão do sensor de entrada (pedido de passagem na catraca) e
 *            botão do sensor de saída.
 *  Saídas:   servo da catraca de entrada, LEDs verde, amarelo e vermelho,
 *            buzzer e LCD 16x2 I2C com a ocupação.
 *
 *  Lógica:
 *    - Entrada: se ocupação < capacidade, libera a catraca por 2 s e soma 1;
 *      se estiver lotado, mantém a catraca travada e toca o aviso.
 *    - Saída: subtrai 1 (nunca fica negativo).
 *    - LED verde abaixo de 80 % da capacidade, amarelo a partir de 80 %,
 *      vermelho quando lotado.
 *  Capacidade reduzida para 8 pessoas na simulação.
 *
 *  Plataforma: Arduino Uno (ATmega328P), simulado no Wokwi.
 * ===========================================================================
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ----------------------------- Pinos ---------------------------------------
const byte PINO_ENTRADA      = 2;   // botões ligados ao GND (INPUT_PULLUP)
const byte PINO_SAIDA        = 3;
const byte PINO_SERVO        = 6;
const byte PINO_BUZZER       = 8;
const byte PINO_LED_VERDE    = 9;
const byte PINO_LED_AMARELO  = 10;
const byte PINO_LED_VERMELHO = 11;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo catraca;

// ----------------------------- Parâmetros ----------------------------------
const int CAPACIDADE = 8;
const int PERCENTUAL_ALERTA = 80;
const unsigned long TEMPO_CATRACA = 2000;  // ms com a catraca liberada
const unsigned long DEBOUNCE = 50;
const int ANGULO_TRAVADA = 0, ANGULO_LIVRE = 90;

// ----------------------------- Estado --------------------------------------
int ocupacao = 0;
bool catracaLiberada = false;
unsigned long inicioLiberacao = 0;

// Botão com debounce e detecção de borda (uma estrutura por botão)
struct Botao {
  byte pino;
  int leituraAnterior;
  int estadoEstavel;
  unsigned long ultimaMudanca;
};
Botao botaoEntrada = {PINO_ENTRADA, HIGH, HIGH, 0};
Botao botaoSaida = {PINO_SAIDA, HIGH, HIGH, 0};

bool foiApertado(Botao &b) {
  int leitura = digitalRead(b.pino);
  bool apertou = false;
  if (leitura != b.leituraAnterior) b.ultimaMudanca = millis();
  if (millis() - b.ultimaMudanca > DEBOUNCE && leitura != b.estadoEstavel) {
    b.estadoEstavel = leitura;
    if (b.estadoEstavel == LOW) apertou = true;
  }
  b.leituraAnterior = leitura;
  return apertou;
}

void atualizarPainel() {
  int percentual = ocupacao * 100 / CAPACIDADE;
  bool lotado = ocupacao >= CAPACIDADE;
  digitalWrite(PINO_LED_VERDE, percentual < PERCENTUAL_ALERTA ? HIGH : LOW);
  digitalWrite(PINO_LED_AMARELO, !lotado && percentual >= PERCENTUAL_ALERTA ? HIGH : LOW);
  digitalWrite(PINO_LED_VERMELHO, lotado ? HIGH : LOW);

  char linha[17];
  snprintf(linha, sizeof(linha), "Pessoas: %d/%d", ocupacao, CAPACIDADE);
  lcd.setCursor(0, 0);
  lcd.print(linha);
  lcd.print("    ");
  snprintf(linha, sizeof(linha), "%-16s", lotado ? "LOTADO" : percentual >= PERCENTUAL_ALERTA ? "Quase lotado" : "Entrada livre");
  lcd.setCursor(0, 1);
  lcd.print(linha);
}

void registrarEntrada() {
  if (ocupacao >= CAPACIDADE) {
    tone(PINO_BUZZER, 500, 700);  // aviso de lotação, catraca segue travada
    Serial.println(F("[LOTACAO] Entrada negada: auditorio lotado"));
    return;
  }
  ocupacao++;
  catraca.write(ANGULO_LIVRE);
  catracaLiberada = true;
  inicioLiberacao = millis();
  tone(PINO_BUZZER, 1500, 80);
  Serial.print(F("[LOTACAO] Entrada. Ocupacao: "));
  Serial.println(ocupacao);
  atualizarPainel();
}

void registrarSaida() {
  if (ocupacao == 0) {
    Serial.println(F("[LOTACAO] Saida ignorada: contador ja em zero"));
    return;
  }
  ocupacao--;
  Serial.print(F("[LOTACAO] Saida. Ocupacao: "));
  Serial.println(ocupacao);
  atualizarPainel();
}

void setup() {
  Serial.begin(9600);
  pinMode(PINO_ENTRADA, INPUT_PULLUP);
  pinMode(PINO_SAIDA, INPUT_PULLUP);
  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_AMARELO, OUTPUT);
  pinMode(PINO_LED_VERMELHO, OUTPUT);
  catraca.attach(PINO_SERVO);
  catraca.write(ANGULO_TRAVADA);
  lcd.init();
  lcd.backlight();
  atualizarPainel();
}

void loop() {
  if (foiApertado(botaoEntrada)) registrarEntrada();
  if (foiApertado(botaoSaida)) registrarSaida();

  // Trava a catraca de novo depois do tempo de passagem
  if (catracaLiberada && millis() - inicioLiberacao >= TEMPO_CATRACA) {
    catracaLiberada = false;
    catraca.write(ANGULO_TRAVADA);
  }
}
