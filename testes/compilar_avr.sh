#!/bin/bash
# Compila cada automação para o ATmega328P (Arduino Uno) com o core oficial
# e as mesmas bibliotecas usadas no Wokwi. Mostra o uso de flash e de RAM.
set -e
# Requisitos: avr-gcc, avr-libc e git (Ubuntu/Debian: sudo apt install gcc-avr avr-libc git)
RAIZ=$(cd "$(dirname "$0")/.." && pwd)
DEPS=${DEPS:-$RAIZ/.deps}
OUT=${OUT:-$RAIZ/build/avr}
# Baixa o núcleo oficial do Arduino e as bibliotecas na primeira execução
mkdir -p "$DEPS"
baixar() { [ -d "$DEPS/$2" ] || git clone -q --depth 1 "https://github.com/$1.git" "$DEPS/$2"; }
baixar arduino/ArduinoCore-avr ArduinoCore-avr
baixar arduino-libraries/Servo Servo
baixar johnrickman/LiquidCrystal_I2C LiquidCrystal_I2C
baixar Chris--A/Keypad Keypad
baixar adafruit/DHT-sensor-library DHT-sensor-library
baixar adafruit/Adafruit_Sensor Adafruit_Sensor
CORE=$DEPS/ArduinoCore-avr
FLAGS="-mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10819 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR -Os -g -ffunction-sections -fdata-sections"
CXXFLAGS="$FLAGS -std=gnu++11 -fpermissive -fno-exceptions -fno-threadsafe-statics -Wno-error=narrowing"
INC="-I$CORE/cores/arduino -I$CORE/variants/standard -I$CORE/libraries/Wire/src -I$CORE/libraries/Wire/src/utility -I$DEPS/Servo/src -I$DEPS/LiquidCrystal_I2C -I$DEPS/Keypad/src -I$DEPS/DHT-sensor-library -I$DEPS/Adafruit_Sensor"
mkdir -p $OUT/core
# Core + bibliotecas: compilados uma vez
if [ ! -f $OUT/libbase.a ]; then
  for f in $CORE/cores/arduino/*.c $CORE/libraries/Wire/src/utility/*.c; do avr-gcc $FLAGS -w $INC -c $f -o $OUT/core/$(basename $f).o; done
  for f in $CORE/cores/arduino/*.S; do avr-gcc $FLAGS -x assembler-with-cpp $INC -c $f -o $OUT/core/$(basename $f).o; done
  for f in $CORE/cores/arduino/*.cpp $CORE/libraries/Wire/src/*.cpp $DEPS/Servo/src/avr/*.cpp $DEPS/LiquidCrystal_I2C/*.cpp $DEPS/Keypad/src/*.cpp $DEPS/DHT-sensor-library/DHT.cpp; do avr-g++ $CXXFLAGS -w $INC -c $f -o $OUT/core/$(basename $f).o; done
  avr-gcc-ar rcs $OUT/libbase.a $OUT/core/*.o
fi
for d in $RAIZ/wokwi/a*/; do
  n=$(basename $d)
  { echo '#include <Arduino.h>'; echo "#line 1 \"$n/sketch.ino\""; cat $d/sketch.ino; } > $OUT/$n.cpp
  avr-g++ $CXXFLAGS -Wall -Wextra -Wno-unused-parameter $INC -c $OUT/$n.cpp -o $OUT/$n.o
  avr-gcc -mmcu=atmega328p -Os -Wl,--gc-sections -o $OUT/$n.elf $OUT/$n.o $OUT/libbase.a -lm
  avr-size -C --mcu=atmega328p $OUT/$n.elf | awk -v n=$n '/Program:/{p=$2" B "$4")"} /Data:/{d=$2" B "$4")"} END{printf "%-26s flash %-18s RAM %s\n", n, p, d}'
done
