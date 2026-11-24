// Simulador mínimo da API do Arduino para testar a lógica no computador.
// Não substitui o Wokwi: serve para conferir as decisões de cada automação.
#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <math.h>
#include <string>
#include <deque>
#include <algorithm>
typedef uint8_t byte;
typedef bool boolean;
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19
#define F(x) (x)
#define constrain(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))
using std::isnan;

namespace sim {
inline unsigned long agora = 0;
inline int saida[20], entrada[20], analogico[20], pwm[20], modo[20];
inline unsigned long ecoUs = 0;
struct Som { bool ligado; unsigned frequencia; unsigned long ate; };
inline Som som[20];
inline std::string serial;
inline std::deque<char> teclas;
inline int angulo[20];
inline float dhtT = 24, dhtH = 45;
inline void reiniciar() {
  agora = 0; serial.clear(); teclas.clear(); ecoUs = 0;
  for (int i = 0; i < 20; i++) { saida[i] = 0; entrada[i] = HIGH; analogico[i] = 0; pwm[i] = 0; modo[i] = 0; som[i] = {false, 0, 0}; angulo[i] = -1; }
}
inline bool tocando(int p) { return som[p].ligado && (som[p].ate == 0 || agora < som[p].ate); }
}

inline unsigned long millis() { return sim::agora; }
inline void delay(unsigned long ms) { sim::agora += ms; }
inline void delayMicroseconds(unsigned int) {}
inline void pinMode(int p, int m) { sim::modo[p] = m; }
inline void digitalWrite(int p, int v) { sim::saida[p] = v ? HIGH : LOW; }
inline int digitalRead(int p) { return sim::entrada[p]; }
inline int analogRead(int p) { return sim::analogico[p]; }
inline void analogWrite(int p, int v) { sim::pwm[p] = v; }
inline unsigned long pulseIn(int, int, unsigned long = 1000000) { return sim::ecoUs; }
inline void tone(int p, unsigned f, unsigned long d = 0) { sim::som[p] = {true, f, d ? sim::agora + d : 0}; }
inline void noTone(int p) { sim::som[p].ligado = false; }

class Print {
 public:
  virtual void escrever(const char *s) = 0;
  virtual ~Print() {}
  void print(const char *s) { escrever(s); }
  void print(char c) { char b[2] = {c, 0}; escrever(b); }
  void print(int v) { fmt("%d", v); }
  void print(unsigned v) { fmt("%u", v); }
  void print(long v) { fmt("%ld", v); }
  void print(unsigned long v) { fmt("%lu", v); }
  void print(double v, int casas = 2) { char b[32]; snprintf(b, sizeof b, "%.*f", casas, v); escrever(b); }
  template <typename T> void println(T v) { print(v); escrever("\n"); }
  void println(double v, int casas) { print(v, casas); escrever("\n"); }
  void println() { escrever("\n"); }
 private:
  template <typename T> void fmt(const char *f, T v) { char b[32]; snprintf(b, sizeof b, f, v); escrever(b); }
};

class SerialSim : public Print {
 public:
  void begin(long) {}
  void escrever(const char *s) override { sim::serial += s; }
};
inline SerialSim Serial;
