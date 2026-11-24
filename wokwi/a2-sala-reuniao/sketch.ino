/*
 * ===========================================================================
 *  Escritório Inteligente | Automação 2: Sala de reunião com aviso de ocupação
 * ---------------------------------------------------------------------------
 *  Problema: pessoas abrem a porta da sala de reunião no meio de uma conversa
 *  para ver se está livre, e salas reservadas ficam vazias porque ninguém
 *  apareceu (o chamado "no-show"), bloqueando a agenda.
 *
 *  Entradas: sensor de presença PIR e botão "Reservar / Cancelar".
 *  Saídas:   LEDs verde (livre), amarelo (reservada), vermelho (ocupada)
 *            e LCD 16x2 I2C na porta da sala.
 *
 *  Lógica (máquina de estados):
 *    LIVRE      -> movimento detectado          -> OCUPADA
 *    LIVRE      -> botão                        -> RESERVADA
 *    RESERVADA  -> movimento (check-in)         -> OCUPADA
 *    RESERVADA  -> botão (cancelar)             -> LIVRE
 *    RESERVADA  -> 20 s sem ninguém (no-show)   -> LIVRE
 *    OCUPADA    -> 15 s sem movimento           -> LIVRE
 *  Os tempos estão reduzidos para a simulação (na prática, 10 e 5 minutos).
 *
 *  Plataforma: Arduino Uno (ATmega328P), simulado no Wokwi.
 * ===========================================================================
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ----------------------------- Pinos ---------------------------------------
const byte PINO_PIR          = 2;
const byte PINO_BOTAO        = 3;   // botão ligado ao GND (INPUT_PULLUP)
const byte PINO_LED_VERDE    = 9;
const byte PINO_LED_AMARELO  = 10;
const byte PINO_LED_VERMELHO = 11;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----------------------------- Parâmetros ----------------------------------
const unsigned long TOLERANCIA_RESERVA = 20000;  // ms até cancelar o no-show
const unsigned long TEMPO_SEM_MOVIMENTO = 15000; // ms parada até liberar a sala
const unsigned long DEBOUNCE = 50;               // ms para filtrar o ruído do botão

// ----------------------------- Estado --------------------------------------
enum EstadoSala { LIVRE, RESERVADA, OCUPADA };
EstadoSala estado = LIVRE;

unsigned long inicioEstado = 0;       // quando o estado atual começou
unsigned long ultimoMovimento = 0;    // último instante com PIR em nível alto
long ultimoSegundoMostrado = -1;

// Leitura do botão com debounce e detecção de borda
int leituraAnterior = HIGH;
int estadoEstavel = HIGH;
unsigned long ultimaMudanca = 0;

void mostrar(const char *linha1, const char *linha2) {
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "%-16s", linha1);
  lcd.setCursor(0, 0);
  lcd.print(buffer);
  snprintf(buffer, sizeof(buffer), "%-16s", linha2);
  lcd.setCursor(0, 1);
  lcd.print(buffer);
}

// Acende somente o LED que corresponde ao estado
void acenderLeds(bool verde, bool amarelo, bool vermelho) {
  digitalWrite(PINO_LED_VERDE, verde ? HIGH : LOW);
  digitalWrite(PINO_LED_AMARELO, amarelo ? HIGH : LOW);
  digitalWrite(PINO_LED_VERMELHO, vermelho ? HIGH : LOW);
}

void mudarEstado(EstadoSala novo) {
  estado = novo;
  inicioEstado = millis();
  ultimoSegundoMostrado = -1;
  switch (novo) {
    case LIVRE:
      acenderLeds(true, false, false);
      mostrar("Sala Atlantico", "LIVRE");
      Serial.println(F("[SALA] LIVRE"));
      break;
    case RESERVADA:
      acenderLeds(false, true, false);
      Serial.println(F("[SALA] RESERVADA: aguardando check-in"));
      break;
    case OCUPADA:
      acenderLeds(false, false, true);
      mostrar("Sala Atlantico", "OCUPADA");
      Serial.println(F("[SALA] OCUPADA: reuniao em andamento"));
      break;
  }
}

// Devolve true uma única vez a cada aperto do botão (borda de descida)
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

void setup() {
  Serial.begin(9600);
  pinMode(PINO_PIR, INPUT);
  pinMode(PINO_BOTAO, INPUT_PULLUP);
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_AMARELO, OUTPUT);
  pinMode(PINO_LED_VERMELHO, OUTPUT);
  lcd.init();
  lcd.backlight();
  mudarEstado(LIVRE);
}

void loop() {
  unsigned long agora = millis();
  bool movimento = digitalRead(PINO_PIR) == HIGH;
  bool apertou = botaoApertado();
  if (movimento) ultimoMovimento = agora;

  switch (estado) {
    case LIVRE:
      if (movimento) mudarEstado(OCUPADA);
      else if (apertou) mudarEstado(RESERVADA);
      break;

    case RESERVADA: {
      if (movimento) { mudarEstado(OCUPADA); break; }       // check-in
      if (apertou) { mudarEstado(LIVRE); break; }           // cancelamento
      unsigned long decorrido = agora - inicioEstado;
      if (decorrido >= TOLERANCIA_RESERVA) {                // no-show
        Serial.println(F("[SALA] Ninguem chegou: reserva cancelada"));
        mudarEstado(LIVRE);
        break;
      }
      long restante = (TOLERANCIA_RESERVA - decorrido + 999) / 1000;
      if (restante != ultimoSegundoMostrado) {              // contagem no visor
        ultimoSegundoMostrado = restante;
        char linha[17];
        snprintf(linha, sizeof(linha), "RESERVADA %lds", restante);
        mostrar("Sala Atlantico", linha);
      }
      break;
    }

    case OCUPADA:
      // A sala só é liberada depois de um período contínuo sem movimento
      if (agora - ultimoMovimento >= TEMPO_SEM_MOVIMENTO) mudarEstado(LIVRE);
      break;
  }
}
