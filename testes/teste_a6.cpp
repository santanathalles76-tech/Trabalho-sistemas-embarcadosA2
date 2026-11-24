#include "testes.h"
#include "mock/LiquidCrystal_I2C.h"
#include "../wokwi/a6-iluminacao-natural/sketch.ino"
// Leitura analógica que o LDR do Wokwi daria para a iluminância lux
int ldr(double lux) { double r = 50e3 * pow(10, 0.7) / pow(lux, 0.7); double v = r / (2000 + r / 5); return (int)lround(v / 5 * 1024); }
int pct() { return sim::pwm[PINO_LUMINARIA] * 100 / 255; }
int main() {
  std::cout << "Automação 6: iluminação com luz natural\n";
  sim::reiniciar(); sim::analogico[PINO_LDR] = ldr(50); setup(); avancar(3000);
  CONFERE(pct() >= 88 && pct() <= 92, "Sala escura (50 lux): luminária a 90 %");
  sim::analogico[PINO_LDR] = ldr(250); avancar(3000);
  CONFERE(pct() >= 48 && pct() <= 52, "250 lux de luz natural: luminária a 50 %");
  sim::analogico[PINO_LDR] = ldr(400); avancar(3000);
  CONFERE(pct() >= 18 && pct() <= 22, "400 lux: luminária a 20 %");
  sim::analogico[PINO_LDR] = ldr(50); avancar(3000);
  sim::analogico[PINO_LDR] = ldr(1000); avancar(150);
  CONFERE(pct() > 20 && pct() < 90, "Sol forte de repente (50 para 1000 lux): a luminária reduz aos poucos, sem salto");
  avancar(3000);
  CONFERE(sim::pwm[PINO_LUMINARIA] == 0, "1000 lux (acima da meta de 500): luminária apagada");
  sim::analogico[PINO_LDR] = ldr(300); avancar(3000);
  int antes = sim::pwm[PINO_LUMINARIA];
  sim::analogico[PINO_LDR] = ldr(304); avancar(3000);
  CONFERE(sim::pwm[PINO_LUMINARIA] == antes, "Variação pequena (300 para 304 lux): faixa morta, luminária não oscila");
  CONFERE(lcd.texto(1).rfind("Luminaria:", 0) == 0, "Visor mostra a potência da luminária");
  return fim();
}
