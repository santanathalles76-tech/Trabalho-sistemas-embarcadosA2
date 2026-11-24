#include "testes.h"
#include "mock/LiquidCrystal_I2C.h"
#include "../wokwi/a5-controle-lotacao/sketch.ino"
int main() {
  std::cout << "Automação 5: contador de pessoas e lotação\n";
  sim::reiniciar(); setup(); avancar(100);
  CONFERE(ocupacao == 0 && sim::saida[PINO_LED_VERDE] && lcd.texto(0) == "Pessoas: 0/8", "Início: 0/8, LED verde");
  apertar(PINO_ENTRADA);
  CONFERE(ocupacao == 1 && sim::angulo[PINO_SERVO] == 90, "Entrada: soma 1 e libera a catraca (90 graus)");
  avancar(2100);
  CONFERE(sim::angulo[PINO_SERVO] == 0, "Catraca volta a travar após 2 s");
  for (int i = 0; i < 5; i++) apertar(PINO_ENTRADA);
  CONFERE(ocupacao == 6 && sim::saida[PINO_LED_VERDE], "6/8 (75 %): ainda verde");
  apertar(PINO_ENTRADA);
  CONFERE(ocupacao == 7 && sim::saida[PINO_LED_AMARELO] && lcd.texto(1) == "Quase lotado", "7/8 (87 %): LED amarelo, 'Quase lotado'");
  apertar(PINO_ENTRADA);
  CONFERE(ocupacao == 8 && sim::saida[PINO_LED_VERMELHO] && lcd.texto(1) == "LOTADO", "8/8: LED vermelho, 'LOTADO'");
  avancar(2100);
  apertar(PINO_ENTRADA);
  CONFERE(ocupacao == 8 && sim::angulo[PINO_SERVO] == 0 && sim::som[PINO_BUZZER].frequencia == 500, "Lotado: nova entrada negada, catraca travada e aviso sonoro");
  apertar(PINO_SAIDA);
  CONFERE(ocupacao == 7, "Saída: subtrai 1");
  for (int i = 0; i < 9; i++) apertar(PINO_SAIDA);
  CONFERE(ocupacao == 0, "Saídas a mais não deixam o contador negativo");
  return fim();
}
