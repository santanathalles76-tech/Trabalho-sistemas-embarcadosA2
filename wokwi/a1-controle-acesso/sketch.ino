/*
 * ===========================================================================
 *  Escritório Inteligente | Automação 1: Controle de acesso por senha
 * ---------------------------------------------------------------------------
 *  Problema: a porta da área administrativa fica destrancada ou depende de
 *  chave física, sem registro de tentativas e sem proteção contra quem tenta
 *  adivinhar a senha.
 *
 *  Entradas: teclado matricial 4x4.
 *  Saídas:   servo motor (trava da porta), LED verde (acesso liberado),
 *            LED vermelho (negado ou bloqueado), buzzer e LCD 16x2 I2C.
 *
 *  Lógica (máquina de estados):
 *    AGUARDANDO -> senha correta + '#'            -> LIBERADO (porta 5 s aberta)
 *    AGUARDANDO -> senha errada + '#' (3a vez)    -> BLOQUEADO (15 s)
 *    LIBERADO   -> fim do tempo                   -> AGUARDANDO (trava fecha)
 *    BLOQUEADO  -> fim do tempo                   -> AGUARDANDO (zera erros)
 *  Tecla '*' apaga o que foi digitado. Letras A a D são ignoradas.
 *
 *  Plataforma: Arduino Uno (ATmega328P), simulado no Wokwi.
 * ===========================================================================
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// ----------------------------- Pinos ---------------------------------------
const byte PINO_SERVO        = 10;
const byte PINO_BUZZER       = 11;
const byte PINO_LED_VERDE    = 12;
const byte PINO_LED_VERMELHO = 13;

// ----------------------------- Teclado 4x4 ---------------------------------
const byte LINHAS = 4;
const byte COLUNAS = 4;
char mapaTeclas[LINHAS][COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte pinosLinhas[LINHAS]   = {9, 8, 7, 6};   // R1..R4
byte pinosColunas[COLUNAS] = {5, 4, 3, 2};   // C1..C4
Keypad teclado = Keypad(makeKeymap(mapaTeclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

LiquidCrystal_I2C lcd(0x27, 16, 2);  // endereço padrão do LCD I2C no Wokwi
Servo trava;

// ----------------------------- Parâmetros ----------------------------------
const char SENHA_CORRETA[] = "4729";            // senha da área administrativa
const byte TAMANHO_MAXIMO = 6;                   // dígitos aceitos no visor
const byte TENTATIVAS_MAXIMAS = 3;               // erros seguidos até bloquear
const unsigned long TEMPO_PORTA_ABERTA = 5000;   // ms com a trava aberta
const unsigned long TEMPO_BLOQUEIO = 15000;      // ms bloqueado (na prática, 5 min)
const int ANGULO_TRAVADA = 0;
const int ANGULO_DESTRAVADA = 90;

// ----------------------------- Estado --------------------------------------
enum Estado { AGUARDANDO, LIBERADO, BLOQUEADO };
Estado estado = AGUARDANDO;

char digitada[TAMANHO_MAXIMO + 1] = "";  // senha que está sendo digitada
byte quantidadeDigitos = 0;
byte errosSeguidos = 0;
unsigned long inicioEstado = 0;          // millis() em que o estado começou
long ultimoSegundoMostrado = -1;         // evita reescrever o LCD sem necessidade

// Escreve as duas linhas do LCD completando com espaços (sem piscar a tela)
void mostrar(const char *linha1, const char *linha2) {
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "%-16s", linha1);
  lcd.setCursor(0, 0);
  lcd.print(buffer);
  snprintf(buffer, sizeof(buffer), "%-16s", linha2);
  lcd.setCursor(0, 1);
  lcd.print(buffer);
}

// Mostra a senha mascarada com asteriscos
void mostrarTelaSenha() {
  char mascara[17] = "Senha: ";
  for (byte i = 0; i < quantidadeDigitos; i++) strcat(mascara, "*");
  mostrar("Area restrita", mascara);
}

void apagarDigitada() {
  quantidadeDigitos = 0;
  digitada[0] = '\0';
}

void entrarAguardando() {
  estado = AGUARDANDO;
  inicioEstado = millis();
  apagarDigitada();
  trava.write(ANGULO_TRAVADA);
  digitalWrite(PINO_LED_VERDE, LOW);
  digitalWrite(PINO_LED_VERMELHO, LOW);
  mostrarTelaSenha();
  Serial.println(F("[ACESSO] Aguardando senha"));
}

void liberarAcesso() {
  estado = LIBERADO;
  inicioEstado = millis();
  errosSeguidos = 0;
  trava.write(ANGULO_DESTRAVADA);
  digitalWrite(PINO_LED_VERDE, HIGH);
  digitalWrite(PINO_LED_VERMELHO, LOW);
  tone(PINO_BUZZER, 1800, 200);  // bipe curto e agudo de confirmação
  mostrar("Acesso liberado", "Porta aberta");
  Serial.println(F("[ACESSO] Senha correta: porta destravada"));
}

void bloquearTeclado() {
  estado = BLOQUEADO;
  inicioEstado = millis();
  ultimoSegundoMostrado = -1;
  digitalWrite(PINO_LED_VERDE, LOW);
  digitalWrite(PINO_LED_VERMELHO, HIGH);
  tone(PINO_BUZZER, 400, 1500);  // som grave e longo de alerta
  Serial.println(F("[ACESSO] 3 erros seguidos: teclado bloqueado"));
}

// Compara a senha digitada com a cadastrada e decide o próximo estado
void conferirSenha() {
  if (quantidadeDigitos == 0) return;  // '#' sem nada digitado não conta
  if (strcmp(digitada, SENHA_CORRETA) == 0) {
    liberarAcesso();
    return;
  }
  errosSeguidos++;
  Serial.print(F("[ACESSO] Senha incorreta. Erros seguidos: "));
  Serial.println(errosSeguidos);
  if (errosSeguidos >= TENTATIVAS_MAXIMAS) {
    bloquearTeclado();
    return;
  }
  char aviso[20];
  snprintf(aviso, sizeof(aviso), "Restam %d tent.", TENTATIVAS_MAXIMAS - errosSeguidos);
  digitalWrite(PINO_LED_VERMELHO, HIGH);
  tone(PINO_BUZZER, 400, 400);
  mostrar("Senha incorreta", aviso);
  delay(1500);  // tempo de leitura da mensagem
  digitalWrite(PINO_LED_VERMELHO, LOW);
  apagarDigitada();
  mostrarTelaSenha();
}

// Trata uma tecla pressionada enquanto o sistema aguarda a senha
void tratarTecla(char tecla) {
  if (tecla >= '0' && tecla <= '9') {
    if (quantidadeDigitos < TAMANHO_MAXIMO) {
      digitada[quantidadeDigitos++] = tecla;
      digitada[quantidadeDigitos] = '\0';
      tone(PINO_BUZZER, 1200, 40);  // clique de tecla
      mostrarTelaSenha();
    }
  } else if (tecla == '*') {
    apagarDigitada();
    mostrarTelaSenha();
  } else if (tecla == '#') {
    conferirSenha();
  }
  // 'A', 'B', 'C' e 'D' não têm função nesta automação
}

void setup() {
  Serial.begin(9600);
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_VERMELHO, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  trava.attach(PINO_SERVO);
  lcd.init();
  lcd.backlight();
  entrarAguardando();
}

void loop() {
  unsigned long agora = millis();
  char tecla = teclado.getKey();

  switch (estado) {
    case AGUARDANDO:
      if (tecla) tratarTecla(tecla);
      break;

    case LIBERADO:
      // Fecha a trava sozinho depois do tempo de passagem
      if (agora - inicioEstado >= TEMPO_PORTA_ABERTA) {
        Serial.println(F("[ACESSO] Tempo esgotado: porta travada"));
        entrarAguardando();
      }
      break;

    case BLOQUEADO: {
      if (tecla) tone(PINO_BUZZER, 300, 100);  // teclas são ignoradas
      unsigned long decorrido = agora - inicioEstado;
      if (decorrido >= TEMPO_BLOQUEIO) {
        errosSeguidos = 0;
        Serial.println(F("[ACESSO] Fim do bloqueio"));
        entrarAguardando();
        break;
      }
      // Contagem regressiva no visor, atualizada uma vez por segundo
      long restante = (TEMPO_BLOQUEIO - decorrido + 999) / 1000;
      if (restante != ultimoSegundoMostrado) {
        ultimoSegundoMostrado = restante;
        char linha[17];
        snprintf(linha, sizeof(linha), "Aguarde %ld s", restante);
        mostrar("ACESSO BLOQUEADO", linha);
      }
      break;
    }
  }
}
