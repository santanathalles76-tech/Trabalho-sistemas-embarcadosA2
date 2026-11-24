#pragma once
#include "Arduino.h"
#define makeKeymap(x) ((char *)x)
#define NO_KEY '\0'
class Keypad {
 public:
  Keypad(char *, byte *, byte *, byte, byte) {}
  char getKey() { if (sim::teclas.empty()) return NO_KEY; char c = sim::teclas.front(); sim::teclas.pop_front(); return c; }
};
