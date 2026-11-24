#include "testes.h"
#include "mock/LiquidCrystal_I2C.h"
#include "../wokwi/a8-reservatorio-agua/sketch.ino"
void distancia(double cm) { sim::ecoUs = (unsigned long)lround(cm * 58); avancar(600); }
int main() {
  std::cout << "Automação 8: reservatório de água\n";
  sim::reiniciar(); setup();
  distancia(150);
  CONFERE(nivel == 35 && !sim::saida[PINO_BOMBA] && lcd.texto(0) == "Nivel:  35%", "150 cm da tampa: 35 %, bomba desligada");
  CONFERE(sim::saida[2] && !sim::saida[3], "Barra de LEDs: 1 de 4 aceso");
  distancia(180);
  CONFERE(nivel == 20 && sim::saida[PINO_BOMBA], "20 % (abaixo de 25 %): bomba liga");
  distancia(100);
  CONFERE(nivel == 60 && sim::saida[PINO_BOMBA], "60 %: bomba continua ligada enchendo (histerese)");
  CONFERE(sim::saida[2] && sim::saida[3] && !sim::saida[4], "Barra de LEDs em 60 %: 2 de 4");
  distancia(38);
  CONFERE(nivel == 91 && !sim::saida[PINO_BOMBA], "91 %: bomba desliga");
  distancia(100);
  CONFERE(!sim::saida[PINO_BOMBA], "Volta a 60 %: bomba continua desligada (histerese)");
  distancia(205);
  CONFERE(nivel == 8 && sim::saida[PINO_BOMBA], "8 %: bomba liga");
  bool bipou = false; for (int i = 0; i < 2100; i++) { loop(); sim::agora++; bipou |= sim::tocando(PINO_BUZZER); }
  CONFERE(bipou, "Reserva crítica (< 10 %): bipe de alerta");
  sim::ecoUs = 0; avancar(600);
  CONFERE(sensorComFalha && !sim::saida[PINO_BOMBA] && lcd.texto(0) == "FALHA NO SENSOR", "Sem eco do sensor: bomba bloqueada e falha no visor");
  return fim();
}
