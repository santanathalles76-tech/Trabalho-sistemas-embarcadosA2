#include "testes.h"
#include "../wokwi/a7-vazamento-gas/sketch.ino"
int main() {
  std::cout << "Automação 7: vazamento de gás\n";
  sim::reiniciar(); sim::analogico[PINO_GAS_ANALOG] = 200; sim::entrada[PINO_GAS_DIGITAL] = HIGH;
  setup(); avancar(3200);
  CONFERE(estado == MONITORANDO && linhaDeBase == 200, "Calibração de 3 s: linha de base 200");
  sim::analogico[PINO_GAS_ANALOG] = 330; avancar(500);
  CONFERE(estado == MONITORANDO && sim::angulo[PINO_VALVULA] == 0, "Leitura 330 (abaixo de 350): sem alarme, válvula aberta");
  sim::analogico[PINO_GAS_ANALOG] = 420; avancar(700);
  CONFERE(estado == ALARME && sim::angulo[PINO_VALVULA] == 90 && sim::saida[PINO_EXAUSTOR], "Leitura 420: alarme, válvula fecha e exaustor liga");
  CONFERE(sim::tocando(PINO_BUZZER), "Sirene tocando");
  apertar(PINO_BOTAO);
  CONFERE(estado == ALARME, "Rearme com gás ainda alto: recusado");
  sim::analogico[PINO_GAS_ANALOG] = 300; avancar(500);
  CONFERE(estado == ALARME, "Gás cai para 300 (acima do nível seguro): alarme segue travado");
  sim::analogico[PINO_GAS_ANALOG] = 260; avancar(500);
  CONFERE(estado == ALARME, "Gás em nível seguro: alarme só sai com o botão (latch)");
  apertar(PINO_BOTAO);
  CONFERE(estado == MONITORANDO && sim::angulo[PINO_VALVULA] == 0 && !sim::saida[PINO_EXAUSTOR] && !sim::tocando(PINO_BUZZER), "Botão com gás seguro: rearmado, válvula abre, exaustor e sirene desligam");
  sim::entrada[PINO_GAS_DIGITAL] = LOW; avancar(100);
  CONFERE(estado == ALARME, "Saída digital do módulo em nível baixo: alarme por redundância");
  return fim();
}
