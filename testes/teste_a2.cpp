#include "testes.h"
#include "mock/LiquidCrystal_I2C.h"
#include "../wokwi/a2-sala-reuniao/sketch.ino"
int main() {
  std::cout << "Automação 2: sala de reunião\n";
  sim::reiniciar(); sim::entrada[PINO_PIR] = LOW; setup(); avancar(100);
  CONFERE(estado == LIVRE && sim::saida[PINO_LED_VERDE] && lcd.texto(1) == "LIVRE", "Sem ninguém: LIVRE, LED verde");
  apertar(PINO_BOTAO);
  CONFERE(estado == RESERVADA && sim::saida[PINO_LED_AMARELO] && lcd.texto(1).rfind("RESERVADA", 0) == 0, "Botão: RESERVADA, LED amarelo e contagem no visor");
  avancar(20500);
  CONFERE(estado == LIVRE, "Reserva sem check-in em 20 s (no-show): volta a LIVRE");
  apertar(PINO_BOTAO); apertar(PINO_BOTAO);
  CONFERE(estado == LIVRE, "Botão de novo durante a reserva: reserva cancelada");
  apertar(PINO_BOTAO);
  sim::entrada[PINO_PIR] = HIGH; avancar(5000); sim::entrada[PINO_PIR] = LOW;
  CONFERE(estado == OCUPADA && sim::saida[PINO_LED_VERMELHO] && lcd.texto(1) == "OCUPADA", "Movimento com a sala reservada: check-in, OCUPADA, LED vermelho");
  avancar(14000);
  CONFERE(estado == OCUPADA, "14 s parada: continua OCUPADA");
  sim::entrada[PINO_PIR] = HIGH; avancar(1000); sim::entrada[PINO_PIR] = LOW; avancar(14000);
  CONFERE(estado == OCUPADA, "Novo movimento reinicia a contagem de 15 s");
  avancar(1500);
  CONFERE(estado == LIVRE && sim::saida[PINO_LED_VERDE], "15 s sem movimento: sala liberada");
  sim::entrada[PINO_PIR] = HIGH; avancar(50);
  CONFERE(estado == OCUPADA, "Movimento com a sala LIVRE (sem reserva): OCUPADA direto");
  return fim();
}
