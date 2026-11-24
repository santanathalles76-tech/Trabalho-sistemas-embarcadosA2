#pragma once
#include "Arduino.h"
class LiquidCrystal_I2C : public Print {
 public:
  char linhas[2][41];
  int col = 0, lin = 0;
  LiquidCrystal_I2C(int, int, int) { clear(); }
  void init() { clear(); }
  void backlight() {}
  void clear() { memset(linhas, ' ', sizeof linhas); linhas[0][40] = linhas[1][40] = 0; col = lin = 0; }
  void setCursor(int c, int l) { col = c; lin = l; }
  void escrever(const char *s) override { for (; *s; s++) { if (col < 40) linhas[lin][col] = *s; col++; } }
  std::string texto(int l) const { std::string t(linhas[l], 16); while (!t.empty() && t.back() == ' ') t.pop_back(); return t; }
};
