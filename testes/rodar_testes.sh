#!/bin/bash
# Compila e roda os testes de lógica das 9 automações no computador.
cd "$(dirname "$0")"
# Requisito: g++ com suporte a C++17
SAIDA=../build/testes
mkdir -p $SAIDA
resultado=0
for t in teste_a*.cpp; do
  g++ -std=c++17 -Wall -Wno-unused-variable -I mock -x c++ $t -o $SAIDA/${t%.cpp} || { resultado=1; continue; }
  $SAIDA/${t%.cpp} || resultado=1
  echo
done
exit $resultado
