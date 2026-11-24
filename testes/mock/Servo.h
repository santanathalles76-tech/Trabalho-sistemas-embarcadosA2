#pragma once
#include "Arduino.h"
class Servo {
  int pino = -1;
 public:
  void attach(int p) { pino = p; }
  void write(int a) { if (pino >= 0) sim::angulo[pino] = a; }
};
