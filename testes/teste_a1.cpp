#include "testes.h"
#include "mock/LiquidCrystal_I2C.h"
#include "../wokwi/a1-controle-acesso/sketch.ino"
void digitar(const char *s) { for (; *s; s++) { sim::teclas.push_back(*s); avancar(30); } }
int main() {
  std::cout << "Automação 1: controle de acesso\n";
  sim::reiniciar(); setup(); avancar(10);
  CONFERE(lcd.texto(0) == "Area restrita" && sim::angulo[PINO_SERVO] == 0, "Ao ligar: visor pede a senha e a trava fica fechada (0 graus)");
  digitar("47");
  CONFERE(lcd.texto(1) == "Senha: **", "Digitos aparecem mascarados com asterisco");
  digitar("*");
  CONFERE(lcd.texto(1) == "Senha:", "Tecla * apaga o que foi digitado");
  digitar("4729#");
  CONFERE(sim::angulo[PINO_SERVO] == 90 && sim::saida[PINO_LED_VERDE] && lcd.texto(0) == "Acesso liberado", "Senha 4729 + #: trava abre (90 graus) e LED verde acende");
  avancar(5100);
  CONFERE(sim::angulo[PINO_SERVO] == 0 && !sim::saida[PINO_LED_VERDE], "Depois de 5 s a trava fecha sozinha");
  digitar("1111#");
  CONFERE(sim::serial.find("Senha incorreta. Erros seguidos: 1") != std::string::npos && lcd.texto(1) == "Senha:", "Senha errada: aviso por 1,5 s e o visor volta a pedir a senha");
  CONFERE(errosSeguidos == 1 && sim::angulo[PINO_SERVO] == 0, "Senha errada: trava continua fechada e conta 1 erro");
  digitar("2222#"); digitar("3333#");
  CONFERE(estado == BLOQUEADO && sim::saida[PINO_LED_VERMELHO] && lcd.texto(0) == "ACESSO BLOQUEADO", "3 erros seguidos: teclado bloqueado com LED vermelho");
  digitar("4729#");
  CONFERE(sim::angulo[PINO_SERVO] == 0, "Durante o bloqueio, nem a senha correta abre a porta");
  avancar(15100);
  CONFERE(estado == AGUARDANDO && errosSeguidos == 0, "Depois de 15 s o bloqueio termina e os erros zeram");
  digitar("1234567890");
  CONFERE(quantidadeDigitos == 6, "Aceita no máximo 6 dígitos");
  digitar("*4729#");
  CONFERE(sim::angulo[PINO_SERVO] == 90, "Senha correta volta a funcionar após o bloqueio");
  return fim();
}
