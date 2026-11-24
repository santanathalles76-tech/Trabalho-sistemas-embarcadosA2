#include "testes.h"
#include "mock/LiquidCrystal_I2C.h"
#include "../wokwi/a3-ar-condicionado/sketch.ino"
// Leitura analógica que o NTC do Wokwi daria para a temperatura t
int ntc(double t) { double x = 1 / (t + 273.15) - 1 / 298.15; return (int)lround(1023.0 / (1 + exp(-3950 * x))); }
int main() {
  std::cout << "Automação 3: ar-condicionado\n";
  sim::reiniciar(); sim::entrada[PINO_PIR] = LOW;
  sim::analogico[PINO_POT] = 409; sim::analogico[PINO_NTC] = ntc(26);
  setup(); avancar(600);
  CONFERE(fabs(lerTemperatura() - 26) < 0.2 && lerTemperaturaDesejada() == 22.0f, "Leitura: 26 C no NTC e alvo de 22 C no potenciômetro");
  CONFERE(!sim::saida[PINO_RELE], "Sala vazia e quente (26 C): ar continua desligado");
  sim::entrada[PINO_PIR] = HIGH; avancar(600);
  CONFERE(sim::saida[PINO_RELE] && lcd.texto(1) == "Ocupada AR LIG", "Pessoa entra com 26 C (>= 23): compressor liga");
  sim::analogico[PINO_NTC] = ntc(22.5); avancar(6000);
  CONFERE(sim::saida[PINO_RELE], "22,5 C (dentro da histerese): continua ligado");
  sim::analogico[PINO_NTC] = ntc(20.8); avancar(600);
  CONFERE(!sim::saida[PINO_RELE], "20,8 C (<= 21): compressor desliga");
  sim::analogico[PINO_NTC] = ntc(23.5); avancar(2000);
  CONFERE(!sim::saida[PINO_RELE], "Esquenta logo em seguida: espera o intervalo mínimo de 5 s");
  avancar(3500);
  CONFERE(sim::saida[PINO_RELE], "Passados 5 s da última mudança: religa");
  sim::analogico[PINO_POT] = 1023; avancar(6000);
  CONFERE(!sim::saida[PINO_RELE], "Alvo muda para 28 C: 23,5 C já está frio o bastante e desliga");
  sim::analogico[PINO_POT] = 409; avancar(6000);
  CONFERE(sim::saida[PINO_RELE], "Alvo volta a 22 C: liga de novo");
  sim::entrada[PINO_PIR] = LOW; avancar(19000);
  CONFERE(sim::saida[PINO_RELE], "Pessoas saem: mantém ligado por até 20 s");
  avancar(1500);
  CONFERE(!sim::saida[PINO_RELE] && lcd.texto(1) == "Vazia   AR DESL", "20 s sem movimento: desliga mesmo com 23,5 C");
  return fim();
}
