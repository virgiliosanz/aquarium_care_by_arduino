#pragma once
#include <Arduino.h>

unsigned freeRam();

// http://playground.arduino.cc/Main/Printf
void p(const __FlashStringHelper* fmt, ...);
