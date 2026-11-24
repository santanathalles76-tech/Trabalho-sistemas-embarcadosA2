#include "testes.h"
#include "mock/LiquidCrystal_I2C.h"
#include "../wokwi/a4-sala-servidores/sketch.ino"
bool buzzerTocouEm(unsigned long ms) { bool tocou = false; unsigned long fim_ = sim::agora + ms; while (sim::agora < fim_) { loop(); sim::agora++; tocou |= sim::tocando(PINO_BUZZER); } return tocou; }
void ajustar(float t, float h) { sim::dhtT = t; sim::dhtH = h; avancar(2100); }
int main() {
  std::cout << "Automação 4: sala de servidores\n";
  sim::reiniciar(); setup(); ajustar(24, 45);
  CONFERE(nivel == NORMAL && sim::saida[PINO_LED_VERDE] && !sim::saida[PINO_RELE], "24 C e 45 %: NORMAL, LED verde, ventilação desligada");
  ajustar(29, 45);
  CONFERE(nivel == ATENCAO && sim::saida[PINO_LED_AMARELO] && !buzzerTocouEm(1500), "29 C: ATENCAO, LED amarelo, sem sirene");
  ajustar(24, 85);
  CONFERE(nivel == ATENCAO, "Umidade 85 %: ATENCAO");
  ajustar(33, 45);
  CONFERE(nivel == CRITICO && sim::saida[PINO_LED_VERMELHO] && sim::saida[PINO_RELE], "33 C: CRITICO, LED vermelho e ventilação de emergência ligada");
  CONFERE(buzzerTocouEm(1200), "No nível crítico o buzzer toca de forma intermitente");
  apertar(PINO_BOTAO);
  CONFERE(silenciado && !buzzerTocouEm(3000), "Botão Silenciar: buzzer para");
  CONFERE(sim::saida[PINO_RELE], "Silenciar não desliga a ventilação");
  ajustar(31, 45);
  CONFERE(nivel == ATENCAO && sim::saida[PINO_RELE], "31 C: volta a ATENCAO, mas a ventilação segue até ficar abaixo de 30 C");
  ajustar(29.5, 45);
  CONFERE(!sim::saida[PINO_RELE], "29,5 C: ventilação desliga (histerese)");
  ajustar(34, 45);
  CONFERE(!silenciado && buzzerTocouEm(1200), "Nova piora para 34 C: alarme volta a tocar");
  ajustar(NAN, NAN);
  CONFERE(nivel == ATENCAO && lcd.texto(0) == "Falha no sensor", "Sensor sem resposta: ATENCAO e aviso no visor");
  return fim();
}
