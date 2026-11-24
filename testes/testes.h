// Utilitários comuns aos testes de lógica
#pragma once
#include "mock/Arduino.h"
#include <iostream>
static int falhas = 0, total = 0;
#define CONFERE(cond, descricao) do { total++; if (cond) std::cout << "  [OK]    " << descricao << "\n"; \
  else { falhas++; std::cout << "  [FALHA] " << descricao << "  (linha " << __LINE__ << ")\n"; } } while (0)
void setup(); void loop();
// Avança o tempo simulado chamando loop() a cada milissegundo
inline void avancar(unsigned long ms) { unsigned long fim = sim::agora + ms; while (sim::agora < fim) { loop(); sim::agora++; } }
// Aperta e solta um botão ligado ao GND (pull-up)
inline void apertar(int pino) { sim::entrada[pino] = LOW; avancar(120); sim::entrada[pino] = HIGH; avancar(120); }
inline int fim() { std::cout << "  " << (total - falhas) << "/" << total << " verificações corretas\n"; return falhas ? 1 : 0; }
