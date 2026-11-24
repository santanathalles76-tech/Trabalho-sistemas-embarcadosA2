/*
 * ===========================================================================
 *  Escritório Inteligente | Automação 9: Painel de senhas da recepção com
 *  atendimento preferencial
 * ---------------------------------------------------------------------------
 *  Problema: na recepção, visitantes e fornecedores formam fila sem ordem
 *  definida, e o atendimento preferencial (idosos, gestantes, pessoas com
 *  deficiência) fica a critério de quem está no balcão.
 *
 *  Entradas: botão "Senha normal", botão "Senha preferencial" (totem) e
 *            botão "Chamar próximo" (balcão da recepção).
 *  Saídas:   LCD 16x2 I2C (painel), buzzer (aviso de chamada) e LED
 *            "Chamando".
 *
 *  Lógica (duas filas com prioridade):
 *    - Cada botão do totem emite uma senha: N001, N002... ou P001, P002...
 *    - "Chamar próximo" dá prioridade à fila preferencial, mas depois de
 *      2 preferenciais seguidas chama 1 normal, se houver alguém esperando.
 *      Assim a fila normal nunca fica parada indefinidamente.
 *    - Fila vazia: o painel avisa e nada é chamado.
 *    - Cada fila guarda até 20 senhas aguardando.
 *
 *  Plataforma: Arduino Uno (ATmega328P), simulado no Wokwi.
 * ===========================================================================
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ----------------------------- Pinos ---------------------------------------
const byte PINO_NORMAL       = 2;   // botões ligados ao GND (INPUT_PULLUP)
const byte PINO_PREFERENCIAL = 3;
const byte PINO_CHAMAR       = 4;
const byte PINO_BUZZER       = 8;
const byte PINO_LED_CHAMANDO = 9;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----------------------------- Parâmetros ----------------------------------
const byte MAX_PREFERENCIAIS_SEGUIDAS = 2;
const int LIMITE_FILA = 20;
const unsigned long TEMPO_LED = 3000;
const unsigned long DEBOUNCE = 50;

// ----------------------------- Estado --------------------------------------
// Cada fila é controlada por dois contadores: senhas emitidas e chamadas.
// Quem aguarda = emitidas - chamadas.
int emitidasNormal = 0, chamadasNormal = 0;
int emitidasPref = 0, chamadasPref = 0;
byte preferenciaisSeguidas = 0;
char ultimaChamada[6] = "----";
unsigned long inicioChamada = 0;
bool ledChamando = false;

struct Botao {
  byte pino;
  int leituraAnterior;
  int estadoEstavel;
  unsigned long ultimaMudanca;
};
Botao botaoNormal = {PINO_NORMAL, HIGH, HIGH, 0};
Botao botaoPref = {PINO_PREFERENCIAL, HIGH, HIGH, 0};
Botao botaoChamar = {PINO_CHAMAR, HIGH, HIGH, 0};

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

int aguardandoNormal() { return emitidasNormal - chamadasNormal; }
int aguardandoPref() { return emitidasPref - chamadasPref; }

void atualizarPainel(const char *mensagem) {
  char linha[17];
  snprintf(linha, sizeof(linha), "Senha: %-9s", ultimaChamada);
  lcd.setCursor(0, 0);
  lcd.print(linha);
  if (mensagem) snprintf(linha, sizeof(linha), "%-16s", mensagem);
  else snprintf(linha, sizeof(linha), "Fila N:%-2d P:%-2d  ", aguardandoNormal(), aguardandoPref());
  lcd.setCursor(0, 1);
  lcd.print(linha);
}

void emitirSenha(bool preferencial) {
  if ((preferencial ? aguardandoPref() : aguardandoNormal()) >= LIMITE_FILA) {
    tone(PINO_BUZZER, 300, 300);
    atualizarPainel("Fila cheia");
    return;
  }
  char senha[6];
  if (preferencial) snprintf(senha, sizeof(senha), "P%03d", ++emitidasPref);
  else snprintf(senha, sizeof(senha), "N%03d", ++emitidasNormal);
  tone(PINO_BUZZER, 1500, 60);
  Serial.print(F("[RECEPCAO] Senha emitida: "));
  Serial.println(senha);
  atualizarPainel(NULL);
}

// Decide qual fila atende agora e chama a próxima senha
void chamarProximo() {
  bool temNormal = aguardandoNormal() > 0;
  bool temPref = aguardandoPref() > 0;
  if (!temNormal && !temPref) {
    tone(PINO_BUZZER, 300, 200);
    atualizarPainel("Fila vazia");
    Serial.println(F("[RECEPCAO] Nenhuma senha aguardando"));
    return;
  }
  bool chamarPreferencial = temPref &&
      (preferenciaisSeguidas < MAX_PREFERENCIAIS_SEGUIDAS || !temNormal);

  if (chamarPreferencial) {
    chamadasPref++;
    preferenciaisSeguidas++;
    snprintf(ultimaChamada, sizeof(ultimaChamada), "P%03d", chamadasPref);
  } else {
    chamadasNormal++;
    preferenciaisSeguidas = 0;
    snprintf(ultimaChamada, sizeof(ultimaChamada), "N%03d", chamadasNormal);
  }

  // "Plim-plom" de chamada e LED aceso por 3 s
  tone(PINO_BUZZER, 1319, 250);
  delay(260);
  tone(PINO_BUZZER, 988, 350);
  digitalWrite(PINO_LED_CHAMANDO, HIGH);
  ledChamando = true;
  inicioChamada = millis();

  Serial.print(F("[RECEPCAO] Chamando: "));
  Serial.println(ultimaChamada);
  atualizarPainel(NULL);
}

void setup() {
  Serial.begin(9600);
  pinMode(PINO_NORMAL, INPUT_PULLUP);
  pinMode(PINO_PREFERENCIAL, INPUT_PULLUP);
  pinMode(PINO_CHAMAR, INPUT_PULLUP);
  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_LED_CHAMANDO, OUTPUT);
  lcd.init();
  lcd.backlight();
  atualizarPainel(NULL);
}

void loop() {
  if (foiApertado(botaoNormal)) emitirSenha(false);
  if (foiApertado(botaoPref)) emitirSenha(true);
  if (foiApertado(botaoChamar)) chamarProximo();

  if (ledChamando && millis() - inicioChamada >= TEMPO_LED) {
    ledChamando = false;
    digitalWrite(PINO_LED_CHAMANDO, LOW);
  }
}
