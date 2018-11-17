#pragma once
class __FlashStringHelper;

unsigned freeRam();

// http://playground.arduino.cc/Main/Printf
void p(const __FlashStringHelper* fmt, ...);
