/*
 * ===========================================================================
 *  Escritório Inteligente | Automação 6: Iluminação com aproveitamento da
 *  luz natural (daylight harvesting)
 * ---------------------------------------------------------------------------
 *  Problema: as luminárias das estações de trabalho ficam em 100 % mesmo
 *  com o sol entrando pela janela, gastando energia sem necessidade.
 *
 *  Entrada: sensor de luminosidade LDR (módulo com saída analógica).
 *  Saídas:  luminária dimerizável (LED controlado por PWM) e LCD 16x2 I2C.
 *
 *  Lógica (controle proporcional):
 *    - Converte a leitura do LDR em lux.
 *    - A meta é 500 lux no plano de trabalho, valor da norma
 *      ABNT NBR ISO/CIE 8995-1 para escritórios.
 *    - A luminária completa só o que falta: potência = (500 - lux) / 500.
 *    - Média móvel exponencial suaviza a leitura, e uma faixa morta de 2 %
 *      evita que a luz fique "tremendo" com pequenas variações.
 *
 *  Plataforma: Arduino Uno (ATmega328P), simulado no Wokwi.
 * ===========================================================================
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

// ----------------------------- Pinos ---------------------------------------
const byte PINO_LDR        = A0;
const byte PINO_LUMINARIA  = 9;   // pino com PWM

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----------------------------- Parâmetros ----------------------------------
const float GAMMA = 0.7;             // constantes do LDR do Wokwi
const float RL10 = 50;               // resistência (kΩ) a 10 lux
const float META_LUX = 500.0;        // iluminância desejada (NBR ISO/CIE 8995-1)
const float SUAVIZACAO = 0.2;        // peso da leitura nova na média móvel
const int FAIXA_MORTA = 5;           // passos de PWM (cerca de 2 %)
const unsigned long INTERVALO = 100; // ms entre leituras

// ----------------------------- Estado --------------------------------------
float luxSuavizado = -1;             // -1 indica que ainda não houve leitura
int potenciaAtual = -1;              // 0 a 255
unsigned long ultimaLeitura = 0;
unsigned long ultimaTela = 0;

// Converte a leitura analógica do módulo LDR em lux
float lerLux() {
  int leitura = analogRead(PINO_LDR);
  leitura = constrain(leitura, 1, 1022);       // evita divisão por zero
  float tensao = leitura / 1024.0 * 5.0;
  float resistencia = 2000.0 * tensao / (1.0 - tensao / 5.0);
  return pow(RL10 * 1e3 * pow(10, GAMMA) / resistencia, 1.0 / GAMMA);
}

// Calcula a potência da luminária (0 a 255) que completa a luz natural
int calcularPotencia(float lux) {
  if (lux >= META_LUX) return 0;               // luz natural já basta
  float falta = (META_LUX - lux) / META_LUX;   // 0,0 a 1,0
  return constrain((int)(falta * 255.0 + 0.5), 0, 255);
}

void setup() {
  Serial.begin(9600);
  pinMode(PINO_LUMINARIA, OUTPUT);
  lcd.init();
  lcd.backlight();
}

void loop() {
  unsigned long agora = millis();
  if (agora - ultimaLeitura < INTERVALO) return;
  ultimaLeitura = agora;

  float lux = lerLux();
  // Média móvel exponencial: filtra variações rápidas (nuvem, sombra de pessoa)
  if (luxSuavizado < 0) luxSuavizado = lux;
  else luxSuavizado = SUAVIZACAO * lux + (1.0 - SUAVIZACAO) * luxSuavizado;

  int potencia = calcularPotencia(luxSuavizado);
  // Faixa morta: só ajusta a luminária quando a mudança é perceptível
  if (potenciaAtual < 0 || abs(potencia - potenciaAtual) >= FAIXA_MORTA ||
      (potencia == 0 && potenciaAtual != 0) || (potencia == 255 && potenciaAtual != 255)) {
    potenciaAtual = potencia;
    analogWrite(PINO_LUMINARIA, potenciaAtual);
    Serial.print(F("[LUZ] Natural: "));
    Serial.print(luxSuavizado, 0);
    Serial.print(F(" lux | Luminaria: "));
    Serial.print(potenciaAtual * 100 / 255);
    Serial.println(F(" %"));
  }

  if (agora - ultimaTela >= 500) {
    ultimaTela = agora;
    lcd.setCursor(0, 0);
    lcd.print("Natural:");
    lcd.print(luxSuavizado > 9999 ? 9999 : (int)luxSuavizado);
    lcd.print(" lx    ");
    lcd.setCursor(0, 1);
    lcd.print("Luminaria: ");
    lcd.print(potenciaAtual * 100 / 255);
    lcd.print("%   ");
  }
}
