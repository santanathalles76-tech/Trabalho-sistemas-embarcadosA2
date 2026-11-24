#pragma once
#include "Arduino.h"
#define DHT22 22
class DHT {
 public:
  DHT(int, int) {}
  void begin() {}
  float readTemperature() { return sim::dhtT; }
  float readHumidity() { return sim::dhtH; }
};
