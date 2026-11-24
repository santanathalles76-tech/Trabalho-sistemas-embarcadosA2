#include "testes.h"
#include "mock/LiquidCrystal_I2C.h"
#include "../wokwi/a9-painel-senhas/sketch.ino"
int main() {
  std::cout << "Automação 9: painel de senhas\n";
  sim::reiniciar(); setup(); avancar(100);
  apertar(PINO_CHAMAR);
  CONFERE(lcd.texto(1) == "Fila vazia" && std::string(ultimaChamada) == "----", "Chamar com fila vazia: aviso, nada é chamado");
  for (int i = 0; i < 3; i++) apertar(PINO_NORMAL);
  for (int i = 0; i < 3; i++) apertar(PINO_PREFERENCIAL);
  CONFERE(lcd.texto(1) == "Fila N:3  P:3", "3 normais e 3 preferenciais emitidas");
  const char *esperado[] = {"P001", "P002", "N001", "P003", "N002", "N003"};
  std::string ordem;
  for (int i = 0; i < 6; i++) { apertar(PINO_CHAMAR); ordem += std::string(ultimaChamada) + " "; }
  std::cout << "  ordem de chamada: " << ordem << "\n";
  bool ok = true; for (int i = 0; i < 6; i++) ok &= ordem.substr(i * 5, 4) == esperado[i];
  CONFERE(ok, "Ordem P001 P002 N001 P003 N002 N003 (2 preferenciais, depois 1 normal)");
  CONFERE(lcd.texto(0) == "Senha: N003" && sim::saida[PINO_LED_CHAMANDO], "Painel mostra a última senha e o LED acende");
  avancar(3100);
  CONFERE(!sim::saida[PINO_LED_CHAMANDO], "LED apaga depois de 3 s");
  apertar(PINO_PREFERENCIAL); apertar(PINO_CHAMAR); apertar(PINO_PREFERENCIAL); apertar(PINO_CHAMAR); apertar(PINO_PREFERENCIAL); apertar(PINO_CHAMAR);
  CONFERE(std::string(ultimaChamada) == "P006", "Só há preferenciais: chamadas em sequência, sem travar");
  return fim();
}
